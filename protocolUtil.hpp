#pragma once
#include<string.h>
#include<iostream>
#include<string>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include "log.hpp"
#include<unistd.h>
#include<stdlib.h>
#include <sstream>
#include <json/json.h>

#define MESSAGE 1024
#define BACKLOG 5
//封装socket
class util{
public:
	static bool registerEnter(std::string& nick_name, std::string& school, std::string& passwd){
		std::cout << "please enter nick name->:";
		std::cin >> nick_name;
		std::cout << "please enter school->:";
		std::cin >> school;
		std::cout << "please enter passwd->:";
		std::cin >> passwd;
		std::string again;
		std::cout << "please enter passwd again->:";
		std::cin >> again;
		if(passwd == again){
			return true;
		}
		return false;
	}
	static bool loginEnter(unsigned int& id, std::string& passwd){
		std::cout << "please enter your id:";
		std::cin >> id;
		std::cout << "please enter your passwd:";
		std::cin >> passwd;
		return true;
	}
	static void seralizer(Json::Value& root, std::string& text){
		Json::FastWriter w;
		text = w.write(root);
	}
	static void unSeralizer(std::string& text, Json::Value& root){
		Json::Reader r;
		r.parse(text, root);
	}
	static std::string intToString(int x){
		std::stringstream ss;
		ss << x;
		return ss.str();
	}
	static int stringToInt(std::string& str){
		std::stringstream ss(str);
		int x;
		ss >> x;
		return x;
	}
	static void recvOneLine(int sock, std::string& outString){
		char c = 'x';
		
		while(1){
			size_t s = recv(sock, &c, 1, 0);
			if(s > 0){
				if(c == '\n'){
					break;
				}
				outString.push_back(c);
			}
			else{
				break;
			}
		}
	}
	
};

class request{
public:
	// 模仿 HTTP 协议
	std::string method; // register／login／logout 操作方式
	std::string content_length; // content_length 89
	std::string blank; //空行
	std::string text; //正文内容

	request()
	{}
	~request()
	{}
};

class socketApi{
public:
	static int _socket(int type){//套接字类型type
		int sock = socket(AF_INET, type, 0);
		if(sock < 0){
			log("_socket is error...", ERROR);
			exit(2);
		}
	}
	static void _bind(int socket, int port){
		struct sockaddr_in server_socket;
		//bzero(&server_socket, sizeof(server_socket));
		server_socket.sin_family = AF_INET;
		server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
		server_socket.sin_port = htons(port);
		if(bind(socket, (struct sockaddr*)&server_socket, sizeof(struct sockaddr_in)) < 0){
			log("_bind is error...", ERROR);
			exit(3);
		}
	}
	static void _listen(int socket){
		if(listen(socket, BACKLOG) < 0){
			log("_listen is error...", ERROR);
			exit(4);
		}
	}
	//ip port同时拿出ip和port
	static int _accept(int listen_socket, std::string &ip, int &port){
		struct sockaddr_in client_sockaddr;//要保存的客户端地址
		socklen_t len = sizeof(client_sockaddr);
		int client_socket = accept(listen_socket, (struct sockaddr *)&client_sockaddr, &len);
		if(client_socket < 0){
			log("_accept is error...", WORNING);
			return -1;
		}
		//用IP 和 PORT保存好客户端的信息传出
		ip = inet_ntoa(client_sockaddr.sin_addr);
		port = ntohs(client_sockaddr.sin_port);
		return client_socket;
	}
	static bool _connect(const int& client_socket, const std::string& server_ip,const int& server_port){
		struct sockaddr_in server_sock;
		//memset(&server_sock, 0, sizeof(server_sock));
		server_sock.sin_family = AF_INET;
		server_sock.sin_addr.s_addr = inet_addr(server_ip.c_str());
		server_sock.sin_port = htons(server_port);
		int ret = connect(client_socket, (struct sockaddr*)&server_sock, sizeof(server_sock));
		if(ret < 0){
			log("_connect is error...", WORNING);
			return false;
		}
		return true;
	}

	//tcp
	static void _recv(int sock, request& rq){
		util::recvOneLine(sock, rq.method);
		util::recvOneLine(sock, rq.content_length);
		util::recvOneLine(sock, rq.blank);

		std::string& cl = rq.content_length; // content_length:  88
		std::size_t pos = cl.find(": ");
		std::size_t npos = cl.find("\n");
		//if(cl.end() == pos){
		//	std::cout << "error" << std::endl;
		//	return;
		//}
		std::string sub = cl.substr(pos+2, npos - pos - 2);

		int size = util::stringToInt(sub);
		char c;
		for(auto i = 0; i < size; ++i){
			recv(sock, &c, 1, 0);
			(rq.text).push_back(c);
		}
		
		//recvOneLine(sock, rq.text);
	}
	static void _send(int sock, request& rq){
		std::string& m_ = rq.method;
		std::string& cl_ = rq.content_length;
		std::string& b_ = rq.blank;
		std::string& text_ = rq.text;
		send(sock, m_.c_str(), m_.size(), 0);
		send(sock, cl_.c_str(), cl_.size(), 0);
		send(sock, b_.c_str(), b_.size(), 0);
		send(sock, text_.c_str(), text_.size(), 0);
		//std::cout << "send rq.method:" << rq.method << std::endl;
		//std::cout << "send rq.contentlength" << rq.content_length << std::endl;
		//std::cout << "send rq.text:" << rq.text << std::endl;
	}

	//udp
	static void recvMessage(int sock, std::string& message, struct sockaddr_in& peer){
		char msg[MESSAGE] = {0};
		socklen_t len = sizeof(peer);
		size_t s = recvfrom(sock, msg, sizeof(msg) - 1, 0, (struct sockaddr*)&peer, &len);
		if(s <= 0){
			log("recv form message error", WORNING);
		}
		else{
			message = msg;
		}
	}
	static void sendMessage(int sock, std::string& message, sockaddr_in& peer){
		sendto(sock, message.c_str(), message.size(), 0, (sockaddr*)&peer, sizeof(peer));
	}
};
