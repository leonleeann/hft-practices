#include <atomic>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <thread>

#include "common.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std;

steady_clock::time_point s_post_time {};
atomic_bool				s_ready { false };

int						s_recv_count = 0;
steady_clock::duration	s_total_delay {};

void ThreadProducer() {
	cout << "producer:started..." << endl;
	int post_count = 0;
	while( post_count < TOTAL_NOTES ) {
		if( s_ready.load( memory_order_acquire ) )	// 消费者尚未处理完前一个通知
			continue;

		s_post_time = steady_clock::now();
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
			auto recv_time = steady_clock::now();
			s_total_delay += recv_time - s_post_time;
			++s_recv_count;
			s_ready.store( false, memory_order_release );
		}
	cout << "consumer:ended." << endl;
};

int main( void ) {
	cout << "main:Hello Conditional Var!" << endl;

	thread consumer = thread( ThreadConsumer );
	this_thread::sleep_for( 100ms );

	ThreadProducer();
	consumer.join();

	cout << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay.count() / double( s_recv_count ) << endl;
	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
