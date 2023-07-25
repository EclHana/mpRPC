#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"


int main(int argc, char** argv)
{
    // 整个程序启动以后,想使用mprpc框架来享受rpc服务调用,一定要先调用框架的初始化函数(只初始化一次)
    MprpcApplication::Init(argc,argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    fixbug::LoginRequest request;       //填好请求函数的参数
    request.set_name("zhang san");
    request.set_pwd("123456");

    fixbug::LoginResponse response;// rpc方法响应 交给provider填充返回
    // 发起rpc方法调用，同步的rpc调用过程，-> MprcpChannel::CallMethod()
    stub.Login(nullptr,&request,&response,nullptr);// RpcChannel->RpcChannel->callMethod();集中来做所有rpc方法调用的参数序列化和网络发送

    // 一次rpc调用完成，读取调用响应的结果
    if(response.result().errcode()==0)
    {
        std::cout<<"rpc Login response success:"<<response.success()<<std::endl;
    }
    else
    {
        std::cout<<"rpc Login response error:"<<response.result().errmsg()<<std::endl;
    }

    fixbug::RegisterRequest regRequest;
    regRequest.set_id(622253);
    regRequest.set_name("mmm-p");
    regRequest.set_pwd("88556222");

    fixbug::RegisterResponse regResponse;
    stub.Register(nullptr,&regRequest,&regResponse,nullptr);
    if(regResponse.result().errcode()==0)
    {
        std::cout<<"rpc Register response success:"<<regResponse.success()<<std::endl;
    }
    else
    {
        std::cout<<"rpc Register response error:"<<regResponse.result().errmsg()<<std::endl;
    }

    return 0;
}