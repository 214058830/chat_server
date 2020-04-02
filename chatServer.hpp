#pragma once
#include<iostream>
#include<pthread.h>
#include "protocolUtil.hpp"
#include"userManager.hpp"
#include"log.hpp"
#include "dataPool.hpp"
#include "message.hpp"

class chatServer;
class param{
public:
	chatServer *cs;
	int client_sock;
	std::string client_ip;
	int client_port;

	param(chatServer *cs_, int& client_sock_, const std::string& client_ip_, const int& client_port_)
		:cs(cs_),
		client_sock(client_sock_),
		client_ip(client_ip_),
		client_port(client_port_)
	{}
	~param(){
	}
};
//登陆 注销 接受 发送消息
class chatServer{
private:
	//TCP帐号注册管理
	int tcp_sock;
	int tcp_port;
	//UDP消息传输
	int udp_sock;
	int udp_port;
	static userManager um;
	dataPool pool;
	static unsigned int id;
public:
	chatServer(int tcp_port_ = 8080, int udp_port_ = 8888)
		:tcp_sock(-1),
		tcp_port(tcp_port_),
		udp_sock(-1),
		udp_port(udp_port_)
	{}
	void initServer(){
		tcp_sock = socketApi::_socket(SOCK_STREAM);
		socketApi::_bind(tcp_sock, tcp_port);
		socketApi::_listen(tcp_sock);

		udp_sock = socketApi::_socket(SOCK_DGRAM);
		socketApi::_bind(udp_sock, udp_port);
	}
	unsigned int registerUser(const std::string& name, const std::string& school, const std::string& passwd){
		return um.insert(name, school, passwd);
	}

	//unsigned int loginUser(const unsigned int& id_, const std::string& passwd_){
	//	int id =  um.check(id_, passwd_);
		// struct sockaddr_in peer;
		// peer.sin_family = AF_INET;
		// peer.sin_addr.s_addr = inet_addr(ip.c_str());
		// peer.sin_port = htons(port);
		//um.addOnlineUser(id, peer);
	//	return id;
	//}
	//接受消息线程
	void product(){
		pthread_detach(pthread_self());
		struct sockaddr_in peer;
		while(1){
			std::string message;
			socketApi::recvMessage(udp_sock, message, peer);
			
			um.addOnlineUser(id, peer);
			////////////////////////////////////////////////
			printf("clientPort=%u\n",ntohs(peer.sin_port));
			//////////////////////////////////////////////

			std::cout << "debug:recv message:" << message;

			if(!message.empty()){
				pool.putMessage(message);
				//message m;
				//m.toRecvValue(message);//测试使用
				//std::cout << m.nick_name << m.school << m.text << std::endl;
				//um.addOnlineUser(m.id(), peer);
			}
		}
	}
	//发送消息线程
	void consume(){
		pthread_detach(pthread_self());
		std::string message;
		while(1){
			pool.getMessage(message);

			std::cout << "debug:send message:" << message;

			auto online = um.online_users;
			for(auto it = online.begin(); it != online.end(); ++it){
				//std::cout << "fa song xiao xi" << std::endl;
				socketApi::sendMessage(udp_sock, message, it->second);
			}
			
		}
	}
	//用户管理线程
	static void *handlerRequest(void *arg){//线程create调用函数 加上static防止调用函数时其他参数 不要this参数
		param *p = (param*)arg;
		int sock = p->client_sock;
		chatServer* cs = p->cs;

		std::string client_ip = p->client_ip;
		int client_port = p->client_port;
		delete p;
		pthread_detach(pthread_self());
		request rq;
		socketApi::_recv(sock, rq);
		std::cout << "recv request.text:" << rq.text;

		Json::Value read;
		util::unSeralizer(rq.text, read);

		if(rq.method == "REGISTER"){
			std::string name = read["name"].asString();
			std::string school = read["school"].asString();
			std::string passwd = read["passwd"].asString();
			unsigned int id = cs->registerUser(name, school, passwd);
			send(sock, &id, sizeof(id), 0);
		}
		else if(rq.method == "LOGIN"){
			id = read["id"].asInt();
			std::string passwd = read["passwd"].asString();
			// check move user to online

			if(um.check(id, passwd)){
			//cs->loginUser(passwd, client_ip, client_port);
				send(sock, &id, sizeof(id), 0);
				if(id >= 10000){
					chatServer::um.initInformation(sock, id);
				}
			}
		}
		else{
			id = read["id"].asInt();
			um.subOnlineUser(id);
			std::cout << "[" << client_ip << "] [" <<  client_port << "]：is logout." << std::endl;
		}
		close(sock);
	}
	void start(){
		std::string client_ip;
		int client_port;
		for(;;){
			int client_sock = socketApi::_accept(tcp_sock, client_ip, client_port);
			
			if(client_sock > 0){
				std::cout << "debug:get a new tcp_client    [ip]:" << client_ip << "  [port]:" << client_port << std::endl;
				param *p = new param(this, client_sock, client_ip, udp_port);
				pthread_t tid;
				pthread_create(&tid, NULL, handlerRequest, p);
			}
		}
	}
	~chatServer(){}
};
userManager chatServer::um;
unsigned int chatServer::id = 0;
