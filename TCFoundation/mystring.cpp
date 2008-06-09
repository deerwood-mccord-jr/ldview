#include "mystring.h"
#include "ConvertUTF.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#ifdef WIN32

#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG

// In Windows, we normally don't have a console.  However, we may have a
// console.
bool g_haveConsole = false;
bool g_bRealConsole = false;
HANDLE g_hStdOut = NULL;

class ConsoleBuffer
{
public:
	~ConsoleBuffer();
	void vprintf(const char *format, va_list argPtr);
	void vwprintf(const wchar_t *format, va_list argPtr);
private:
	ucstring m_buffer;
};

ConsoleBuffer g_consoleBuffer;

ConsoleBuffer::~ConsoleBuffer()
{
	if (m_buffer.size() > 0)
	{
#ifdef TC_NO_UNICODE
		MessageBoxA(NULL, m_buffer.c_str(), "LDView console output", MB_OK);
#else // TC_NO_UNICODE
		MessageBoxW(NULL, m_buffer.c_str(), L"LDView console output", MB_OK);
#endif // TC_NO_UNICODE
	}
}

void ConsoleBuffer::vprintf(const char *format, va_list argPtr)
{
	std::string temp;

#if _MSC_VER < 1400	// VC < VC 2005
	int size;
	temp.resize(65536);
	size = vsprintf(&temp[0], format, argPtr);
	temp.resize(size);
#else
	temp.resize(_vscprintf(format, argPtr));
	vsprintf(&temp[0], format, argPtr);
#endif
#ifdef TC_NO_UNICODE
	m_buffer += temp;
#else // TC_NO_UNICODE
	std::wstring wtemp;
	stringtowstring(wtemp, temp);
	m_buffer += wtemp;
#endif // TC_NO_UNICODE
}

void ConsoleBuffer::vwprintf(const wchar_t *format, va_list argPtr)
{
	std::wstring wtemp;

#if _MSC_VER < 1400	// VC < VC 2005
	int size;
	wtemp.resize(65536);
	size = vswprintf(&wtemp[0], format, argPtr);
	wtemp.resize(size);
#else
	// While std::wstring (and std::string) appear to always allocate space for
	// a terminating NULL character, their documentation indicates that they
	// might not.
	wtemp.resize(_vscwprintf(format, argPtr) + 1);
	vswprintf(&wtemp[0], wtemp.size(), format, argPtr);
	wtemp.resize(wtemp.size() - 1);
#endif
#ifdef TC_NO_UNICODE
	std::string temp;
	wstringtostring(temp, wtemp);
	m_buffer += temp;
#else // TC_NO_UNICODE
	m_buffer += wtemp;
#endif // TC_NO_UNICODE
}

#else // WIN32

#include <stdlib.h>
#include <wchar.h>

#endif

static int debugLevel = 0;

int sucprintf(UCSTR buffer, size_t maxLen, CUCSTR format, ...)
{
	va_list argPtr;
	int retValue;

	va_start(argPtr, format);
	retValue = vsucprintf(buffer, maxLen, format, argPtr);
	va_end(argPtr);
	return retValue;
}

#if (defined(_MSC_VER) && _MSC_VER < 1400) || defined(TC_NO_UNICODE)
int vsucprintf(UCSTR buffer, size_t /*maxLen*/, CUCSTR format, va_list argPtr)
#else // Not VC, or VC 2005+
int vsucprintf(UCSTR buffer, size_t maxLen, CUCSTR format, va_list argPtr)
#endif
{
#ifdef TC_NO_UNICODE
	return vsprintf(buffer, format, argPtr);
#else // TC_NO_UNICODE
	int formatLen = wcslen(format);
	wchar_t *newFormat = new wchar_t[formatLen + 1];
	wchar_t *spot;
	int retValue;

	wcscpy(newFormat, format);
	while ((spot = wcsstr(newFormat, L"%s")) != NULL)
	{
		wchar_t *temp = new wchar_t[formatLen + 2];
		size_t offset = spot - newFormat;

		wcsncpy(temp, newFormat, offset);
		wcscpy(&temp[offset], L"%ls");
		wcscpy(&temp[offset + 3], &spot[2]);
		delete newFormat;
		newFormat = temp;
		formatLen++;
	}
#if defined(_MSC_VER) && _MSC_VER < 1400	// VC < VC 2005
	retValue = vswprintf(buffer, newFormat, argPtr);
#else // Not VC, or VC 2005+
	retValue = vswprintf(buffer, maxLen, newFormat, argPtr);
#endif
	delete newFormat;
	return retValue;
#endif // TC_NO_UNICODE
}

char *copyString(const char *string, int pad)
{
	if (string)
	{
		return strcpy(new char[strlen(string) + 1 + pad], string);
	}
	else
	{
		return NULL;
	}
}

wchar_t *copyString(const wchar_t *string, int pad)
{
	if (string)
	{
		return wcscpy(new wchar_t[wcslen(string) + 1 + pad], string);
	}
	else
	{
		return NULL;
	}
}

#ifndef __APPLE__

char *strnstr(const char *s1, const char *s2, size_t n)
{
	return strnstr2(s1, s2, n, 0);
}

char *strcasestr(const char *s1, const char *s2) __THROW
{
	char* spot;
	int len1 = strlen(s1);
	int len2 = strlen(s2);

	for (spot = (char*)s1; spot - s1 <= len1 - len2; spot++)
	{
		if (strncasecmp(spot, s2, len2) == 0)
		{
			return spot;
		}
	}
	return NULL;
}

