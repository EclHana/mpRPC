#pragma once
#include "lockqueue.h"
#include <string>

// mprpc提供的日志系统
class Logger
{
public:
    static Logger& Instance();
    // 写日志
    void Log(std::string msg);
private:
    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;

    LockQueue<std::string> m_lckQue;//日志缓存队列
};

// 定义宏
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger& logger = Logger::Instance(); \
        char c[1024] = {"[INFO] "}; \
        snprintf(c+7,1024,logmsgformat,##__VA_ARGS__); \
        logger.Log(c); \
    } while (0);
    
#define LOG_ERROR(logmsgformat, ...) \
    do \
    {  \
        Logger& logger = Logger::Instance(); \
        char c[1024] = {"[\033[33mERRO\033[0m] "}; \
        snprintf(c+22,1024,logmsgformat,##__VA_ARGS__); \
        logger.Log(c); \
    } while (0);
    