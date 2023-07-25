#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"

int main(int argc, char** argv)
{
    // 整个程序启动以后,想使用mprpc框架来享受rpc服务调用,一定要先调用框架的初始化函数(只初始化一次)
    MprpcApplication::Init(argc,argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());
    fixbug::GetFriendsListRequest request;       //填好请求函数的参数
    request.set_userid(1700);

    fixbug::GetFriendsListResponse response;// rpc方法响应 交给provider填充返回
    
    // 发起rpc方法调用，同步的rpc调用过程，-> MprcpChannel::CallMethod()

    MprpcController controller;//MprpcController作为一个错误消息的接收器，服务器调用过程中出错的信息全部由controller带回

    stub.GetFriendsList(&controller, &request, &response, nullptr);// RpcChannel->RpcChannel->callMethod();集中来做所有rpc方法调用的参数序列化和网络发送

    // 一次rpc调用完成，读取调用响应的结果
    if(controller.Failed()) 
    {
        std::cout<<controller.ErrorText()<<std::endl;
    }
    else
    {
        if(response.result().errcode()==0)
        {
            std::cout<<"rpc GetFriendsList response success:1"<<std::endl;
            int size = response.friends_size();
            for(int i = 0;i<size;i++)
            {
                std::cout<<"index:"<<(i+1)<<" name"<<response.friends(i)<<std::endl;
            }
        }
        else
        {
            std::cout<<"rpc GetFriendsList response error:"<<response.result().errmsg()<<std::endl;
        }
    }

    return 0;
}