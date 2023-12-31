#pragma once
#include <zookeeper/zookeeper.h>
#include <string>

// 封装的zk客户端类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // zkclient启动连接zkserver
    void Start();
    // 在zkServer上根据指定的path创建znode节点，state表明0：永久性节点 1：临时性节点
    void Create(const char* path, const char* data, int datalen, int state = 0);
    // 根据参数指定的znode节点路径，获取节点的值
    std::string GetData(const char* path);
private:
    // zk的客户端句柄
    zhandle_t* m_zhandle;
};