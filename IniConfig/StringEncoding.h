#pragma once
#include <vector>
#include <fstream>
#include <iostream>

enum StringEncoding
{
	StringEncoding_Empty = -2,
	StringEncoding_Unknown = -1,
	StringEncoding_ANSI = 0,
	StringEncoding_UTF8,
	StringEncoding_UTF8_BOM,
	StringEncoding_UTF16_LE,
	StringEncoding_UTF16_BE,
};

extern std::string ReadFileString(const char* fileName);
extern std::vector<unsigned char> ReadFile(const char* fileName);
extern bool WriteFile(const char* fileName, const std::string& data);
extern bool WriteFile(const char* fileName, const std::vector<unsigned char>& data);

extern bool IsUtf8Encoding(const std::string& s);
extern StringEncoding GetStringEncoding(const std::string& s);

extern std::string Gbk2Utf(const std::string& s);
extern std::string Utf2Gbk(const std::string& s);
extern std::string UtfBom2Utf(const std::string& s);
extern std::string Utf2UtfBom(const std::string& s);
extern std::string UcsLE2Utf(const std::string& s, bool mark = false);
extern std::string UcsBE2Utf(const std::string& s, bool mark = false);
extern std::string Utf2UcsLE(const std::string& s, bool mark = false);
extern std::string Utf2UcsBE(const std::string& s, bool mark = false);

//若字符串前几字节为编码标识，可自动识别编码并转换。若不存在不应调用下列5个函数
extern std::string Convert2Gbk(const std::string& s);
extern std::string Convert2Utf8(const std::string& s);
extern std::string Convert2Utf8Bom(const std::string& s);
extern std::string Convert2UcsLE(const std::string& s);
extern std::string Convert2UcsBE(const std::string& s);