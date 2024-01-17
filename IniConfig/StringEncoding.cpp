#include <windows.h>
#include "StringEncoding.h"

std::string ReadFileString(const char* fileName)
{
	//读取文件，并转换为GBK编码
	std::vector<unsigned char> fileContents = ReadFile(fileName);
	std::string content(fileContents.begin(), fileContents.end());
	return Convert2Gbk(content);
}
std::vector<unsigned char> ReadFile(const char* fileName)
{
	std::vector<unsigned char> fileContents;

	// 打开文件进行读取
	std::ifstream infile(fileName, std::ios::binary);
	if (infile) {
		// 获取文件大小
		infile.seekg(0, std::ios::end);
		std::streampos fileSize = infile.tellg();
		infile.seekg(0, std::ios::beg);

		// 读取文件内容到 vector
		fileContents.resize(fileSize);
		infile.read(reinterpret_cast<char*>(&fileContents[0]), fileSize);
		infile.close();
	}
	return fileContents;
}
bool WriteFile(const char* fileName, const std::string& data)
{
	std::vector<unsigned char> writeContent(data.begin(), data.end());
	return WriteFile(fileName, writeContent);
}
bool WriteFile(const char* fileName, const std::vector<unsigned char>& data) 
{
	std::ofstream outfile(fileName, std::ios::out | std::ios::binary); // 以二进制模式打开文件
	if (outfile.is_open()) 
	{
		outfile.write(reinterpret_cast<const char*>(data.data()), data.size()); // 将数据写入文件
		outfile.close(); // 关闭文件
		return true;
	}
	return false;
}

bool IsUtf8Encoding(const std::string& s)
{
	int count = 0;
	unsigned char single;
	bool allAscii = true;
	unsigned long len = s.size();

	//0x00-0x7F为ASCII码范围
	for (unsigned long uIndex = 0; uIndex < len; ++uIndex)
	{
		single = s[uIndex];

		if ((single & 0x80) != 0)
			allAscii = false;

		if (count == 0)
		{
			if (single >= 0x80)
			{
				if (single >= 0xFC && single <= 0xFD)
					count = 6;
				else if (single >= 0xF8)
					count = 5;
				else if (single >= 0xF0)
					count = 4;
				else if (single >= 0xE0)
					count = 3;
				else if (single >= 0xC0)
					count = 2;
				else
					return false;

				count--;
			}
		}
		else
		{
			if ((single & 0xC0) != 0x80)//在UTF-8中，以位模式10开始的所有字节是多字节序列的后续字节
				return false;

			count--;
		}
	}

	if (count > 0)
		return false;

	if (allAscii)
		return false;

	return true;
}
StringEncoding GetStringEncoding(const std::string& s)
{
	/*
	ANSI                     无格式定义							对于中文编码格式是GB2312;
	Unicode little endian    文本里前两个字节为FF FE			字节流是little endian
	Unicode big endian       文本里前两个字节为FE FF			字节流是big endian
	UTF-8带BOM               前两字节为EF BB，第三字节为BF		带BOM
	UTF-8不带BOM             无格式定义,需另加判断				不带BOM
	*/
	if (s.empty())
	{
		return StringEncoding_Empty;
	}
	if (s.length() == 1)
	{
		return StringEncoding_ANSI;
	}
	StringEncoding fe;
	int nHead = ((unsigned char)s[0] << 8) + (unsigned char)s[1];

	switch (nHead)
	{
	case 0xFFFE:
		fe = StringEncoding_UTF16_LE;
		break;
	case 0xFEFF:
		fe = StringEncoding_UTF16_BE;
		break;
	case 0xEFBB:
		fe = StringEncoding_UTF8_BOM;
		break;
	default:
	{
		if (IsUtf8Encoding(s))
			fe = StringEncoding_UTF8;
		else
			fe = StringEncoding_ANSI;
		break;
	}
	}
	return fe;
}

