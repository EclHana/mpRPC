#include "mprpcchannel.h"
#include <string>
#include "rpcheader.pb.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <error.h>
#include <memory>
#include "mprpcapplication.h"
#include "mprpccontroller.h"
#include "zookeeperutil.h"

/*
    header_size + service_name + method_name + args_size + args
*/

// 所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的 数据序列化 和 网络发送 
void MprpcChannel::CallMethod(const MethodDescriptor* method,
                    RpcController* controller, const Message* request,
                    Message* response, Closure* done)
{
    const ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name();  //service_name
    std::string method_name = method->name();//method_name

    // 获取参数的序列化字符串长度 args_size;
    uint32_t args_size = 0;
    std::string args_str;
    if(request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("request->SerializeToString error!");
        return;
    }

    // 定义mprpc的请求header
    mprpc::RpcHeader rpcheader;
    rpcheader.set_service_name(service_name);
    rpcheader.set_method_name(method_name);
    rpcheader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if(rpcheader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        controller->SetFailed("rpcheader.SerializeToString error!");
        return;
    }

    // 组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4));    //header_size
    send_rpc_str += rpc_header_str; //rpcheader
    send_rpc_str += args_str;   //args_str

    // 打印调试信息
    std::cout<<"-------------------------------------"<<std::endl;
    std::cout<<"header size:"<<header_size<<std::endl;
    std::cout<<"rpc_header_str:"<<rpc_header_str<<std::endl;
    std::cout<<"service_name:"<<service_name<<std::endl;
    std::cout<<"method_name:"<<method_name<<std::endl;
    std::cout<<"args_str:"<<args_str<<std::endl;
    std::cout<<"-------------------------------------"<<std::endl;



    //使用简单的tcp编程，发送，完成rpc方法的远程调用，不需要什么高并发，也就不用muduo

    // 使用智能指针管理fd
    std::shared_ptr<int> clientfd(new int(socket(AF_INET,SOCK_STREAM,0)),[](int* x)
    {
        close(*x);
        std::cout<<"clientfd:"<<*x<<" has been closed."<<std::endl;
        delete x;
    });

    if(-1==*clientfd)
    {
        controller->SetFailed("create socket error!");
        exit(EXIT_FAILURE);
    }
    // 读取配置文件rpcserver的信息
    // std::string ip = MprpcApplication::GetInstance().getConfig().Load("rpcserverip");
    // uint16_t port = stoi(MprpcApplication::GetInstance().getConfig().Load("rpcserverport"));
    
    
    // rpc调用方想要调用service_name的method_name服务，需要在zookeeper上查询到该服务的ip与port
    ZkClient zkCli;
    zkCli.Start();
    std::string method_path = "/"+service_name+"/"+method_name;
    std::string host_data = zkCli.GetData(method_path.c_str());
    if(host_data.empty())
    {
        controller->SetFailed(method_path+" is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if(idx==-1)
    {
        controller->SetFailed(method_path+" address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0,idx);
    uint16_t port = stoi(host_data.substr(idx+1));


    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if(-1 == connect(*clientfd, (struct sockaddr*)&server_addr,sizeof server_addr))
    {
        controller->SetFailed("connect error!");
        exit(EXIT_FAILURE);
    }
    if(-1==send(*clientfd, send_rpc_str.c_str(), send_rpc_str.size(),0))
    {
        controller->SetFailed("send error!");
        return;
    }
    // 接收rpc请求的响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if(-1==(recv_size = recv(*clientfd, recv_buf, 1024, 0)))
    {
        controller->SetFailed("recv error!");
        return;
    }

    // 反序列化rpc调用的响应数据
    // std::string response_str(recv_buf, 0, recv_size);   //出现问题：recv_buf中遇到\0后面的数据就存不下来了，导致反序列化失败
    
    if(!(response->ParseFromArray(recv_buf,recv_size)))
    {
        controller->SetFailed("parse error! response_str");
        return;
    }
}