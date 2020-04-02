#pragma once

#include <iostream>
#include <string>
#include "protocolUtil.hpp"
#include "message.hpp"
//默认情况下
#define SERVER_TCP_PORT 8080
#define UDP_PORT 8888

class chatClient{
public:
	chatClient(std::string server_ip_)
	:server_ip(server_ip_),
	tcp_sock(-1),
	udp_sock(-1),
	id(0)
	{
		server.sin_family = AF_INET;
		server.sin_port = htons(UDP_PORT);
		server.sin_addr.s_addr = inet_addr(server_ip.c_str());
	}
	void initClient(){
		udp_sock = socketApi::_socket(SOCK_DGRAM);
	}
	bool connectServer(){
		tcp_sock = socketApi::_socket(SOCK_STREAM);
		return socketApi::_connect(tcp_sock, server_ip, SERVER_TCP_PORT);
	}
	bool _register(){
		if(util::registerEnter(nick_name, school, passwd) && connectServer() ){
			request rq;
			rq.method = "REGISTER\n";
			
			Json::Value root;
			root["name"] = nick_name;
			root["school"] = school;
			root["passwd"] = passwd;

			util::seralizer(root, rq.text);
			//std::cout << "serqlizer q.text:" << rq.text << std::endl;
			rq.content_length = "Content-Length: ";
			rq.content_length += util::intToString((rq.text).size());
			rq.content_length += "\n";
			rq.blank = "\n";
			socketApi::_send(tcp_sock, rq);
			recv(tcp_sock, &id, sizeof(id), 0); //错误判断

			bool res = false;
			if(id >= 10000){
				std::cout << "register success, your login id is:" << id << std::endl;
				res = true;
			}
			else std::cout << "register failed." << std::endl;
			
			close(tcp_sock);
			return res;
		}
	}
	bool login(){
		if(util::loginEnter(id, passwd) && connectServer()){ //connecServer
			request rq;
			rq.method = "LOGIN\n";
			rq.blank = "\n";
			Json::Value root;
			root["id"] = id;
			root["passwd"] = passwd;//用passwd 和 id一起条件下线   不建议只id下线
			util::seralizer(root, rq.text);
			rq.content_length = "Content-Length: ";
			rq.content_length += util::intToString((rq.text).size());
			rq.content_length += "\n";
			//std::cout << "send rq is begin" << std::endl;
			socketApi::_send(tcp_sock, rq);
			//std::cout << "send rq is success:" << rq.text << std::endl;
			recv(tcp_sock, &id, sizeof(id), 0);
			//std::cout << "recv id :" << id << std::endl;
			bool res = false;
			if(id >= 10000){
				std::cout << "login success, your login id is:" << id << std::endl;
				res = true;
				request rq1;
				socketApi::_recv(tcp_sock, rq1);
				Json::Value read;
				util::unSeralizer(rq1.text, read);
				if(rq1.method == "INIT"){
					nick_name = read["nick_name"].asString();
					school = read["school"].asString();
				}
				else{
					res = false; //数据初始化错误
				}
			}
			close(tcp_sock);
			std::string test = "is coming...";
			message msg(nick_name, school, test, id);
			std::string sendString;
			msg.toSendString(sendString);
			socketApi::sendMessage(udp_sock, sendString, server);
			
			return res;
		}
	}
	void recvChat(){
		pthread_detach(pthread_self());
		std::string recvString;
		while(1){
			socketApi::recvMessage(udp_sock, recvString, server);
			std::cout << "debug:recv udp message:" << recvString;
			msg.toRecvValue(recvString);
			std::cout << "[" <<msg.nick_name << "][" << msg.school << "]:" << msg.text << std::endl;
		}
	}
	void chat(){
		message msg;
		std::string sendString;	
		struct sockaddr_in peer;
		while(1){
			msg.nick_name = nick_name;
			msg.school = school;
			msg.id = id;
			//std::cout << "please enter:";
			std::cin >> msg.text;
			if(msg.text == "quit"){
				/////////////////////////
				//guanbixiancheng
				break;
			}
			msg.toSendString(sendString);
			socketApi::sendMessage(udp_sock, sendString, server);
			std::cout << "debug:send udp message:" << sendString;
		}
	}
	void logout(){
		request rq;
			rq.method = "LOGOUT\n";
			rq.blank = "\n";
			Json::Value root;
			root["id"] = id;
			//root["text"] = "is logout.";//用passwd 和 id一起条件下线   不建议只id下线
			util::seralizer(root, rq.text);
			rq.content_length = "Content-Length: ";
			rq.content_length += util::intToString((rq.text).size());
			rq.content_length += "\n";
			socketApi::_send(tcp_sock, rq);c
	}
	~chatClient(){
		close(udp_sock);
	}
private:
	std::string server_ip;
	struct sockaddr_in server;
	int tcp_sock;
	int udp_sock;

	unsigned int id;
	std::string passwd;
	std::string nick_name;
	std::string school;
};
