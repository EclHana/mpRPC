#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>


// 全局的watcher观察器   zkserver给zkclient的通知
void globle_watcher(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx)
{
    if(type==ZOO_SESSION_EVENT)//回调的消息类型是和会话相关的
    {
        if(state == ZOO_CONNECTED_STATE)//zkclient和zkserver连接成功
        {
            //这个地方获取到zoo_set_context在zh这个句柄上设置的参数sem
            sem_t* sem = (sem_t*)zoo_get_context(zh);            
            sem_post(sem);
        }
    }
}


ZkClient::ZkClient():m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if(m_zhandle)
    {
        zookeeper_close(m_zhandle);//关闭句柄，释放资源，
    }
}

// zkclient启动连接zkserver
void ZkClient::Start()
{
    std::string host = MprpcApplication::getConfig().Load("zookeeperip");
    std::string port = MprpcApplication::getConfig().Load("zookeeperport");
    std::string conn_str = host+ ":" +port;

    /*
        zookeeper_mt:多线程版本
        zookeeper的api客户端程序提供了三个线程
            1.zookeeper_init api调用的线程
            2.网络IO线程  用pthread_create( poll )生成一个线程专门发起网络io操作
            3.watcher回调线程 负责server通知client
    */

    // zookeeper_init连接server是异步方法 这个函数返回之后连接还没建立了，这个函数返回非空只是说明创建 zhandle_t成功了
    // 等待globle_watcher执行完毕就说明到zookeeper的连接建立成功，通过在globle_watcher中sem_post(sem)告知当前线程
    m_zhandle = zookeeper_init(conn_str.c_str(), globle_watcher, 30000, nullptr, nullptr, 0);
    
    if(m_zhandle==nullptr)
    {
        std::cout<<"zookeeper_init error!"<<std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);

    //给m_zhandle这个句柄设置参数,globle_watcher这个监听回调函数用到这个参数，也就是这个sem
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem);
    std::cout<<"zookeeper_init success!"<<std::endl;
}

// 在zkServer上根据指定的path创建znode节点，state表明0：永久性节点 1：临时性节点
void ZkClient::Create(const char* path, const char* data, int datalen, int state)
{
    char path_buf[128];
    int buffer_len = sizeof(path_buf);
    int flag;
    // 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
    flag = zoo_exists(m_zhandle,path,0,nullptr);
    if(flag==ZNONODE)//表示path表示znode节点不存在
    {
        // 创建指定path的znode节点
        flag = zoo_create(m_zhandle,path,data,datalen, &ZOO_OPEN_ACL_UNSAFE, state, path_buf, buffer_len);
        if(flag==ZOK)
        {
            std::cout<<"znode create success... path:"<<path<<std::endl;
        }
        else
        {
            std::cout<<"flag:"<<flag<<std::endl;
            std::cout<<"znode create error... path:"<<path<<std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 根据参数指定的znode节点路径，获取节点的值
std::string ZkClient::GetData(const char* path)
{
    char buf[64];
    int buf_len = sizeof(buf);
    int flag = zoo_get(m_zhandle, path,0,buf,&buf_len,nullptr);
    if(flag!=ZOK)
    {
        std::cout<<"get znode error... path:"<<path<<std::endl;
        return "";
    } 
    else
    {
        return buf;
    }
}