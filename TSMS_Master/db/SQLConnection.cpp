#include "SQLConnection.h"
#include <iostream>

#include "../common/Logger.h"


SQLHANDLE SQLEnvHandle = NULL;
SQLHANDLE SQLConnectionHandle = NULL;
SQLHANDLE SQLStatementHandle = NULL;

void Free()
{
	if (SQLStatementHandle != NULL)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, SQLStatementHandle);
		SQLStatementHandle = NULL;
	}

	if (SQLConnectionHandle != NULL)
	{
		SQLDisconnect(SQLConnectionHandle);
		SQLFreeHandle(SQL_HANDLE_DBC, SQLConnectionHandle);
		SQLConnectionHandle = NULL;
	}

	if (SQLEnvHandle != NULL)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, SQLEnvHandle);
		SQLEnvHandle = NULL;
	}
}

void OnError(unsigned int handletype, const SQLHANDLE& handle)
{
	SQLCHAR sqlstate[1024];
	SQLCHAR message[1024];
	if (SQL_SUCCESS == SQLGetDiagRec(handletype, handle, 1, sqlstate, NULL, message, 1024, NULL))
	{
		Free();
		logger::WriteLog("DB ERROR: " + std::string((char*)message) + "\nSQLSTATE : " + std::string((char*)sqlstate));
	}
}

SQLConnection::SQLConnection()
{
}

SQLConnection::~SQLConnection()
{
	Free();
}

bool SQLConnection::Connect(std::string connection_string)
{
	std::string errorMsg = "Error while connecting SQL Server\n";
	SQLCHAR retconstring[1024] = "";

	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &SQLEnvHandle))
	{
		throw std::runtime_error(errorMsg);
	}

	if (SQL_SUCCESS != SQLSetEnvAttr(SQLEnvHandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
	{
		throw std::runtime_error(errorMsg);
	}

	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, SQLEnvHandle, &SQLConnectionHandle))
	{
		throw std::runtime_error(errorMsg);
	}

	if (SQL_SUCCESS != SQLSetConnectAttr(SQLConnectionHandle, SQL_LOGIN_TIMEOUT, (SQLPOINTER)2, 0))
	{
		throw std::runtime_error(errorMsg);
	}

	switch (SQLDriverConnect(SQLConnectionHandle, NULL, (SQLCHAR*)connection_string.c_str(), SQL_NTS, retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT))
	{
	case SQL_SUCCESS_WITH_INFO:
		logger::WriteLog("Connection to SQL-Server established");
		break;

	case SQL_INVALID_HANDLE:
		OnError(SQL_HANDLE_DBC, SQLConnectionHandle);
		break;

	case SQL_NO_DATA_FOUND:
		OnError(SQL_HANDLE_DBC, SQLConnectionHandle);
		break;

	case SQL_ERROR:
		OnError(SQL_HANDLE_DBC, SQLConnectionHandle);
		break;

	default:
		break;
	}

	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, SQLConnectionHandle, &SQLStatementHandle))
	{
		OnError(SQL_HANDLE_DBC, SQLConnectionHandle);
	}
	
	return true;
}

void SQLConnection::Disconnect()
{
	Free();
}

std::pair<bool, SQLHANDLE> SQLConnection::ExecSQL(std::string statement)
{
	bool success = true;
	SQLRETURN ret = SQLExecDirect(SQLStatementHandle, (SQLCHAR*)statement.c_str(), SQL_NTS);
	if (SQL_SUCCESS != ret)
	{
		success = false;
		OnError(SQL_HANDLE_STMT, SQLStatementHandle);
	}
	return { success, SQLStatementHandle };
}