std::string Gbk2Utf(const std::string& s)
{
	int gbkLength = static_cast<int>(s.length());
	int utf8Length = MultiByteToWideChar(CP_ACP, 0, s.c_str(), gbkLength, NULL, 0);
	if (utf8Length > 0)
	{
		std::wstring utf16String(utf8Length, L'\0');
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), gbkLength, &utf16String[0], utf8Length);
		int utf8BytesLength = WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), utf8Length, NULL, 0, NULL, NULL);
		if (utf8BytesLength > 0)
		{
			std::string utf8String(utf8BytesLength, '\0');
			WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), utf8Length, &utf8String[0], utf8BytesLength, NULL, NULL);
			return utf8String;
		}
	}
	return "";
}
std::string Utf2Gbk(const std::string& s)
{
	int utf8Length = static_cast<int>(s.length());
	int utf16Length = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), utf8Length, NULL, 0);
	if (utf16Length > 0)
	{
		std::wstring utf16String(utf16Length, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, s.c_str(), utf8Length, &utf16String[0], utf16Length);
		int gbkBytesLength = WideCharToMultiByte(CP_ACP, 0, utf16String.c_str(), utf16Length, NULL, 0, NULL, NULL);
		if (gbkBytesLength > 0)
		{
			std::string gbkString(gbkBytesLength, '\0');
			WideCharToMultiByte(CP_ACP, 0, utf16String.c_str(), utf16Length, &gbkString[0], gbkBytesLength, NULL, NULL);
			return gbkString;
		}
	}
	return "";
}
std::string UtfBom2Utf(const std::string& s)
{
	if (s.length() >= 3 && static_cast<unsigned char>(s[0]) == 0xEF
		&& static_cast<unsigned char>(s[1]) == 0xBB && static_cast<unsigned char>(s[2]) == 0xBF)
	{
		return s.substr(3);
	}
	return s;
}
std::string Utf2UtfBom(const std::string& s)
{
	return "\xEF\xBB\xBF" + s;
}
std::string UcsLE2Utf(const std::string& s, bool mark)
{
	std::string ucsString = s;
	if (mark)
	{
		ucsString = ucsString.substr(2);
	}
	if (ucsString.length() % 2 != 0 || s.length() < 0)
	{
		return "";
	}
	std::vector<unsigned short> wideVec(ucsString.length() / 2, 0);
	for (size_t i = 0; i < wideVec.size(); i++)
	{
		wideVec[i] = static_cast<unsigned short>(static_cast<unsigned char>(ucsString[i * 2])) |
			(static_cast<unsigned short>(static_cast<unsigned char>(ucsString[i * 2 + 1])) << 8);
	}
	std::wstring utf16String(wideVec.begin(), wideVec.end());
	int utf8Length = WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), static_cast<int>(utf16String.length()), NULL, 0, NULL, NULL);
	if (utf8Length > 0)
	{
		std::string utf8String(utf8Length, '\0');
		WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), static_cast<int>(utf16String.length()), &utf8String[0], utf8Length, NULL, NULL);
		return utf8String;
	}
	return "";
}
std::string UcsBE2Utf(const std::string& s, bool mark)
{
	std::string ucsString = s;
	if (mark)
	{
		ucsString = ucsString.substr(2);
	}
	if (ucsString.length() % 2 != 0 || s.length() < 0)
	{
		return "";
	}
	std::vector<unsigned short> wideVec(ucsString.length() / 2, 0);
	for (size_t i = 0; i < wideVec.size(); i++)
	{
		wideVec[i] = (static_cast<unsigned short>(static_cast<unsigned char>(ucsString[i * 2])) << 8) |
			static_cast<unsigned short>(static_cast<unsigned char>(ucsString[i * 2 + 1]));
	}
	std::wstring utf16String(wideVec.begin(), wideVec.end());
	int utf8Length = WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), static_cast<int>(utf16String.length()), NULL, 0, NULL, NULL);
	if (utf8Length > 0)
	{
		std::string utf8String(utf8Length, '\0');
		WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), static_cast<int>(utf16String.length()), &utf8String[0], utf8Length, NULL, NULL);
		return utf8String;
	}
	return "";
}
std::string Utf2UcsLE(const std::string& s, bool mark)
{
	int utf16Length = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.length()), NULL, 0);
	if (utf16Length > 0)
	{
		std::vector<unsigned short> wideVec(utf16Length, 0);
		MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.length()), reinterpret_cast<LPWSTR>(&wideVec[0]), utf16Length);
		std::string ucs2leStr(wideVec.size() * 2, 0);
		for (size_t i = 0; i < wideVec.size(); i++)
		{
			ucs2leStr[i * 2] = static_cast<char>(wideVec[i] & 0xFF);
			ucs2leStr[i * 2 + 1] = static_cast<char>((wideVec[i] >> 8) & 0xFF);
		}
		if (mark)
		{
			return "\xFF\xFE" + ucs2leStr;
		}
		else
		{
			return ucs2leStr;
		}
	}
	return "";
}
std::string Utf2UcsBE(const std::string& s, bool mark)
{
	int utf16Length = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.length()), NULL, 0);
	if (utf16Length > 0)
	{
		std::vector<unsigned short> wideVec(utf16Length, 0);
		MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.length()), reinterpret_cast<LPWSTR>(&wideVec[0]), utf16Length);
		std::string ucs2beStr(wideVec.size() * 2, 0);
		for (size_t i = 0; i < wideVec.size(); i++)
		{
			ucs2beStr[i * 2] = static_cast<char>((wideVec[i] >> 8) & 0xFF);
			ucs2beStr[i * 2 + 1] = static_cast<char>(wideVec[i] & 0xFF);
		}
		if(mark)
		{
			return "\xFE\xFF" + ucs2beStr;
		}
		else
		{
			return ucs2beStr;
		}
		}
	return "";
}

