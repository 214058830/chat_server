#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <semaphore.h>

class dataPool{
private:
	std::vector<std::string> pool;
	int cap;
	sem_t data_sem;
	sem_t blank_sem;
	int product_step;
	int consume_step;
public:
	dataPool(int cap_ = 512)
		:cap(cap_),
		pool(cap_)
	{
		sem_init(&data_sem, 0 , 0);
		sem_init(&blank_sem, 0, cap);
		product_step = 0;
		consume_step = 0;
	}

	void putMessage(std::string& msg){
		sem_wait(&blank_sem);
		pool[product_step] = msg;
		product_step++;
		product_step %= cap;
		sem_post(&data_sem);
	}
	void getMessage(std::string& msg){
		sem_wait(&data_sem);
		msg = pool[consume_step];
		consume_step++;
		consume_step %= cap;
		sem_post(&blank_sem);
	}
	~dataPool(){
		sem_destroy(&data_sem);
		sem_destroy(&blank_sem);
	}
};
