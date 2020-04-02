#include "chatServer.hpp"
#include <iostream>

// ./chatServer tcp_port udp_port运行时这样输入
void* runProduct(void *arg){
	pthread_detach(pthread_self());
	chatServer* sp = (chatServer*)arg;
	for(;;){
		sp->product();
	}
}
void* runConsume(void*arg)
{
	pthread_detach(pthread_self());
	chatServer* sp = (chatServer*)arg;
	for(;;){
		sp->consume();
	}
}
int main(int argc, char* argv[]){
	if(argc != 3){
		std::cout << "your input is error,please enter ./xxx tcp_port udp_port." << std::endl;
		exit(1);
	}

	int tcp_port = atoi(argv[1]);
	int udp_port = atoi(argv[2]);

	chatServer *sp = new chatServer(tcp_port, udp_port);
	sp->initServer();

	pthread_t c, p;
	pthread_create(&c, NULL, runProduct, (void*)sp); //接受消息线程
	pthread_create(&p, NULL, runConsume, (void*)sp); //发送消息线程

	sp->start();

	return 0;
}
