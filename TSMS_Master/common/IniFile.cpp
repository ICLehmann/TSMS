#include "IniFile.h"
#include <iostream>

using namespace std;

IniFile::IniFile(const char* filename, bool bCreateIfNotExists)
{
	m_filename = filename;
	m_bModified = false;
	m_bError = true;
	if (!ReadFile() && bCreateIfNotExists)
		Create();
}

IniFile::~IniFile(void)
{
	if (m_bModified)
		WriteFile();
}

// A function to trim whitespace from both sides of a given string
void Trim(std::string& str, const std::string & ChrsToTrim = " \t\n\r", int TrimDir = 0)
{
	size_t startIndex = str.find_first_not_of(ChrsToTrim);
	if (startIndex == std::string::npos)
	{
		str.erase();
		return;
	}
	if (TrimDir < 2)
		str = str.substr(startIndex, str.size() - startIndex);
	if (TrimDir != 1)
		str = str.substr(0, str.find_last_not_of(ChrsToTrim) + 1);
}

// A function to transform a string to uppercase if neccessary
//void UCase(string& str, bool ucase)
//{
//	if(ucase) transform(str.begin(), str.end(), str.begin(), toupper);
//}

bool IniFile::ReadFile()
{
	string line;											// Holds the current line from the ini file
	string CurrentSection;									// Holds the current section name

	ifstream inFile(m_filename.c_str());
	if (!inFile.is_open())
	{
		m_bError = true;
		return false;
	}

	m_content.clear();										// Clear the content vector

	string comments = "";									// A string to store comments in

	while (!std::getline(inFile, line).eof())				// Read until the end of the file
	{
		Trim(line);											// Trim whitespace from the ends
		if (!line.empty())									// Make sure its not a blank line
		{
			Record r;										// Define a new record

			if (line[0] == '#')								// Is this a commented line?
			{
				if ((line.find('[') == string::npos) &&		// If there is no [ or =
						(line.find('=') == string::npos))	// Then it's a comment
				{
					comments += line + '\n';				// Add the comment to the current comments string
				}
				else
				{
					r.Commented = line[0];					// Save the comment character
					line.erase(line.begin());				// Remove the comment for further processing
					Trim(line);								// Remove any more whitespace
				}
			}
			else
				r.Commented = 0;							// else mark it as not being a comment

			if (line.find('[') != string::npos)				// Is this line a section?
			{
				line.erase(line.begin());					// Erase the leading bracket
				line.erase(line.find(']'));					// Erase the trailing bracket
				r.Comments = comments;						// Add the comments string (if any)
				comments = "";								// Clear the comments for re-use
				r.Section = line;							// Set the Section value
				r.Key = "";									// Set the Key value
				r.Value = "";								// Set the Value value
				CurrentSection = line;
			}

			if (line.find('=') != string::npos)				// Is this line a Key/Value?
			{
				r.Comments = comments;						// Add the comments string (if any)
				comments = "";								// Clear the comments for re-use
				r.Section = CurrentSection;					// Set the section to the current Section
				r.Key = line.substr(0, line.find('='));		// Set the Key value to everything before the = sign
				r.Value = line.substr(line.find('=') + 1);	// Set the Value to everything after the = sign
			}
			if (comments == "")								// Don't add a record yet if its a comment line
				m_content.push_back(r);						// Add the record to content
		}
	}

	inFile.close();
	m_bError = false;
	m_bModified = false;
	return true;
}

bool IniFile::WriteFile()
{
	ofstream outFile(m_filename.c_str()/*, ios::out|ios::binary*/);
	if (!outFile.is_open())
	{
		m_bError = true;
		return false;
	}

	for (int i = 0; i < (int) m_content.size(); i++)		// Loop through each vector
	{
		outFile << m_content[i].Comments;					// Write out the comments
		if (m_content[i].Commented != 0)
			outFile << m_content[i].Commented;
		if (m_content[i].Key == "")							// Is this a section?
			// Then format the section
			outFile << "[" << m_content[i].Section << "]" << /*"\r" <<*/ endl;
		else
			// Else format a key/value
			outFile << m_content[i].Key << "=" << m_content[i].Value << /*"\r" <<*/ endl;
	}

	outFile.close();
	m_bError = false;
	m_bModified = false;
	return true;
}