#endif // !__APPLE__

char *strnstr2(const char *s1, const char *s2, size_t n, int skipZero)
{
	char* spot;
	int len2 = strlen(s2);

	for (spot = (char*)s1; (*spot != 0 || skipZero) &&
		(unsigned)(spot-s1) < n; spot++)
	{
		if (strncmp(spot, s2, len2) == 0)
		{
			return spot;
		}
	}
	return NULL;
}

char *strncasestr(const char *s1, const char *s2, size_t n, int skipZero)
{
	char* spot;
	int len2 = strlen(s2);

	for (spot = (char*)s1; (*spot != 0 || skipZero) &&
		(unsigned)(spot - s1) < n; spot++)
	{
		if (strncasecmp(spot, s2, len2) == 0)
		{
			return spot;
		}
	}
	return NULL;
}

//int stringHasPrefix(const char *s1, const char *s2)
//{
//	return strncmp(s1, s2, strlen(s2)) == 0;
//}

void printStringArray(char** array, int count)
{
	int i;

	for (i = 0; i < count; i++)
	{
		printf("<%s>\n", array[i]);
	}
}

char **copyStringArray(char** array, int count)
{
	if (array)
	{
		int i;
		char **newArray = new char*[count];

		for (i = 0; i < count; i++)
		{
			newArray[i] = copyString(array[i]);
		}
		return newArray;
	}
	else
	{
		return NULL;
	}
}

/*
template<class T>void deleteStringArray(T** array, int count)
{
	int i;

	for (i = 0; i < count; i++)
	{
		delete array[i];
	}
	delete array;
}
*/
/*
void deleteStringArray(wchar_t** array, int count)
{
	int i;

	for (i = 0; i < count; i++)
	{
		delete array[i];
	}
	delete array;
}
*/
bool arrayContainsString(char** array, int count, const char* string)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (strcmp(array[i], string) == 0)
		{
			return true;
		}
	}
	return false;
}

bool arrayContainsPrefix(char** array, int count, const char* prefix)
{
	int i;
	int prefixLength = strlen(prefix);

	for (i = 0; i < count; i++)
	{
		if (strncmp(array[i], prefix, prefixLength) == 0)
		{
			return true;
		}
	}
	return false;
}

char** componentsSeparatedByString(const char* string, const char* separator,
								   int &count)
{
	int i;
	char* spot = (char*)string;
	char* tokenEnd = NULL;
	int separatorLength = strlen(separator);
	char** components;
	char* stringCopy;

	if (strlen(string) == 0)
	{
		count = 0;
		return NULL;
	}
	for (i = 0; (spot = strstr(spot, separator)) != NULL; i++)
	{
		spot += separatorLength;
	}
	count = i + 1;
	components = new char*[count];
	stringCopy = new char[strlen(string) + 1];
	strcpy(stringCopy, string);
	tokenEnd = strstr(stringCopy, separator);
	if (tokenEnd)
	{
		*tokenEnd = 0;
	}
	spot = stringCopy;
	for (i = 0; i < count; i++)
	{
		if (spot)
		{
			components[i] = new char[strlen(spot) + 1];
			strcpy(components[i], spot);
		}
		else
		{
			components[i] = new char[1];
			components[i][0] = 0;
		}
		if (tokenEnd)
		{
			spot = tokenEnd + separatorLength;
		}
		else
		{
			spot = NULL;
		}
		if (spot)
		{
			tokenEnd = strstr(spot, separator);
			if (tokenEnd)
			{
				*tokenEnd = 0;
			}
		}
		else
		{
			tokenEnd = NULL;
		}
	}
	delete stringCopy;
	return components;
}

wchar_t** componentsSeparatedByString(const wchar_t* string,
									  const wchar_t* separator,
									  int &count)
{
	int i;
	wchar_t* spot = (wchar_t*)string;
	wchar_t* tokenEnd = NULL;
	int separatorLength = wcslen(separator);
	wchar_t** components;
	wchar_t* stringCopy;

	if (wcslen(string) == 0)
	{
		count = 0;
		return NULL;
	}
	for (i = 0; (spot = wcsstr(spot, separator)) != NULL; i++)
	{
		spot += separatorLength;
	}
	count = i + 1;
	components = new wchar_t*[count];
	stringCopy = copyString(string);
	tokenEnd = wcsstr(stringCopy, separator);
	if (tokenEnd)
	{
		*tokenEnd = 0;
	}
	spot = stringCopy;
	for (i = 0; i < count; i++)
	{
		if (spot)
		{
			components[i] = new wchar_t[wcslen(spot) + 1];
			wcscpy(components[i], spot);
		}
		else
		{
			components[i] = new wchar_t[1];
			components[i][0] = 0;
		}
		if (tokenEnd)
		{
			spot = tokenEnd + separatorLength;
		}
		else
		{
			spot = NULL;
		}
		if (spot)
		{
			tokenEnd = wcsstr(spot, separator);
			if (tokenEnd)
			{
				*tokenEnd = 0;
			}
		}
		else
		{
			tokenEnd = NULL;
		}
	}
	delete stringCopy;
	return components;
}

char* componentsJoinedByString(char** array, int count, const char* separator)
{
	int length = 0;
	int i;
	int separatorLength = strlen(separator);
	char* string;

	for (i = 0; i < count; i++)
	{
		length += strlen(array[i]);
		if (i < count - 1)
		{
			length += separatorLength;
		}
	}
	string = new char[length + 1];
	string[0] = 0;
	for (i = 0; i < count - 1; i++)
	{
		strcat(string, array[i]);
		strcat(string, separator);
	}
	if (count)
	{
		strcat(string, array[count - 1]);
	}
	return string;
}

