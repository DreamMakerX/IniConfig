#pragma once
#include <string>
#include <cctype>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

class IniConfig
{
private:
	std::string	fileName_;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> config_;
	
	bool IsHeader(const std::string& s);
	bool IsConfig(const std::string& s);
	bool IsDigits(const std::string& s);
	void TrimCarriageReturn(std::string& s);
	void TrimCarriageReturn(std::vector<std::string>& lines);
	void GetConfigValue(std::unordered_map<std::string, std::string>& config, const std::string& s);
	void SplitString(const std::string& origion, std::vector<std::string>& res, const std::string& spliter);
	bool ReadConfigFile();

	std::string GetHeader(const std::string& s);
	std::string TrimSpace(const std::string& s);
	std::string GetLowercase(const std::string& s);
	std::string GetUppercase(const std::string& s);

	template<class T>
	T DataTypeConvert(const std::string& value)
	{
		if (IsDigits(value))
		{
			return static_cast<T>(std::stod(value));
		}
		else
		{
			if (!GetLowercase(value).compare("true"))
			{
				return static_cast<T>(std::stod("1"));
			}
			else
			{
				return static_cast<T>(std::stod("0"));
			}
		}
	}
	template<class T>
	T DataTypeConvert(const char* value)
	{
		return DataTypeConvert<T>((std::string)value);
	}
	template<class T>
	std::string DataTypeConvert(const T& value)
	{
		std::stringstream ss;
		ss << value;
		return ss.str();
	}
public:
	IniConfig(std::string name);
	~IniConfig();

	std::string GetStringConfig(std::string group, std::string key);
	int GetIntConfig(std::string group, std::string key);
	bool GetBoolConfig(std::string group, std::string key);
};