//若字符串前几字节为编码标识，可自动识别编码并转换。若不存在编码标识不应调用下列5个函数
std::string Convert2Gbk(const std::string& s)
{
	std::string result = s;
	switch (GetStringEncoding(s))
	{
	case StringEncoding_ANSI:
		break;
	case StringEncoding_UTF8:
		result = Utf2Gbk(s);
		break;
	case StringEncoding_UTF8_BOM:
		result = Utf2Gbk(UtfBom2Utf(s));
		break;
	case StringEncoding_UTF16_LE:
		result = Utf2Gbk(UcsLE2Utf(s, true));
		break;
	case StringEncoding_UTF16_BE:
		result = Utf2Gbk(UcsBE2Utf(s, true));
		break;
	}
	return result;
}
std::string Convert2Utf8(const std::string& s)
{
	std::string result = s;
	switch (GetStringEncoding(s))
	{
	case StringEncoding_ANSI:
		result = Gbk2Utf(s);
		break;
	case StringEncoding_UTF8:
		break;
	case StringEncoding_UTF8_BOM:
		result = UtfBom2Utf(s);
		break;
	case StringEncoding_UTF16_LE:
		result = UcsLE2Utf(s, true);
		break;
	case StringEncoding_UTF16_BE:
		result = UcsBE2Utf(s, true);
		break;
	}
	return result;
}
std::string Convert2Utf8Bom(const std::string& s)
{
	std::string result = s;
	switch (GetStringEncoding(s))
	{
	case StringEncoding_ANSI:
		result = Utf2UtfBom(Gbk2Utf(s));
		break;
	case StringEncoding_UTF8:
		result = Utf2UtfBom(s);
		break;
	case StringEncoding_UTF8_BOM:
		break;
	case StringEncoding_UTF16_LE:
		result = Utf2UtfBom(UcsLE2Utf(s, true));
		break;
	case StringEncoding_UTF16_BE:
		result = Utf2UtfBom(UcsBE2Utf(s, true));
		break;
	}
	return result;
}
std::string Convert2UcsLE(const std::string& s)
{
	std::string result = s;
	switch (GetStringEncoding(s))
	{
	case StringEncoding_ANSI:
		result = Utf2UcsLE(Gbk2Utf(s));
		break;
	case StringEncoding_UTF8:
		result = Utf2UcsLE(s);
		break;
	case StringEncoding_UTF8_BOM:
		result = Utf2UcsLE(UtfBom2Utf(s));
		break;
	case StringEncoding_UTF16_LE:
		break;
	case StringEncoding_UTF16_BE:
		result = Utf2UcsLE(UcsBE2Utf(s, true));
		break;
	}
	return result;
}
std::string Convert2UcsBE(const std::string& s)
{
	std::string result = s;
	switch (GetStringEncoding(s))
	{
	case StringEncoding_ANSI:
		result = Utf2UcsBE(Gbk2Utf(s));
		break;
	case StringEncoding_UTF8:
		result = Utf2UcsBE(s);
		break;
	case StringEncoding_UTF8_BOM:
		result = Utf2UcsBE(UtfBom2Utf(s));
		break;
	case StringEncoding_UTF16_LE:
		result = Utf2UcsBE(UcsLE2Utf(s, true));
		break;
	case StringEncoding_UTF16_BE:
		break;
	}
	return result;
}