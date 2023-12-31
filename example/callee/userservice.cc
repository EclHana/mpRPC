#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"


using namespace fixbug;

/*
UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和GetFriendLists
*/

class UserService : public UserServiceRpc//使用在rpc服务发布端的（rpc服务的提供者）
{
public:
    bool Login(std::string name,std::string pwd)
    {
        std::cout<<"doing local service:Login" <<std::endl;
        std::cout<<"name:"<<name<<" pwd:"<<pwd<<std::endl;
        return true;
    }

    bool Register(int32_t id, std::string name, std::string pwd)
    {
        std::cout<<"doing local service:Register" <<std::endl;
        std::cout<<"id:"<<id<<" name:"<<name<<std::endl;
        return true;
    }

    /*
        重写基类UserServiceRpc的虚函数，下面这些方法都是框架直接调用的
        1.caller -> Login(LogRequest) >--muduo--> callee
        2.callee -> Login(LogRequest) >> 交给下面重写的这个方法上了
    */
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        // 1.框架给业务上报了请求参数LogRequest，应用从LoginRequest解析相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 2.本地调用服务的函数
        bool login_result = Login(name,pwd);

        // 3.把响应写入LoginResponse，交给框架来发送回去结果
        ResultCode* rc = response->mutable_result();
        rc->set_errcode(0);
        rc->set_errmsg("");
        response->set_success(login_result);

        // 4.执行回调操作，执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        int32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool result = Register(id,name,pwd);
        ResultCode* rc = response->mutable_result();
        rc->set_errcode(0);
        rc->set_errmsg("");
        response->set_success(result);

        done->Run();
    }

private:
};

int main(int argc, char** argv)
{
    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    //把UserService服务对象发布到rpc节点上,RpcProvider是一个rpc网络服务对象
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程rpc调用请求
    provider.Run();
}