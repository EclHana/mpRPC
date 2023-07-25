#include "mprpcconfig.h"
#include <iostream>
#include <unistd.h>
#include <string>


// 负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char* config_file)
{
    FILE* pf = fopen(config_file, "r");
    if(nullptr == pf)
    {
        std::cout<<"config_file do not exist!"<<std::endl;
        exit(EXIT_FAILURE);
    }
    // 1.注释 2.正确的配置项 3去掉开头多余空格
    while (!feof(pf))
    {
        char buf[512] = {0};
        fgets(buf, 512, pf);
        
        std::string read_buf(buf);
        //去掉前面多余空格
        Trim(read_buf);

        // 判断#注释
        if(read_buf[0]=='#' || read_buf.empty()) continue;

        // 解析配置项
        int idx = read_buf.find('=');
        if(idx==-1)
        {
            //配置不合法
            continue;
        }
        std::string key;
        std::string value;
        key = read_buf.substr(0,idx);
        Trim(key);
        int endidx = read_buf.find('\n',idx);
        value = read_buf.substr(idx+1,endidx-idx-1);
        Trim(value);
        m_configMap[key] = value;
    }
    fclose(pf);
}

// 查询配置项信息
std::string MprpcConfig::Load(const std::string& key)
{
    return m_configMap.count(key)==0 ? "" : m_configMap[key];
}

// 去掉字符串前后空格
void MprpcConfig::Trim(std::string& src_buf)
{
    int idx = src_buf.find_first_not_of(' ');
    if(idx!=-1)
    {
        src_buf = src_buf.substr(idx);
    }
    // 去掉后面的空格
    idx = src_buf.find_last_not_of(' ');
    if(idx!=-1)
    {
        src_buf=src_buf.substr(0,idx+1);
    }
}