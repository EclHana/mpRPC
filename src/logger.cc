#include "logger.h"
#include <time.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

Logger& Logger::Instance()
{
    static Logger logger;
    return logger;
}

Logger::Logger()
{
    std::thread writeLogTask([&](){
        while(1)
        {
            // 获取当天日期，然后取日志信息，写入相应的日志文件当中
            time_t now = time(nullptr);
            tm* nowtm = localtime(&now);
            char file_name[128] = {0};
            sprintf(file_name,"./log/%d-%d-%d-log.txt",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);
            if(access("./log", F_OK ) == -1 && mkdir("./log",S_IREAD|S_IWRITE)==-1)
            {
                std::cout<<"make log directory error!"<<std::endl;
                exit(EXIT_FAILURE);
            }
            
            FILE* pf = fopen(file_name, "a+");
            if(pf==nullptr)
            {
                std::cout<<"logger file: "<<file_name<<"open error!"<<std::endl;
                exit(EXIT_FAILURE);
            }
            std::string msg = m_lckQue.pop();

            char time_buf[128] = {0};
            sprintf(time_buf,"%d:%d:%d >> ",nowtm->tm_hour,nowtm->tm_min,nowtm->tm_sec);
            msg.insert(0, time_buf);
            msg.append("\n");

            fputs(msg.c_str(), pf);
            fclose(pf);
        }
    });
    writeLogTask.detach();
}

// 写日志
void Logger::Log(std::string msg)
{
    // 将日志信息放入lockqueue中
    m_lckQue.push(msg);
}
