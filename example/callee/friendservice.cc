#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <vector>
#include "logger.h"

using namespace fixbug;

class FriendService : public FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(int32_t userid)
    {
        std::cout<<"do GetFriendsList service"<<"userid: "<<userid<<std::endl;
        std::vector<std::string> res;
        res.push_back("gao yang");
        res.push_back("wang shuo");
        return res;
    }

    // 重写基类方法
    void GetFriendsList(google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendsListRequest* request,
                       ::fixbug::GetFriendsListResponse* response,
                       ::google::protobuf::Closure* done)
    {
        int32_t userid = request->userid();

        auto friendsList = GetFriendsList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for(auto& name : friendsList)
        {
            std::string* p = response->add_friends();
            *p = name;
        }
        done->Run();
    }

};


int main(int argc, char** argv)
{
    LOG_INFO("first log msg");
    LOG_ERROR("%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);


    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    //把UserService服务对象发布到rpc节点上,RpcProvider是一个rpc网络服务对象
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程rpc调用请求
    provider.Run();
}