string IniFile::Content()
{
	string s = "";
	if (m_content.empty())
		return s;

	for (int i = 0; i < (int) m_content.size(); i++)		// Loop through the content
	{
		if (m_content[i].Comments != "")
			s += m_content[i].Comments;						// Add the comments
		if (m_content[i].Commented != 0)
			s += m_content[i].Commented;					// If this is commented, then add it
		if ((m_content[i].Key == ""))						// Is this a section?
			s += '[' + m_content[i].Section + ']';			// Add the section
		else
			s += m_content[i].Key + '=' + m_content[i].Value;		// Or the Key value to the return string

		if (i != (int) m_content.size())
			s += '\n';										// If this is not the last line, add a CrLf
	}
	return s;
}

vector<string> IniFile::GetSectionNames()
{
	vector<string> data;
	for (int i = 0; i < (int) m_content.size(); i++)		// Loop through the content
	{
		if (m_content[i].Key == "")							// If there is no key value, then its a section
			data.push_back(m_content[i].Section);			// Add the section to the return data
	}
	return data;
}

vector<IniFile::Record> IniFile::GetSection(string Section)
{
	vector<Record> data;
	for (int i = 0; i < (int) m_content.size(); i++)		// Loop through the content
	{
		if ((m_content[i].Section == Section) &&			// If this is the section name we want
				(m_content[i].Key != ""))					// but not the section name itself
			data.push_back(m_content[i]);					// Add the record to the return data
	}
	return data;
}

bool IniFile::RecordExists(string Key, string Section)
{
	// Locate the Section/Key
	vector<Record>::iterator iter = find_if(m_content.begin(), m_content.end(), RecordSectionKeyIs(Section, Key));
	if (iter == m_content.end())
		return false;
	else
		return true;
}

bool IniFile::SectionExists(string Section)
{
	// Locate the Section
	vector<Record>::iterator iter = find_if(m_content.begin(), m_content.end(), RecordSectionIs(Section));
	if (iter == m_content.end())
		return false;
	else
		return true;
}

vector<IniFile::Record> IniFile::GetRecord(string Key, string Section)
{
	vector<Record> data;

	// Locate the Record
	vector<Record>::iterator iter = find_if(m_content.begin(), m_content.end(), RecordSectionKeyIs(Section, Key));
	if (iter == m_content.end())
		return data;										// The Record was not found
	data.push_back(*iter);									// The Record was found
	return data;
}

string IniFile::GetValue(string Section, string Key)
{
	vector<Record> content = GetRecord(Key, Section);		// Get the Record
	if (!content.empty())									// Make sure there is a value to return
		return content[0].Value;							// And return the value
	return "";												// No value was found
}

void IniFile::SetValue(string Section, string Key, string Value)
{
	if (m_bError)
		return;

	if (!SectionExists(Section))							// If the Section doesn't exist
	{
		Record s =
		{ "", 0, Section, "", "" };							// Define a new section
		Record r =
		{ "", 0, Section, Key, Value };						// Define a new record
		m_content.push_back(s);								// Add the section
		m_content.push_back(r);								// Add the record
		m_bModified = true;
		return;
	}

	if (!RecordExists(Key, Section))						// If the Key doesn't exist
	{
		// Locate the Section
		vector<Record>::iterator iter = find_if(m_content.begin(), m_content.end(), RecordSectionIs(Section));

		//CL: Insert new key at the end of this section
		while (iter < m_content.end() && iter->Section == Section)
		{
			iter++;
		}
		// Advance just past the section
		Record r =
		{ "", 0, Section, Key, Value };						// Define a new record
		m_content.insert(iter, r);							// Add the record
		m_bModified = true;
		return;
	}

	// Locate the Record
	vector<Record>::iterator iter = find_if(m_content.begin(), m_content.end(), RecordSectionKeyIs(Section, Key));

	iter->Value = Value;									// Insert the correct value
	m_bModified = true;
}

