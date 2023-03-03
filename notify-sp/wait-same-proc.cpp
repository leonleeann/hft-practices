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

atomic<steady_clock::time_point> s_post_time {};
int						s_recv_count = 0;
steady_clock::duration	s_total_delay {};

void ThreadProducer() {
	cout << "producer:started..." << endl;
	int post_count = 0;
	steady_clock::time_point last_post = steady_clock::now();
	while( post_count < TOTAL_NOTES ) {
		s_post_time.wait( last_post );
		if( s_post_time.load( memory_order_acquire ) == last_post )
			// 消费者尚未处理完前一个通知
			continue;

		last_post = steady_clock::now();
		s_post_time.store( last_post, memory_order_release );
		s_post_time.notify_one();
		++post_count;
		this_thread::sleep_for( SEND_INTERVEL );
	}
	cout << "producer:ended." << endl;
};

void ThreadConsumer() {
	cout << "consumer:started..." << endl;
	steady_clock::time_point last_recv {}, new_post {};
	while( s_recv_count < TOTAL_NOTES ) {
		s_post_time.wait( last_recv );
		new_post = s_post_time.load( memory_order_acquire );
		if( new_post == last_recv )
			// 会有虚假唤醒吗?
			continue;

		last_recv = steady_clock::now();
		s_total_delay += last_recv - new_post;
		s_post_time.store( last_recv, memory_order_release );
		s_post_time.notify_one();
		++s_recv_count;
	};
	cout << "consumer:ended." << endl;
};

int main( void ) {
	cout << "main:atomic<time_point> is lock free:" << s_post_time.is_lock_free()
		 << endl;

	thread consumer = thread( ThreadConsumer );
	this_thread::sleep_for( 100ms );

	ThreadProducer();
	consumer.join();

	cout << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay.count() / double( s_recv_count ) << endl;
	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