bool stringHasCaseInsensitivePrefix(const char* string, const char* prefix)
{
	int i;

	for (i = 0; string[i] && prefix[i] &&
		toupper(string[i]) == toupper(prefix[i]); i++)
	{
	}
	return prefix[i] == 0;
}

bool stringHasCaseInsensitivePrefix(const wchar_t* string,
	const wchar_t* prefix)
{
	int i;

	for (i = 0; string[i] && prefix[i] &&
		toupper(string[i]) == toupper(prefix[i]); i++)
	{
	}
	return prefix[i] == 0;
}

bool stringHasPrefix(const char* string, const char* prefix)
{
	int i;

	for (i = 0; string[i] && prefix[i] && string[i] == prefix[i]; i++)
	{
	}
	return prefix[i] == 0;
}

bool stringHasCaseInsensitiveSuffix(const char* string, const char* suffix)
{
	int i;
	int len1 = strlen(string);
	int len2 = strlen(suffix);

	for (i = 0; i < len1 && i < len2 &&
		toupper(string[len1 - i - 1]) == toupper(suffix[len2 - i - 1]); i++)
	{
	}
	return i == len2;
}

bool stringHasSuffix(const char* string, const char* suffix)
{
	int i;
	int len1 = strlen(string);
	int len2 = strlen(suffix);

	for (i = 0; i < len1 && i < len2 &&
		string[len1 - i - 1] == suffix[len2 - i - 1]; i++)
	{
	}
	return i == len2;
}

bool stringHasSuffix(const wchar_t* string, const wchar_t* suffix)
{
	int i;
	int len1 = wcslen(string);
	int len2 = wcslen(suffix);

	for (i = 0; i < len1 && i < len2 &&
		string[len1 - i - 1] == suffix[len2 - i - 1]; i++)
	{
	}
	return i == len2;
}

char* convertStringToUpper(char* string)
{
	int length = strlen(string);
	int i;

	for (i = 0; i < length; i++)
	{
		string[i] = (char)toupper(string[i]);
	}
	return string;
}

char* convertStringToLower(char* string)
{
	int length = strlen(string);
	int i;

	for (i = 0; i < length; i++)
	{
		string[i] = (char)tolower(string[i]);
	}
	return string;
}

char* findExecutable(const char* executable)
{
	char *path = getenv("PATH");
	char *retValue = NULL;
	int pathCount;
	char **pathComponents = componentsSeparatedByString(path, ":", pathCount);
	int i;

	for (i = 0; i < pathCount && retValue == NULL; i++)
	{
		FILE *file;

		retValue = copyString(pathComponents[i], 7);
		strcat(retValue, "/");
		strcat(retValue, executable);
		file = fopen(retValue, "r");
		if (file)
		{
			fclose(file);
		}
		else
		{
			delete retValue;
			retValue = NULL;
		}
	}
	deleteStringArray(pathComponents, pathCount);
	return retValue;
}

bool isRelativePath(const char* path)
{
#ifdef WIN32
	if (strlen(path) >= 3 && isalpha(path[0]) && path[1] == ':' &&
		(path[2] == '\\' || path[2] == '/'))
	{
		return false;
	}
	return true;
#else // WIN32
	if (path[0] == '/')
	{
		return false;
	}
	return true;
#endif // WIN32
}

char* findRelativePath(const char* cwd, const char* path)
{
	int lastSlash = 0;
	int cwdLen = strlen(cwd);
	int pathLen = strlen(path);
	char *fixedCwd;
	char *fixedPath;
	char **cwdComponents;
	int cwdCount;
	int dotDotCount;
	char *retValue;
	const char *diffSection;
	int i;

	if (isRelativePath(cwd))
	{
		// cwd must be a full path.
		return NULL;
	}
	if (isRelativePath(path))
	{
#ifdef WIN32
		if (path[0] == '/' || path[0] == '\\')
		{
			// In Windows, it's not an absolute path if it doesn't start with a
			// drive letter followed by a colon.  However, it's not truly a
			// relative path either, since you can't tag it on to cwd and have
			// it work.
			return NULL;
		}
#endif // WIN32
		// If path is already relative, it should be relative to cwd; return it.
		return copyString(path);
	}
#ifdef WIN32
	if (strncasecmp(cwd, path, 2) != 0)
	{
		// The two paths are on different drives; relative path is impossible.
		return NULL;
	}
#endif // WIN32
	fixedCwd = copyString(cwd);
	fixedPath = copyString(path);
#ifdef WIN32
	replaceStringCharacter(fixedCwd, '\\', '/');
	convertStringToLower(fixedCwd);
	replaceStringCharacter(fixedPath, '\\', '/');
	convertStringToLower(fixedPath);
#endif // WIN32
	for (i = 0; i < cwdLen && i < pathLen; i++)
	{
		if (fixedCwd[i] == '/' && fixedPath[i] == '/')
		{
			lastSlash = i;
		}
		else if (fixedCwd[i] != fixedPath[i])
		{
			break;
		}
	}
	cwdComponents = componentsSeparatedByString(&fixedCwd[lastSlash], "/",
		cwdCount);
	dotDotCount = cwdCount - 2;	// There's a / at the beginning and end.
	diffSection = &path[lastSlash + 1];
	retValue = new char[dotDotCount * 3 + strlen(diffSection) + 1];
	for (i = 0; i < dotDotCount; i++)
	{
		strcpy(&retValue[i * 3], "../");
	}
	strcpy(&retValue[i * 3], diffSection);
	deleteStringArray(cwdComponents, cwdCount);
	delete fixedCwd;
	delete fixedPath;
	return retValue;
}