void IniFile::RenameSection(string OldSection, string NewSection)
{
	if (m_bError)
		return;

	// Loop through the records
	for (vector<Record>::iterator iter = m_content.begin(); iter < m_content.end(); iter++)
	{
		if (iter->Section == OldSection)					// Is this the OldSection?
			iter->Section = NewSection;						// Now its the NewSection
	}
	m_bModified = true;
}

void IniFile::CommentRecord(string Key, string Section)
{
	if (m_bError)
		return;

	// Locate the Section/Key
	vector<Record>::iterator iter = find_if(m_content.begin(), m_content.end(), RecordSectionKeyIs(Section, Key));
	if (iter == m_content.end())
		return;												// The Section/Key was not found

	iter->Commented = ';';									// Change the Comment value
	m_bModified = true;
}

void IniFile::UnCommentRecord(string Key, string Section)
{
	if (m_bError)
		return;

	// Locate the Section/Key
	vector<Record>::iterator iter = find_if(m_content.begin(), m_content.end(), RecordSectionKeyIs(Section, Key));
	if (iter == m_content.end())
		return;												// The Section/Key was not found

	iter->Commented = 0;									// Remove the Comment value
	m_bModified = true;
}

void IniFile::CommentSection(string Section)
{
	if (m_bError)
		return;

	for (vector<Record>::iterator iter = m_content.begin(); iter < m_content.end(); iter++)
	{
		if (iter->Section == Section)						// Is this the right section?
			iter->Commented = ';';							// Change the comment value
	}
	m_bModified = true;
}

void IniFile::UnCommentSection(string Section)
{
	if (m_bError)
		return;

	for (vector<Record>::iterator iter = m_content.begin(); iter < m_content.end(); iter++)
	{
		if (iter->Section == Section)						// Is this the right section?
			iter->Commented = 0;							// Remove the comment value
	}
	m_bModified = true;
}

bool IniFile::DeleteRecord(string Key, string Section)
{
	if (m_bError)
		return false;

	// Locate the Section/Key
	vector<Record>::iterator iter = find_if(m_content.begin(), m_content.end(), RecordSectionKeyIs(Section, Key));
	if (iter == m_content.end())
		return false;										// The Section/Key was not found

	m_content.erase(iter);									// Remove the Record
	m_bModified = true;
	return true;
}

bool IniFile::DeleteSection(string Section)
{
	if (m_bError)
		return false;

	bool bRet = false;
	for (int i = (int) m_content.size() - 1; i > -1; i--)	// Iterate backwards through the content
	{
		if (m_content[i].Section == Section)				// Is this related to the Section?
		{
			m_content.erase(m_content.begin() + i);			// Then erase it
			m_bModified = true;
			bRet = true;
		}
	}
	return bRet;
}

bool IniFile::SetSectionComments(string Comments, string Section)
{
	if (m_bError)
		return false;

	// Loop through the records
	for (vector<Record>::iterator iter = m_content.begin(); iter < m_content.end(); iter++)
	{
		if ((iter->Section == Section) && (iter->Key == ""))		// Is this the section and not a record?
		{
			if (Comments.size() >= 2)								// Is there a comment?
			{
				if (Comments.substr(Comments.size() - 2) != "\n")	// Does the string end in a newline?
					Comments += "\n";								// If not, add one
			}
			iter->Comments = Comments;								// Set the comments
			m_bModified = true;
			return true;
		}
	}
	return false;
}

