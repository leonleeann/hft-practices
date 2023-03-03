#include <atomic>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <thread>

#include "common.hpp"

using namespace std;

uint64_t	s_post_time {};
uint64_t	s_total_delay {};
int			s_recv_count {};
atomic_bool	s_ready { false };

void ThreadProducer() {
	cout << "producer:started..." << endl;
	int post_count = 0;
	while( post_count < TOTAL_NOTES ) {
		if( s_ready.load( memory_order_acquire ) )	// 消费者尚未处理完前一个通知
			continue;

		s_post_time = rdtscp();
		s_ready.store( true, memory_order_release );
		++post_count;
		this_thread::sleep_for( SEND_INTERVEL );
	}
	cout << "producer:ended." << endl;
};

void ThreadConsumer() {
	cout << "consumer:started..." << endl;
	while( s_recv_count < TOTAL_NOTES )
		if( s_ready.load( memory_order_acquire ) ) {
			s_total_delay += rdtscp() - s_post_time;
			++s_recv_count;
			s_ready.store( false, memory_order_release );
		}
// 		else this_thread::sleep_for( 1ns );
	cout << "consumer:ended." << endl;
};

int main( void ) {
	cout << "main:atomic busy check." << endl;

	thread consumer = thread( ThreadConsumer );
	this_thread::sleep_for( 100ms );

	ThreadProducer();
	consumer.join();

	cout << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay / double( s_recv_count ) << endl;
	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
