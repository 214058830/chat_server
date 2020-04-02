#include <iostream>
#include "chatClient.hpp"

// ./chatClient ip

static void menu(int& s){
	std::cout << "/*******************************************************/"<<std::endl;
	std::cout << "/****	1.register        2.login           ************/"<<std::endl;
	std::cout << "/****	3.exit                              ************/"<<std::endl;
	std::cout << "/*******************************************************/"<<std::endl;
	std::cout << "please select:";
	std::cin >> s;
}

static void usage(std::string proc){
	std::cout << "usage:" << proc << "server ip" << std::endl;
} 
static void* recvChat(void* arg){
		pthread_detach(pthread_self());
		chatClient* cp = (chatClient*)arg;
		std::string recvString;
		message msg;
		while(1){
			socketApi::recvMessage(cp->udp_sock, recvString, cp->server);
			//std::cout << "debug:recv udp message:" << recvString;
			msg.toRecvValue(recvString);
			std::cout << "[" <<msg.nick_name << "][" << msg.school << "]:" << msg.text << std::endl;
		}
	}
int main(int argc, char *argv[]){
	if(argc != 2){
		usage(argv[0]);
		exit(1);
	}
	chatClient *cp = new chatClient(argv[1]);
	cp->initClient();

	int select = 0;
	while(1){
		menu(select);
		switch (select)
		{
			case 1:
				if(cp->_register()){
				}
				else
				{
					std::cout << "register is error, please again" << std::endl;
				}
				break;
			case 2:
				if(cp->login()){
					pthread_t p;
					pthread_create(&p, NULL, recvChat, (void*)cp); //接受消息线程
					cp->chat();

				}
				else{
					std::cout << "login is error, please again" << std::endl;
				}
				break;
			case 3:
				cp->logout();
				delete cp;
				exit(0);
				break;
			default:
				std::cout << "your input is error, please again" << std::endl;
				break;
		}
	}
	return 0;
}
