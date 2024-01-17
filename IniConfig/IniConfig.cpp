#include "IniConfig.h"
#include "StringEncoding.h"
#include <fstream>
#include <sstream>
#include <unordered_set>

IniConfig::IniConfig(std::string name):fileName_(name)
{
	ReadConfigFile();
}
IniConfig::~IniConfig()
{

}

bool IniConfig::IsHeader(const std::string& s)
{
	if (s.length() < 3)
	{
		return false;
	}
	if (s[0] == ';' || s[0] == '#')
	{
		return false;
	}
	size_t start = s.find("[");
	size_t end = s.find("]");
	if (start != std::string::npos && end != std::string::npos && start < end)
	{
		return true;
	}
	return false;
}
bool IniConfig::IsConfig(const std::string& s)
{
	if (s.length() < 3)
	{
		return false;
	}
	if (s[0] == ';' || s[0] == '#')
	{
		return false;
	}
	if (s.find("=") == std::string::npos)
	{
		return false;
	}
	return true;
}
bool IniConfig::IsDigits(const std::string& s) {
	std::istringstream iss(s);
	double value;

	// 尝试从字符串中提取浮点数
	if (iss >> value) {
		// 检查提取后是否到达字符串末尾
		char c;
		if (iss >> c) {
			// 如果提取后仍有字符，则说明不是有效的浮点数格式
			return false;
		}
		else {
			// 提取成功且到达字符串末尾，说明是有效的浮点数格式
			return true;
		}
	}
	else {
		// 提取失败，说明不是有效的浮点数格式
		return false;
	}
}
void IniConfig::TrimCarriageReturn(std::string& s)
{
	if (s.empty())
	{
		return;
	}
	if (s[s.length() - 1] == '\r')
	{
		s = s.substr(0, s.length() - 1);
	}
}
void IniConfig::TrimCarriageReturn(std::vector<std::string>& lines)
{
	for (auto i = 0; i < lines.size(); ++i)
	{
		TrimCarriageReturn(lines[i]);
	}
}
void IniConfig::GetConfigValue(std::unordered_map<std::string, std::string>& config, const std::string& s)
{
	std::string key, value;
	size_t pos = s.find("=");
	if (pos + 1 < s.length())
	{
		key = s.substr(0, pos);
		value = s.substr(pos + 1, s.length() - pos + 1);
		config.insert(std::make_pair(TrimSpace(key), TrimSpace(value)));
	}
}
void IniConfig::SplitString(const std::string& origion, std::vector<std::string>& res, const std::string& spliter)
{
	std::string::size_type pos1, pos2;
	pos2 = origion.find(spliter);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		res.push_back(origion.substr(pos1, pos2 - pos1));
		pos1 = pos2 + spliter.size();
		pos2 = origion.find(spliter, pos1);
	}
	if (pos1 != origion.length())
		res.push_back(origion.substr(pos1));
}
bool IniConfig::ReadConfigFile()
{
	config_.clear();
	if (fileName_.empty())
	{
		throw (std::string)"The name of the profile cannot be left blank!";;
		return false;
	}
	std::string content = ReadFileString(fileName_.c_str());
	if (content.empty())
	{
		throw "File not found or file content is empty! <" + fileName_ + ">";
		return false;
	}
	std::string lastHeader;
	std::vector<std::string> lines;
	std::unordered_map<std::string, std::string> config;

	SplitString(content, lines, "\n");
	TrimCarriageReturn(lines);
	for (auto i = 0; i < lines.size(); ++i)
	{
		if (IsHeader(lines[i]))
		{
			if (!lastHeader.empty() && config.size() > 0)
			{
				config_.insert(std::make_pair(lastHeader, config));
				config.clear();
			}
			lastHeader = GetHeader(lines[i]);
			continue;
		}
		if (IsConfig(lines[i]))
		{
			GetConfigValue(config, lines[i]);
		}
	}
	if (!lastHeader.empty() && config.size() > 0)
	{
		config_.insert(std::make_pair(lastHeader, config));
	}
	return true;
}

std::string IniConfig::GetHeader(const std::string& s)
{
	size_t start = s.find("[");
	size_t end = s.find("]");
	if (start != std::string::npos && end != std::string::npos && start < end)
	{
		return s.substr(start + 1, end - start - 1);
	}
}
std::string IniConfig::TrimSpace(const std::string& s)
{
	std::size_t first = s.find_first_not_of(' ');
	if (first == std::string::npos) {
		return "";
	}

	std::size_t last = s.find_last_not_of(' ');
	return s.substr(first, last - first + 1);
}
std::string IniConfig::GetLowercase(const std::string& s)
{
	std::string result = s;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}
std::string IniConfig::GetUppercase(const std::string& s)
{
	std::string result = s;
	std::transform(result.begin(), result.end(), result.begin(), ::toupper);
	return result;
}
std::string IniConfig::GetStringConfig(std::string group, std::string key)
{
	if (config_.find(group) == config_.end())
	{
		throw "Can not find the section <"+ group + ">!";
		return "";
	}
	if (config_[group].find(key) == config_[group].end())
	{
		throw "Can not find the key <" + group + ":" + key + ">!";
		return "";
	}
	return config_[group][key];
}
int IniConfig::GetIntConfig(std::string group, std::string key)
{
	return DataTypeConvert<int>(GetStringConfig(group, key));
}
bool IniConfig::GetBoolConfig(std::string group, std::string key)
{
	std::string value = GetStringConfig(group, key);
	if (DataTypeConvert<int>(value) == 1)
	{
		return true;
	}
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	if (value == "true")
	{
		return true;
	}
	return false;
}