char* directoryFromPath(const char* path)
{
	if (path)
	{
		const char* slashSpot = strrchr(path, '/');
#ifdef WIN32
		const char* backslashSpot = strrchr(path, '\\');

		if (backslashSpot > slashSpot)
		{
			slashSpot = backslashSpot;
		}
#endif // WIN32
		if (slashSpot)
		{
			int length = slashSpot - path;
			char* directory = new char[length + 1];

			strncpy(directory, path, length);
			directory[length] = 0;
			return directory;
		}
		else
		{
			return copyString("");
		}
	}
	else
	{
		return NULL;
	}
}

char* cleanedUpPath(const char* path)
{
	char *newPath = copyString(path);

#ifdef WIN32
	replaceStringCharacter(newPath, '\\', '/');
#endif // WIN32
	if (strstr(newPath, "../"))
	{
		char **pathComponents;
		int pathCount;
		//int newCount;
		std::stack<std::string> pathStack;
		std::list<std::string> pathList;
		std::list<std::string>::const_iterator it;
		int len = 1;	// The terminating NULL.
		int offset = 0;

		pathComponents = componentsSeparatedByString(newPath, "/", pathCount);
		// Note that we're intentionally skipping the first component.  That's
		// either empty (for a Unix path), or the drive letter followed by a
		// colon (for a Windows path).  We'll put it back later, though.
		for (int i = 1; i < pathCount; i++)
		{
			if (strcmp(pathComponents[i], "..") == 0)
			{
				if (pathStack.size() > 0)
				{
					pathStack.pop();
				}
			}
			else
			{
				pathStack.push(pathComponents[i]);
			}
		}
		while (pathStack.size() > 0)
		{
			pathList.push_front(pathStack.top());
			pathStack.pop();
			len += pathList.front().size() + 1;
		}
		len += strlen(pathComponents[0]);
		delete newPath;
		newPath = new char[len];
		strcpy(newPath, pathComponents[0]);
		offset = strlen(newPath);
		for (it = pathList.begin(); it != pathList.end(); it++)
		{
			// The following line leaves the string without a NULL terminator,
			// but the line after that puts one back.
			newPath[offset++] = '/';
			strcpy(&newPath[offset], it->c_str());
			offset += it->size();
		}
		deleteStringArray(pathComponents, pathCount);
	}
#ifdef WIN32
	replaceStringCharacter(newPath, '/', '\\');
#endif // WIN32
	return newPath;
}

static size_t lastSlashIndex(const ucstring &path)
{
	size_t slashSpot = path.rfind('/');
#ifdef WIN32
	size_t backslashSpot = path.rfind('\\');

	if (slashSpot >= path.size() ||
		(backslashSpot < path.size() && backslashSpot > slashSpot))
	{
		slashSpot = backslashSpot;
	}
#endif // WIN32
	return slashSpot;
}

TCExport ucstring filenameFromPath(const ucstring &path)
{
	size_t slashSpot = lastSlashIndex(path);
	if (slashSpot < path.size())
	{
		return path.substr(slashSpot + 1);
	}
	else
	{
		return path;
	}
}

TCExport ucstring directoryFromPath(const ucstring &path)
{
	size_t slashSpot = lastSlashIndex(path);
	if (slashSpot < path.size())
	{
		return path.substr(0, slashSpot);
	}
	else
	{
		return _UC("");
	}
}

char* filenameFromPath(const char* path)
{
	if (path)
	{
		const char* slashSpot = strrchr(path, '/');
#ifdef WIN32
		const char* backslashSpot = strrchr(path, '\\');

		if (backslashSpot > slashSpot)
		{
			slashSpot = backslashSpot;
		}
#endif // WIN32
		if (slashSpot)
		{
			slashSpot++;
			return copyString(slashSpot);
		}
		else
		{
			return copyString(path);
		}
	}
	else
	{
		return NULL;
	}
}

void stripCRLF(char* line)
{
	if (line)
	{
		int length = strlen(line);

		while (length > 0 && (line[length-1] == '\r' || line[length-1] == '\n'))
		{
			line[--length] = 0;
		}
	}
}

void stripCRLF(wchar_t* line)
{
	if (line)
	{
		int length = wcslen(line);

		while (length > 0 && (line[length-1] == '\r' || line[length-1] == '\n'))
		{
			line[--length] = 0;
		}
	}
}

void stripLeadingWhitespace(char* string)
{
	char *spot;

	for (spot = string; spot[0] == ' ' || spot[0] == '\t'; spot++)
		;
	if (spot[0])
	{
		if (spot != string)
		{
			memmove(string, spot, strlen(spot) + 1);
		}
	}
	else
	{
		string[0] = 0;
	}
}

void stripLeadingWhitespace(wchar_t* string)
{
	wchar_t *spot;

	for (spot = string; spot[0] == ' ' || spot[0] == '\t'; spot++)
		;
	if (spot[0])
	{
		if (spot != string)
		{
			memmove(string, spot, (wcslen(spot) + 1) * sizeof(wchar_t));
		}
	}
	else
	{
		string[0] = 0;
	}
}

