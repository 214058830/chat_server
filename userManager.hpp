#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include "protocolUtil.hpp"
#include <json/json.h>
class user{
public:
	user()
	{}
	user(const std::string& nick_name_, const std::string& school_, const std::string& passwd_)
		:nick_name(nick_name_),
		school(school_),
		passwd(passwd_)
	{}
	~user()
	{}
	bool isPasswd(const std::string passwd_){
		if(passwd == passwd_){
			return true;
		}
		return false;
	}

public:
	std::string nick_name;
	std::string school;
	std::string passwd;
};

class userManager{
public:
	userManager()
		:assgin_id(10000)
	{
		pthread_mutex_init(&mutex, NULL);
	}
	unsigned int insert(const std::string& name_, const std::string& school_, const std::string& passwd_){
		lock();
		unsigned int id = assgin_id++;
		
		user u(name_, school_, passwd_);
		users.insert(std::make_pair(id, u));
		//[debug]std::cout << "insert name is:" << name_ << "insert school is:" << school_ << "insert passwd is:" <<  passwd_ << std::endl;
		unlock();
		return id;
	}
	bool check(const int& id, const std::string& passwd){
		lock();
		auto pos1 = users.find(id);
		auto pos2 = online_users.find(id);
 		if(pos1 != users.end() && pos2 == online_users.end()){
			user u = pos1->second;
			if(u.isPasswd(passwd)){
				std::cout << "login test success." << std::endl;
				unlock();
				return true;
			}
		}
		unlock();
		return false;
	}
	void addOnlineUser(unsigned int id, sockaddr_in& peer){
		lock();
		online_users.insert(std::make_pair(id, peer));
		unlock();
	}
	void subOnlineUser(unsigned int id){
		lock();
		online_users.erase(id);
		unlock();
	}
	std::unordered_map<unsigned int, struct sockaddr_in> onlineUsers(){
		lock();
		auto online = online_users;
		unlock();
		return online;
	}
	void initInformation(int sock, unsigned int id){
		request rq;
		rq.method = "INIT\n";
		Json::Value root;
		root["nick_name"] = users[id].nick_name;
		root["school"] = users[id].school;
		util::seralizer(root, rq.text);
		rq.blank = "\n";
		rq.content_length = "Content-Length: ";
		rq.content_length += util::intToString((rq.text).size());
		rq.content_length += "\n";
		socketApi::_send(sock, rq);
		/////////////////////////////////////////////////
		std::cout << "send request.text:" << rq.text << std::endl;
	}
	~userManager()
	{
		pthread_mutex_destroy(&mutex);
	}

	std::unordered_map<unsigned int, sockaddr_in> online_users;
private:
	int assgin_id;	//注册起始id
	std::unordered_map<unsigned int, user> users;
	pthread_mutex_t mutex;

	void lock(){
		pthread_mutex_lock(&mutex);
	}
	void unlock(){
		pthread_mutex_unlock(&mutex);
	}
};
