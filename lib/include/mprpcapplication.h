#pragma once
#include "mprpcconfig.h"
#include "mprpccontroller.h"
#include "mprpcchannel.h"

//mprpc框架基础类（单例）
class MprpcApplication
{
public:
    static void Init(int argc, char** argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& getConfig();
    
private:
    MprpcApplication() = default;
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
    MprpcApplication& operator=(const MprpcApplication&) = delete;

    static MprpcConfig m_config;
};