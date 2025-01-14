#ifndef __BOBLIEW_TCP_SERVER_H__
#define __BOBLIEW_TCP_SERVER_H__

#include <memory>
#include <functional>
#include "address.h"
#include "iomanager.h"
#include "socket.h"
#include "noncopyable.h"
#include "config.h"
#include "mutex.h"


namespace bobliew {

struct TcpServerConf {
    typedef std::shared_ptr<TcpServerConf> ptr;

    std::vector<std::string> address;
    int keepalive = 0;
    int timeout = 1000 * 2 * 60;
    int ssl = 0;
    std::string id;
    //server type
    std::string type = "http";
    std::string name;
    std::string cert_file;
    std::string key_file;
    std::string accept_worker;
    std::string io_worker;
    std::string process_worker;
    std::map<std::string, std::string> args;

    bool isValid() const {
        return !address.empty();
    }

    bool operator==(const TcpServerConf& oth) const {
        return address == oth.address
        && keepalive == oth.keepalive
        && timeout == oth.timeout
        && name == oth.name
        && ssl == oth.ssl
        && cert_file == oth.cert_file
        && key_file == oth.key_file
        && accept_worker == oth.io_worker
        && process_worker == oth.process_worker
        && args == oth.args
        && id == oth.id
        && type == oth.type;
    }
};

template<>
class LexicalCast<std::string, TcpServerConf> {
public:
    TcpServerConf operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        TcpServerConf conf;
        conf.id = node["id"].as<std::string>(conf.id);
        conf.type = node["type"].as<std::string>(conf.type);
        conf.keepalive = node["keepalive"].as<int>(conf.keepalive);
        conf.timeout = node["timeout"].as<int>(conf.timeout);
        conf.name = node["name"].as<std::string>(conf.name);
        conf.ssl = node["ssl"].as<int>(conf.ssl);
        conf.cert_file = node["cert_file"].as<std::string>(conf.cert_file);
        conf.key_file = node["key_file"].as<std::string>(conf.key_file);
        conf.accept_worker = node["accept_worker"].as<std::string>();
        conf.io_worker = node["io_worker"].as<std::string>();
        conf.process_worker = node["process_worker"].as<std::string>();
        conf.args = LexicalCast<std::string, std::map<std::string, std::string> >()(node["args"].as<std::string>(""));
        if(node["address"].IsDefined()) {
            for(size_t i = 0; i < node["address"].size(); ++i) {
                conf.address.push_back(node["address"][i].as<std::string>());
            }
        }
        return conf;
    }
};

class TcpServer : public std::enable_shared_from_this<TcpServer>, Noncopyable {
public:
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer(bobliew::IOManager* worker = bobliew::IOManager::GetThis(),
              bobliew::IOManager* io_worker = bobliew::IOManager::GetThis(),
              bobliew::IOManager* accept_worker = bobliew::IOManager::GetThis());
    virtual ~TcpServer();
    //绑定地址，返回是否绑定成功
    virtual bool bind(bobliew::Address::ptr addr, bool ssl = false);
    virtual bool bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& fails,
        bool ssl = false);
    bool loadCertificates(const std::string& cert_file, const std::string& key_file);

    virtual bool start();
    virtual void stop();

    uint64_t getRecvTimeout() const { return m_recvTimeout;}

    std::string getName() const { return m_name;}

    void setRecvTimeout(uint64_t v) { m_recvTimeout = v;}

    virtual void setName(const std::string& v) {m_name = v;}

    bool isStop() const { return m_isStop;}

    TcpServerConf::ptr getConf() const { return m_conf;}

    void setConf(TcpServerConf::ptr v) { m_conf = v;}

    void setConf(const TcpServerConf& v);

    virtual std::string toString(const std::string& prefix = "");

    std::vector<Socket::ptr> getSocks() const { return m_socks;}
protected:
    //处理新链接的Socket类
    virtual void handleClient(Socket::ptr sock);
    //开始接受链接
    virtual void startAccept(Socket::ptr sock);
protected:
    // 监听Socket数组
    std::vector<Socket::ptr> m_socks;
    // 新链接的Socket工作调度器
    IOManager* m_worker;
    IOManager* m_ioWorker;
    //服务器Socket接受链接的调度器
    IOManager* m_acceptWorker;
    //超时（毫秒）
    uint64_t m_recvTimeout;
    //服务器名称
    std::string m_name;
    //服务器类型
    std::string m_type = "tcp";
    //服务是否停止
    bool m_isStop;

    bool m_ssl = false;

    TcpServerConf::ptr m_conf;
};

}

#endif
