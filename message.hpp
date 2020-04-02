#pragma once
#include <iostream>
#include <string>
#include "json/json.h"
#include "protocolUtil.hpp"
//消息类型
class message{
public:
	message(const std::string& n_, std::string& s_, const std::string& t_, const unsigned int& id_)
		:nick_name(n_),
		school(s_),
		text(t_),
		id(id_)
	{}
	message(){
	}

	void toSendString(std::string& sendString){
		Json::Value root;
		root["name"] = nick_name;
		root["school"] = school;
		root["text"] = text;
		root["id"] = id;
		util::seralizer(root, sendString);
	}
	void toRecvValue(std::string& recvString){
		Json::Value root;
		util::unSeralizer(recvString, root);
		nick_name = root["name"].asString();
		school = root["school"].asString();
		text = root["text"].asString();
		id = root["id"].asInt();
	}
	~message(){
	}

	std::string nick_name;
	std::string school;
	std::string text;
	unsigned int id;
};
