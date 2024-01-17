#include "IniConfig.h"
#include <iostream>

/* test.ini格式
[Project]
ProjectPath = D:/Program Files/Steam
ShareProject = true

[DataCollect]
RequestFrequency = 10000
*/
void main()
{
	try
	{
		//解析配置文件
		IniConfig config("test.ini");

		//读取配置
		bool valueBool = config.GetBoolConfig("Project", "ShareProject");
		int valueInt = config.GetIntConfig("DataCollect", "RequestFrequency");
		std::string valueString = config.GetStringConfig("Project", "ProjectPath");

		//输出配置信息
		std::cout << "Project->ShareProject: " << valueBool << std::endl;
		std::cout << "DataCollect->RequestFrequency: " << valueInt << std::endl;
		std::cout << "Project->ProjectPath: " << valueString << std::endl;
	}
	catch (const std::string& msg)
	{
		//输出错误信息
		std::cout << msg << std::endl;
	}
	std::cin.get();
}