void stripTrailingWhitespace(char* string)
{
	if (string)
	{
		int length = strlen(string);

		while (length > 0 && (string[length - 1] == ' ' ||
			string[length - 1] == '\t'))
		{
			string[--length] = 0;
		}
	}
}

void stripTrailingWhitespace(wchar_t* string)
{
	if (string)
	{
		int length = wcslen(string);

		while (length > 0 && (string[length - 1] == ' ' ||
			string[length - 1] == '\t'))
		{
			string[--length] = 0;
		}
	}
}

void stripTrailingPathSeparators(char* path)
{
	if (path)
	{
		int length = strlen(path);

#ifdef WIN32
		while (length > 0 && (path[length-1] == '\\' || path[length-1] == '/'))
#else
		while (length > 0 && path[length-1] == '/')
#endif
		{
			path[--length] = 0;
		}
	}
}

void replaceStringCharacter(char* string, char oldChar, char newChar, int 
repeat)
{
	char* spot;

	if ((spot = strchr(string, oldChar)) != NULL)
	{
		*spot = newChar;
		if (repeat)
		{
			while ((spot = strchr(spot, oldChar)) != NULL)
			{
				*spot = newChar;
			}
		}
	}
}

char *stringByReplacingSubstring(const char* string, const char* oldSubstring,
								 const char* newSubstring, bool repeat)
{
	char *newString = NULL;

	if (repeat)
	{
		int count;
		char **components = componentsSeparatedByString(string, oldSubstring,
			count);

		newString = componentsJoinedByString(components, count,
			newSubstring);
		deleteStringArray(components, count);
	}
	else
	{
		const char *oldSpot = strstr(string, oldSubstring);

		if (oldSpot)
		{
			int oldSubLength = strlen(oldSubstring);
			int newSubLength = strlen(newSubstring);
			int preLength = oldSpot - string;

			newString = new char[strlen(string) + newSubLength - oldSubLength +
				1];
			strncpy(newString, string, preLength);
			strcpy(newString + preLength, newSubstring);
			strcpy(newString + preLength + newSubLength,
				string + preLength + oldSubLength);
		}
		else
		{
			newString = new char[strlen(string) + 1];
			strcpy(newString, string);
		}
	}
	return newString;
}

int countStringLines(const char* string)
{
	int len = strlen(string);
	int count = 0;
	char *spot = (char*)string;

	if (string[len - 1] != '\n')
	{
		count++;
	}
	while ((spot = strchr(spot, '\n')) != NULL)
	{
		count++;
		spot++;
	}
	return count;
}

void setDebugLevel(int value)
{
	debugLevel = value;
}

int getDebugLevel(void)
{
	return debugLevel;
}

void indentPrintf(int indent, const char *format, ...)
{
	int i;
	va_list argPtr;

	for (i = 0; i < indent; i++)
	{
		printf(" ");
	}
	va_start(argPtr, format);
	vprintf(format, argPtr);
	va_end(argPtr);
}

void consoleVPrintf(const char *format, va_list argPtr)
{
#ifdef WIN32
	if (g_haveConsole)
	{
#ifdef TC_NO_UNICODE
		vprintf(format, argPtr);
		fflush(stdout);
#else // TC_NO_UNICODE
		static std::string temp;
		std::wstring wtemp;
		DWORD bytesWritten;

#if _MSC_VER < 1400	// VC < VC 2005
		int size;
		temp.resize(65536);
		size = vsprintf(&temp[0], format, argPtr);
		temp.resize(size);
#else
		temp.resize(_vscprintf(format, argPtr));
		vsprintf(&temp[0], format, argPtr);
#endif
		stringtowstring(wtemp, temp);
		if (g_bRealConsole)
		{
			// g_bRealConsole means we're running from an actual console app,
			// not a child process of the launcher app.
			WriteConsoleW(g_hStdOut, wtemp.c_str(), wtemp.size(), &bytesWritten, NULL);
		}
		else
		{
			WriteFile(g_hStdOut, wtemp.c_str(), wtemp.size() * 2, &bytesWritten, NULL);
			FlushFileBuffers(g_hStdOut);
		}
#endif // TC_NO_UNICODE
	}
	else
	{
		g_consoleBuffer.vprintf(format, argPtr);
	}
#else // WIN32
	vprintf(format, argPtr);
#endif
}

void consoleVPrintf(const wchar_t *format, va_list argPtr)
{
#ifdef WIN32
	if (g_haveConsole)
	{
#ifdef TC_NO_UNICODE
		vwprintf(format, argPtr);
		fflush(stdout);
#else // TC_NO_UNICODE
		std::wstring wtemp;
		DWORD bytesWritten;

#if _MSC_VER < 1400	// VC < VC 2005
		int size;
		wtemp.resize(65536);
		size = vswprintf(&wtemp[0], format, argPtr);
		wtemp.resize(size);
#else
		wtemp.resize(_vscwprintf(format, argPtr));
		// Note: second arg is size of buffer.  Buffer in std::string has one
		// extra character for the terminating NULL.
		vswprintf(&wtemp[0], wtemp.size() + 1, format, argPtr);
#endif
		if (g_bRealConsole)
		{
			// g_bRealConsole means we're running from an actual console app,
			// not a child process of the launcher app.
			WriteConsoleW(g_hStdOut, wtemp.c_str(), wtemp.size(), &bytesWritten, NULL);
		}
		else
		{
			WriteFile(g_hStdOut, wtemp.c_str(), wtemp.size() * 2, &bytesWritten, NULL);
			FlushFileBuffers(g_hStdOut);
		}
#endif // TC_NO_UNICODE
	}
	else
	{
		g_consoleBuffer.vwprintf(format, argPtr);
	}
#else // WIN32
#ifdef NO_WSTRING
	printf("wprintf attempted.\n");
#else // NO_WSTRING
	vwprintf(format, argPtr);
#endif // NO_WSTRING
#endif
}

