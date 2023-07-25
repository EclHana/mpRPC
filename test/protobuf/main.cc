#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;

int main()
{
    // LoginResponse rsp;
    // ResultCode* rc =  rsp.mutable_result();
    // rc->set_errcode(0);
    // rc->set_errmsg("log fail");

    GetFriendListsRespose rsp;
    ResultCode* rc = rsp.mutable_result();
    rc->set_errcode(0);
    
    user* u1 =  rsp.add_friendlist();
    u1->set_age(12);
    u1->set_name("jais");
    u1->set_sex(user::MAN);


    user* u2 =  rsp.add_friendlist();
    u2->set_age(24);
    u2->set_name("Roober");
    u2->set_sex(user::WOMAN);

    std::cout<<rsp.friendlist_size()<<std::endl;

}




int main1()
{
    LoginRequest req;
    req.set_name("zhnag sanf");
    req.set_pwd("123456");

    std::string buf;
    if(req.SerializeToString(&buf))
    {
        std::cout<<buf<<std::endl;
    }
    //反序列化
    LoginRequest reqB;
    if(reqB.ParseFromString(buf))
    {
        std::cout<<req.name()<<std::endl;
        std::cout<<req.pwd()<<std::endl;
    }


    return 0;
}