bool IniFile::SetRecordComments(string Comments, string Key, string Section)
{
	if (m_bError)
		return false;

	// Locate the Section/Key
	vector<Record>::iterator iter = find_if(m_content.begin(), m_content.end(), RecordSectionKeyIs(Section, Key));
	if (iter == m_content.end())
		return false;											// The Section/Key was not found

	if (Comments.size() >= 2)									// Is there a comment?
	{
		if (Comments.substr(Comments.size() - 2) != "\n")		// Does the string end in a newline?
			Comments += "\n";									// If not, add one
	}
	iter->Comments = Comments;									// Set the comments
	m_bModified = true;
	return true;
}

vector<IniFile::Record> IniFile::GetSections()
{
	vector<Record> data;

	for (int i = 0; i < (int) m_content.size(); i++)			// Loop through the content
	{
		if (m_content[i].Key == "")								// If this is a section
			data.push_back(m_content[i]);						// Add the record to the return data
	}
	return data;
}

void IniFile::AddSection(string Section)
{
	if (m_bError)
		return;
	Record s =
	{ "", ' ', Section, "", "" };								// Define a new section
	m_content.push_back(s);										// Add the section
	m_bModified = true;
}

bool IniFile::Create()
{
	vector<Record> content;										// Create empty content
	return WriteFile();
}

//---------------------------------------------------------------------------------------
//CL: Erweiterung für Kompatibilität mit "alter" Klasse
#include <sstream>

int IniFile::ReadInteger(const char* cSection, const char* cKey, int iDefaultValue)
{
	string strResult = GetValue((string) cSection, (string) cKey);
	int iResult = 0;
	if (strResult != "")
		iResult = atoi(strResult.c_str());
	else
	{
		iResult = iDefaultValue;
		WriteInteger(cSection, cKey, iDefaultValue);
	}
	return iResult;
}

float IniFile::ReadFloat(const char* cSection, const char* cKey, float fDefaultValue)
{
	string strResult = GetValue((string) cSection, (string) cKey);
	float fResult = 0;
	if (strResult != "")
		fResult = (float) atof(strResult.c_str());
	else
	{
		fResult = fDefaultValue;
		WriteFloat(cSection, cKey, fDefaultValue);
	}
	return fResult;
}

bool IniFile::ReadBoolean(const char* cSection, const char* cKey, bool bDefaultValue)
{
	string strResult = GetValue((string) cSection, (string) cKey);
	bool bResult = false;

	if (strResult != "")
		bResult = (strResult == "True" || strResult == "true") ? true : false;
	else
	{
		bResult = bDefaultValue;
		WriteBoolean(cSection, cKey, bDefaultValue);
	}
	return bResult;
}

const char* IniFile::ReadString(const char* cSection, const char* cKey, const char* cDefaultValue)
{
	m_ReadString_Result = GetValue((string) cSection, (string) cKey);
	if (m_ReadString_Result == "")
	{
		WriteString(cSection, cKey, cDefaultValue);
		return cDefaultValue; 
	}
	return m_ReadString_Result.c_str();
}

void IniFile::WriteInteger(const char* cSection, const char* cKey, int iValue)
{
	std::stringstream streamValue;
	streamValue << iValue;
	SetValue((string) cSection, (string) cKey, streamValue.str());
}

void IniFile::WriteFloat(const char* cSection, const char* cKey, float fValue)
{
	std::stringstream streamValue;
	streamValue << fValue;
	SetValue((string) cSection, (string) cKey, streamValue.str());
}

void IniFile::WriteBoolean(const char* cSection, const char* cKey, bool bValue)
{
	string strValue;
	if (bValue)
		strValue = "True";
	else
		strValue = "False";
	SetValue((string) cSection, (string) cKey, strValue);
}

void IniFile::WriteString(const char* cSection, const char* cKey, const char* cValue)
{
	SetValue((string) cSection, (string) cKey, (string) cValue);
}