void consolePrintf(const char *format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	consoleVPrintf(format, argPtr);
	va_end(argPtr);
}

void consolePrintf(const wchar_t *format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	consoleVPrintf(format, argPtr);
	va_end(argPtr);
}

void debugVPrintf(int level, const char *format, va_list argPtr)
{
	if (debugLevel >= level)
	{
#ifdef WIN32
		if (g_haveConsole)
		{
			consoleVPrintf(format, argPtr);
			return;
		}
#endif // WIN32
		vprintf(format, argPtr);
		fflush(stdout);
	}
}

void debugPrintf(const char *format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	debugVPrintf(1, format, argPtr);
	va_end(argPtr);
}

void debugPrintf(int level, const char *format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	debugVPrintf(level, format, argPtr);
	va_end(argPtr);
}

#ifndef WIN32

char *prettyLongLongString(long long num)
{
	char backwards[256];
	char *forwards;
	int i, j;

	for (i = 0; num; i++)
	{
		if (!((i + 1) % 4))
		{
			backwards[i++] = ',';
		}
		backwards[i] = (num % 10) + '0';
		num /= 10;
	}
	if (i == 0)
	{
		backwards[0] = '0';
		i = 1;
	}
	backwards[i] = 0;
	forwards = new char[(i + 1) * sizeof(char)];
	for (j = 0, i--; i >= 0; i--, j++)
	{
		forwards[j] = backwards[i];
	}
	forwards[j] = 0;
	return forwards;
}

long long longLongFromString(char* string)
{
	long long val;
	char* tmpString = NULL;
	int length = strlen(string);
	char* spot;

	while ((spot = strchr(string, ',')) != NULL)
	{
		int diff = spot - string;
		int needsDelete = 0;

		if (tmpString)
		{
			needsDelete = 1;
		}
		tmpString = new char[length];
		memcpy(tmpString, string, diff);
		tmpString[diff] = 0;
		strcat(tmpString, string + diff + 1);
		if (needsDelete)
		{
			delete string;
		}
		string = tmpString;
	}
	sscanf(string, "%lld", &val);
	if (tmpString)
	{
		delete tmpString;
	}
	return val;
}

#endif // WIN32

static const char *getEscapeString(char ch)
{
	switch (ch)
	{
	case '\a':
		return "\\a";
		break;
	case '\b':
		return "\\b";
		break;
	case '\f':
		return "\\f";
		break;
	case '\n':
		return "\\n";
		break;
	case '\r':
		return "\\r";
		break;
	case '\t':
		return "\\t";
		break;
	case '\v':
		return "\\v";
		break;
	case '\?':
		return "\\?";
		break;
	case '\\':
		return "\\\\";
		break;
	}
	return NULL;
}

static int escapeReplacement(char ch)
{
	switch (ch)
	{
	case 'a':
		return '\a';
		break;
	case 'b':
		return '\b';
		break;
	case 'f':
		return '\f';
		break;
	case 'n':
		return '\n';
		break;
	case 'r':
		return '\r';
		break;
	case 't':
		return '\t';
		break;
	case 'v':
		return '\v';
		break;
	case '?':
		return '\?';
		break;
	case '\'':
		return '\'';
		break;
	case '"':
		return '"';
		break;
	case '\\':
		return '\\';
		break;
	case '0':
		return '\0';
		break;
	}
	return -1;
}

static int escapeReplacement(wchar_t wch)
{
	return escapeReplacement((char)wch);
}

char *createEscapedString(const char *string)
{
	int i;
	int len = strlen(string);
	int tmpLen = 0;
	bool bFound = false;

	for (i = 0; i < len; i++)
	{
		const char *escapeString = getEscapeString(string[i]);

		if (escapeString)
		{
			tmpLen += strlen(escapeString);
			bFound = true;
		}
		else
		{
			tmpLen += 1;
		}
	}
	if (bFound)
	{
		char *retValue = new char[tmpLen + 1];

		tmpLen = 0;
		for (i = 0; i < len; i++)
		{
			const char *escapeString = getEscapeString(string[i]);

			if (escapeString)
			{
				strcpy(&retValue[tmpLen], escapeString);
				tmpLen += strlen(escapeString);
			}
			else
			{
				retValue[tmpLen] = string[i];
				tmpLen += 1;
			}
		}
		retValue[tmpLen] = 0;
		return retValue;
	}
	else
	{
		return copyString(string);
	}
}

void processEscapedString(char *string)
{
	int i;
	int len = strlen(string);
	int tmpLen = 0;
	char *tmpString = new char[len + 1];
	int lastSpot = 0;

	// Note we skip the last character, because even if it's a backslash, we
	// can't do anything with it.
	for (i = 0; i < len - 1; i++)
	{
		if (string[i] == '\\')
		{
			int replacement = escapeReplacement(string[i + 1]);

			if (replacement != -1)
			{
				if (i > lastSpot)
				{
					int count = i - lastSpot;

					strncpy(&tmpString[tmpLen], &string[lastSpot], count);
					tmpLen += count;
					lastSpot += count;
				}
				lastSpot += 2;
				tmpString[tmpLen++] = (char)replacement;
				i++;
			}
		}
	}
	strcpy(&tmpString[tmpLen], &string[lastSpot]);
	strcpy(string, tmpString);
	delete tmpString;
}

