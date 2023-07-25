#pragma once
#include <google/protobuf/service.h>
#include <memory>
#include <string>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/Buffer.h>
#include <muduo/base/Timestamp.h>
#include <unordered_map>

using namespace muduo::net;

//框架提供的专门负责发布rpc服务的网络对象类

class RpcProvider
{
public: 
    //这里是框架提供给外部使用的，可以发布rpc Service方法的函数接口
    void NotifyService(::google::protobuf::Service* service);
    //启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();

private:
    // 组合EventLoop
    EventLoop m_eventLoop;
    // 新的socket连接回调
    void onConnection(const TcpConnectionPtr&);
    // 已建立连接用户的读写事件回调
    void onMessage(const TcpConnectionPtr&,Buffer*,muduo::Timestamp);

    // 服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service* m_service;//保存服务对象
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_methodMap;//保存服务的方法
    };
    
    // 存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string,ServiceInfo> m_serviceMap;

    // closure done的回调函数，用于序列化rpc响应和网络发送
    void SendRpcResponse(const TcpConnectionPtr&conn, google::protobuf::Message*msg);
    
};