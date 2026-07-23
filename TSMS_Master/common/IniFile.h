#pragma once

// based on:
// http://www.codeproject.com/Articles/8342/CIniFile-Class-for-C-A-robust-cross-platform-INI-f
// modified CL

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>

template < typename R, typename A > using my_unary_function = std::function< R(A) >;

class IniFile
{
public:
	struct Record
	{
		std::string Comments;
		char Commented;
		std::string Section;
		std::string Key;
		std::string Value;
	};

	IniFile(const char* filename, bool bCreateIfNotExists = true);
	virtual ~IniFile(void);

	bool ReadFile();
	bool WriteFile();

	void AddSection(std::string Section);
	void CommentRecord(std::string Key, std::string Section);
	void CommentSection(std::string Section);
	std::string Content();
	bool Create();
	bool DeleteRecord(std::string Key, std::string Section);
	bool DeleteSection(std::string Section);
	std::vector<Record> GetRecord(std::string Key, std::string Section);
	std::vector<Record> GetSection(std::string Section);
	std::vector<std::string> GetSectionNames();
	std::string GetValue(std::string Section, std::string Key);
	bool RecordExists(std::string Key, std::string Section);
	void RenameSection(std::string OldSection, std::string NewSection);
	bool SectionExists(std::string Section);
	bool SetRecordComments(std::string Comments, std::string Key, std::string Section);
	bool SetSectionComments(std::string Comments, std::string Section);
	void SetValue(std::string Section, std::string Key, std::string Value);
	void UnCommentRecord(std::string Key, std::string Section);
	void UnCommentSection(std::string Section);

	//CL: extensions for compatibility with my "old" ini class
	int ReadInteger(const char* cSection, const char* cKey, int iDefaultValue);
	float ReadFloat(const char* cSection, const char* cKey, float fDefaultValue);
	bool ReadBoolean(const char* cSection, const char* cKey, bool bDefaultValue);
	const char* ReadString(const char* cSection, const char* cKey, const char* szDefaultValue);

	void WriteInteger(const char* cSection, const char* cKey, int iValue);
	void WriteFloat(const char* cSection, const char* cKey, float fValue);
	void WriteBoolean(const char* cSection, const char* cKey, bool bValue);
	void WriteString(const char* cSection, const char* cKey, const char* cValue);

	bool IsError() {return m_bError;}

private:
	bool m_bError;
	bool m_bModified;
	std::string m_filename;
	std::vector<Record> m_content;		// Holds the current record
	std::string m_ReadString_Result;		// Hilfsvariable für Rückgabe

	std::vector<Record> GetSections();

	struct RecordSectionIs: my_unary_function<Record, bool>
	{
		std::string section_;
		RecordSectionIs(const std::string& section) :
				section_(section)
		{
		}

		bool operator()(const Record& rec) const
		{
			return rec.Section == section_;
		}
	};

	struct RecordSectionKeyIs: my_unary_function<Record, bool>
	{
		std::string section_;
		std::string key_;
		RecordSectionKeyIs(const std::string& section, const std::string& key) :
				section_(section), key_(key)
		{
		}

		bool operator()(const Record& rec) const
		{
			return ((rec.Section == section_) && (rec.Key == key_));
		}
	};
};