void processEscapedString(wchar_t *string)
{
	int i;
	int len = wcslen(string);
	int tmpLen = 0;
	wchar_t *tmpString = new wchar_t[len + 1];
	int lastSpot = 0;

	// Note we skip the last character, because even if it's a backslash, we
	// can't do anything with it.
	for (i = 0; i < len - 1; i++)
	{
		if (string[i] == '\\')
		{
			int replacement = escapeReplacement(string[i + 1]);

			if (replacement != -1)
			{
				if (i > lastSpot)
				{
					int count = i - lastSpot;

					wcsncpy(&tmpString[tmpLen], &string[lastSpot], count);
					tmpLen += count;
					lastSpot += count;
				}
				lastSpot += 2;
				tmpString[tmpLen++] = (wchar_t)replacement;
				i++;
			}
		}
	}
	wcscpy(&tmpString[tmpLen], &string[lastSpot]);
	wcscpy(string, tmpString);
	delete tmpString;
}

#ifdef TC_NO_UNICODE
void mbstoucstring(ucstring &dst, const char *src, int /*length*/)
#else
void mbstoucstring(ucstring &dst, const char *src, int length /*= -1*/)
#endif
{
#ifdef TC_NO_UNICODE
	dst = src;
#else
	mbstowstring(dst, src, length);
#endif
}

void mbstowstring(std::wstring &dst, const char *src, int length /*= -1*/)
{
	dst.erase();
	if (src)
	{
#ifndef NO_WSTRING
		mbstate_t state = { 0 };
#endif // !NO_WSTRING
		size_t newLength;

		if (length == -1)
		{
			length = strlen(src);
		}
		if (length > 0)
		{
			dst.resize(length);
			// Even though we don't check, we can't pass NULL instead of &state
			// and still be thread-safe.
#ifdef NO_WSTRING
			newLength = mbstowcs(&dst[0], src, length + 1);
#else // NO_WSTRING
			newLength = mbsrtowcs(&dst[0], &src, length + 1, &state);
#endif // NO_WSTRING
			if (newLength == (size_t)-1)
			{
				dst.resize(wcslen(&dst[0]));
			}
			else
			{
				dst.resize(newLength);
			}
		}
	}
}

#ifdef TC_NO_UNICODE
UCSTR mbstoucstring(const char *src, int /*length*/ /*= -1*/)
#else // TC_NO_UNICODE
UCSTR mbstoucstring(const char *src, int length /*= -1*/)
#endif // TC_NO_UNICODE
{
	if (src)
	{
#ifdef TC_NO_UNICODE
		return copyString(src);
#else // TC_NO_UNICODE
		std::wstring dst;
		mbstowstring(dst, src, length);
		return copyString(dst.c_str());
#endif // TC_NO_UNICODE
	}
	else
	{
		return NULL;
	}
}

#ifdef TC_NO_UNICODE
char *ucstringtombs(CUCSTR src, int /*length*/ /*= -1*/)
#else // TC_NO_UNICODE
char *ucstringtombs(CUCSTR src, int length /*= -1*/)
#endif // TC_NO_UNICODE
{
	if (src)
	{
#ifdef TC_NO_UNICODE
		return copyString(src);
#else // TC_NO_UNICODE
		std::string dst;
		wcstostring(dst, src, length);
		return copyString(dst.c_str());
#endif // TC_NO_UNICODE
	}
	else
	{
		return NULL;
	}
}

#ifdef TC_NO_UNICODE
char *ucstringtoutf8(CUCSTR src, int /*length*/ /*= -1*/)
#else // TC_NO_UNICODE
char *ucstringtoutf8(CUCSTR src, int length /*= -1*/)
#endif // TC_NO_UNICODE
{
	if (src)
	{
#ifdef TC_NO_UNICODE
		// This isn't 100% accurate, but we don't have much choice.
		return copyString(src);
#else // TC_NO_UNICODE
		UTF8 *dst;
		UTF8 *dstDup;
		UTF16 *src16;
		const UTF16 *src16Dup;
		int utf8Length;
		char *retValue = NULL;

		if (length == -1)
		{
			length = wcslen(src);
		}
		// I think every UTF-16 character can fit in 4 UTF-8 characters.
		// (Actually, hopefully it's less than that, but I'm trying to be safe.
		utf8Length = length * 4 + 1;
		dst = new UTF8[utf8Length];
		if (sizeof(wchar_t) == sizeof(UTF16))
		{
			src16 = (UTF16 *)src;
		}
		else
		{
			int i;

			src16 = new UTF16[length + 1];
			for (i = 0; i < length; i++)
			{
				src16[i] = (UTF16)src[i];
			}
			src16[length] = 0;
		}
		src16Dup = src16;
		dstDup = dst;
		// Note: length really is correct for end below, not length - 1.
		ConversionResult result = ConvertUTF16toUTF8(&src16Dup, &src16[length],
			&dstDup, &dst[utf8Length], lenientConversion);
		if (result == conversionOK)
		{
			utf8Length = dstDup - dst;
			retValue = new char[utf8Length + 1];
			memcpy(retValue, dst, utf8Length);
			retValue[utf8Length] = 0;
		}
		delete dst;
		if (src16 != (UTF16 *)src)
		{
			delete src16;
		}
		return retValue;
#endif // TC_NO_UNICODE
	}
	else
	{
		return NULL;
	}
}

