#include "rpcprovider.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/InetAddress.h>
#include "mprpcapplication.h"
#include <functional>
#include <google/protobuf/descriptor.h>
#include "rpcheader.pb.h"
#include "zookeeperutil.h"


/*  
    service_name => service描述 
                            =>  service* 记录服务对象
                            method_name => method方法对象

*/

// 这里是框架提供给外部使用的，可以发布rpc Service方法的函数接口,注册服务
void RpcProvider::NotifyService(::google::protobuf::Service* service)
{
    ServiceInfo service_info;
    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor* pserviceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象 方法的数量
    int methodCnt = pserviceDesc->method_count();

    std::cout<<"service name:"<<service_name<<std::endl;

    for(int i = 0;i<methodCnt;i++)
    {
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name,pmethodDesc});//将方法名字和方法的描述作为键值对放入ServiceInfo中
        std::cout<<"method name:"<<method_name<<std::endl;
    }

    service_info.m_service = service;
    m_serviceMap.insert({service_name,service_info});

}

//启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{

    std::string ip = MprpcApplication::GetInstance().getConfig().Load("rpcserverip");
    uint16_t port = stoi(MprpcApplication::GetInstance().getConfig().Load("rpcserverport"));
    InetAddress address(ip,port);

    // 创建TcpServer对象
    TcpServer server(&m_eventLoop,address,"RpcProvider");

    // 绑定连接回调和消息读写回调,分离了业务代码和网络io代码
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection,this,std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    
    // 设置线程数量
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout 30s  zkClient 会起网络io线程，每1/3timeout时间发送心跳消息
    ZkClient zkCli;
    zkCli.Start();
    // service_name 为永久节点， method_name为临时性节点
    for(auto&sp : m_serviceMap)
    {
        std::string service_path = "/"+sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for(auto& mp : sp.second.m_methodMap)
        {
            // /service_name/method_name
            std::string method_path = service_path+"/"+mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d",ip.c_str(),port);
            // ZOO_EPHEMERAL表示znode是临时性节点
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }

    std::cout<<"RpcProvider start service at ip:"<<ip<<" port:"<<port<<std::endl;

    server.start();
    m_eventLoop.loop();

}


// 新的socket连接回调
void RpcProvider::onConnection(const TcpConnectionPtr& conn)
{
    if(conn->connected()==false)
    {
        // 和rpc client的连接断开了
        conn->shutdown();
    }
}

/*
    在框架内部 RpcProvider和RpcConsumer要协商好之间通信的protobuf的数据类型
    service_name  method_name  args    定义proto的message类型，进行数据头的序列化和反序列化
                                        service_name method_name   args_size（记录参数长度为了解决粘包问题）

    header_size + service_name + method_name + args_size + args

    header_size(4个字节) +  header_str (包括service和method)     +   args_size +     args_str
    表明header_str有多长 |   protobuf去解析出service和method |     |       这部分解析出参数        |
*/

// 已建立连接用户的读写事件回调，如果远端有一个rpc服务调用请求，那么onMessage方法就会响应
void RpcProvider::onMessage(const TcpConnectionPtr&conn, Buffer*buf, muduo::Timestamp)
{
    // 网络上接收的远程rpc调用请求的字符流      包含 服务对象名，方法名，参数
    std::string recv_buf = buf->retrieveAllAsString();
    //从字符流中读取前四个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);//数字按照二进制的方式存储

    // 根据header size读取数据头的原始字符流,然后反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4,header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // 数据头反序列化失败
        std::cout<<"rpc_header_str: "<<rpc_header_str<<" parse error!"<<std::endl;
        return;
    }
    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4+header_size, args_size);

    // 打印调试信息
    std::cout<<"-------------------------------------"<<std::endl;
    std::cout<<"header size:"<<header_size<<std::endl;
    std::cout<<"rpc_header_str:"<<rpc_header_str<<std::endl;
    std::cout<<"service_name:"<<service_name<<std::endl;
    std::cout<<"method_name:"<<method_name<<std::endl;
    std::cout<<"args_str:"<<args_str<<std::endl;
    std::cout<<"-------------------------------------"<<std::endl;

    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if(it==m_serviceMap.end())
    {
        std::cout<<service_name<<" does not exist!"<<std::endl;
        return;
    }
    google::protobuf::Service* service = it->second.m_service;  //获取Service对象

    if(it->second.m_methodMap.count(method_name)==0)
    {
        std::cout<<service_name<<":"<<method_name<<" does not exist!"<<std::endl;
        return;
    }
    const google::protobuf::MethodDescriptor* method = it->second.m_methodMap[method_name];//获取method对象

    // 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str))
    {
        std::cout<<"request parse error"<<"contet: "<<args_str<<std::endl;
    }
    google::protobuf::Message* response = service->GetResponsePrototype(method).New();

    // 给下面的CallMethod方法的调用，绑定一个Closure的回调函数
    google::protobuf::Closure* done = 
        google::protobuf::NewCallback<RpcProvider,const TcpConnectionPtr&,google::protobuf::Message*>(this, &RpcProvider::SendRpcResponse, conn, response);


    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // 相当于new UserService()->Login(controller, request, response, done);
    service->CallMethod(method,nullptr, request, response, done);

}

// done->Run();就是调用下面这个函数
// closure done的回调函数，用于序列化rpc响应和网络发送
void RpcProvider::SendRpcResponse(const TcpConnectionPtr&conn, google::protobuf::Message*response)
{
    std::string response_str;
    if(response->SerializePartialToString(&response_str))//response进行序列化
    {
        // 序列化成功后,通过网络把rpc方法执行的结果发送回rpc的调用方
        conn->send(response_str);
    }
    else
    {
        std::cout<<"Serialize response_str error!"<<std::endl;
    }
    conn->shutdown();   //模拟http短连接服务,由rpcprovider主动断开连接
}