#include <Windows.h>
#include <iostream>
#include <stdexcept>   
#include <conio.h>

#include "StateMachine.h"
StateMachine stateMachine;

void DisableCloseButton()
{
	HWND hwnd = GetConsoleWindow();
	HMENU hmenu = GetSystemMenu(hwnd, FALSE);
	EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
}

bool DisableQuickEdit()
{
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (hStdin != INVALID_HANDLE_VALUE)
	{
		DWORD prev_mode = 0;
		if (GetConsoleMode(hStdin, &prev_mode))
		{
			DWORD new_mode = ENABLE_EXTENDED_FLAGS | (prev_mode & ~ENABLE_QUICK_EDIT_MODE);
			if (SetConsoleMode(hStdin, new_mode))
			{
				return true;
			}
		}
	}
	return false;
}

void DisableClosing()
{
	HWND hwnd = ::GetConsoleWindow();
	if (hwnd != NULL)
	{
		HMENU hMenu = ::GetSystemMenu(hwnd, FALSE);
		if (hMenu != NULL) DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
	}
}

int main(void)
{
	ShowWindow(GetConsoleWindow(), SW_MINIMIZE);

	DisableQuickEdit();	// otherwise the console stops when the mouse is clicked
//	DisableCloseButton();
	DisableClosing();
	
	SetConsoleTitleA("TSMS Master Version 0.0.2.0 (241120)");		// todo : version management

	try
	{
		stateMachine.Startup();
		
		bool exit_program = false;
		while (!exit_program)
		{
			auto state = stateMachine.Update();

			if (GetAsyncKeyState('E') && GetAsyncKeyState('N') && GetAsyncKeyState('D'))
				stateMachine.ChangeState(eStates::SHUTDOWN_CMD);

			if (state == eStates::EXIT)
				exit_program = true;
		}

		stateMachine.Shutdown();
		Sleep(100);

		std::cout << "\nProgram finished" << "\n";
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << "\n";
		std::cout << "\nProgram finished with error" << "\n";
	}

#ifndef DEBUG
	system("pause");
#endif // !DEBUG

}