#ifdef TC_NO_UNICODE
UCSTR utf8toucstring(const char *src, int /*length*/ /*= -1*/)
#else // TC_NO_UNICODE
UCSTR utf8toucstring(const char *src, int length /*= -1*/)
#endif // TC_NO_UNICODE
{
	if (src)
	{
#ifdef TC_NO_UNICODE
		// This isn't 100% accurate, but we don't have much choice.
		return copyString(src);
#else // TC_NO_UNICODE
		UTF16 *dst;
		UTF16 *dstDup;
		UTF8 *src8;
		const UTF8 *src8Dup;
		int utf16Length;
		wchar_t *retValue = NULL;

		if (length == -1)
		{
			length = strlen(src);
		}
		// I'm going to assume that the UTF-16 string has no more characters
		// than the UTF-8 one.
		utf16Length = length + 1;
		dst = new UTF16[utf16Length];
		if (sizeof(char) == sizeof(UTF8))
		{
			src8 = (UTF8 *)src;
		}
		else
		{
			int i;

			src8 = new UTF8[length + 1];
			for (i = 0; i < length; i++)
			{
				src8[i] = (UTF8)src[i];
			}
			src8[length] = 0;
		}
		src8Dup = src8;
		dstDup = dst;
		// Note: length really is correct for end below, not length - 1.
		ConversionResult result = ConvertUTF8toUTF16(&src8Dup, &src8[length],
			&dstDup, &dst[utf16Length], lenientConversion);
		if (result == conversionOK)
		{
			utf16Length = dstDup - dst;
			retValue = new wchar_t[utf16Length + 1];
			if (sizeof(wchar_t) == sizeof(UTF16))
			{
				memcpy(retValue, dst, utf16Length * sizeof(wchar_t));
			}
			else
			{
				int i;

				for (i = 0; i < utf16Length; i++)
				{
					retValue[i] = dst[i];
				}
			}
			retValue[utf16Length] = 0;
		}
		delete dst;
		if (src8 != (UTF8 *)src)
		{
			delete src8;
		}
		return retValue;
#endif // TC_NO_UNICODE
	}
	else
	{
		return NULL;
	}
}

void wcstostring(std::string &dst, const wchar_t *src, int length /*= -1*/)
{
	dst.erase();
	if (src)
	{
#ifndef NO_WSTRING
		mbstate_t state = { 0 };
#endif // !NO_WSTRING
		size_t newLength;

		if (length == -1)
		{
			length = wcslen(src);
		}
		length *= 2;
		dst.resize(length);
		// Even though we don't check, we can't pass NULL instead of &state and
		// still be thread-safe.
#ifdef NO_WSTRING
		newLength = wcstombs(&dst[0], src, length);
#else // NO_WSTRING
		newLength = wcsrtombs(&dst[0], &src, length, &state);
#endif // NO_WSTRING
		if (newLength == (size_t)-1)
		{
			dst.resize(strlen(&dst[0]));
		}
		else
		{
			dst.resize(newLength);
		}
	}
}

void wstringtostring(std::string &dst, const std::wstring &src)
{
#ifndef NO_WSTRING
	wcstostring(dst, src.c_str(), src.length());
#endif // NO_WSTRING
}

void stringtowstring(std::wstring &dst, const std::string &src)
{
	mbstowstring(dst, src.c_str(), src.length());
}

#ifdef WIN32

void runningWithConsole(bool bRealConsole /*= false*/)
{
	g_haveConsole = true;
	g_bRealConsole = bRealConsole;
	g_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
}

#endif // WIN32

#ifdef NO_WSTRING

unsigned long wcstoul(const wchar_t *start, wchar_t **end, int base)
{
	std::string temp;
	char *tempEnd;
	wcstostring(temp, start);
	unsigned long retValue = strtoul(temp.c_str(), &tempEnd, base);
	if (end != NULL)
	{
		*end = const_cast<wchar_t *>(&start[tempEnd - temp.c_str()]);
	}
	return retValue;
}

#endif // NO_WSTRING

ucstring ftoucstr(double value, int precision /*= 6*/)
{
#ifdef TC_NO_UNICODE
	return ftostr(value, precision);
#else // TC_NO_UNICODE
	std::string string = ftostr(value, precision);
	std::wstring wstring;

	stringtowstring(wstring, string);
	return wstring;
#endif // TC_NO_UNICODE
}

std::string ftostr(double value, int precision /*= 6*/)
{
	char buf[128];
	char format[128];
	int len;

	sprintf(format, "%%.%df", precision);
	sprintf(buf, format, value);
	len = strlen(buf);
	while (buf[len - 1] == '0')
	{
		buf[--len] = 0;
	}
	if (buf[len - 1] == '.')
	{
		buf[--len] = 0;
	}
	return buf;
}

ucstring ltoucstr(long value)
{
#ifdef TC_NO_UNICODE
	return ltostr(value);
#else // TC_NO_UNICODE
	std::string string = ltostr(value);
	std::wstring wstring;

	stringtowstring(wstring, string);
	return wstring;
#endif // TC_NO_UNICODE
}

std::string ltostr(long value)
{
	char buf[32];

	sprintf(buf, "%ld", value);
	return buf;
}
