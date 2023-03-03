#include <chrono>
#include <condition_variable>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

#include "common.hpp"

using namespace std;

mutex					s_mutex;
condition_variable		s_cv;
bool					s_ready { false };

uint64_t	s_post_time {};
uint64_t	s_total_delay {};
int			s_recv_count = 0;

void ThreadProducer() {
	cout << "producer:started..." << endl;
	int post_count = 0;
	while( post_count < TOTAL_NOTES ) {
		{
			unique_lock<mutex> ulk( s_mutex );
			if( s_ready )	// 消费者尚未处理完前一个通知
				continue;

			s_post_time = rdtscp();
			s_ready = true;
		}
		s_cv.notify_one();
		++post_count;
		this_thread::sleep_for( SEND_INTERVEL );
	}
	cout << "producer:ended." << endl;
};

void ThreadConsumer() {
	cout << "consumer:started..." << endl;
	uint64_t recv_tsc;
	while( s_recv_count < TOTAL_NOTES ) {
		unique_lock<mutex> ulk( s_mutex );
		s_cv.wait( ulk, []() { return s_ready; } );
		// 先在第一时间获取时戳
		recv_tsc = rdtscp();
		if( !s_ready )	// 会不会有虚假唤醒呢?
			continue;

		s_total_delay += recv_tsc - s_post_time;
		++s_recv_count;
		s_ready = false;
		ulk.unlock();
	}
	cout << "consumer:ended." << endl;
};

int main( void ) {
	cout << "main:Conditional Var in same process." << endl;

	thread consumer = thread( ThreadConsumer );
	this_thread::sleep_for( 100ms );

	ThreadProducer();
	consumer.join();

	cout << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay / double( s_recv_count ) << endl;
	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
