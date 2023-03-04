#include <atomic>
#include <iomanip>
#include <iostream>
#include <thread>

#include "common.hpp"

using namespace std;

atomic_uint64_t	s_post_time { 0 };
uint64_t		s_total_delay { 0 };
int				s_recv_count { 0 };

void ThreadProducer() {
	cout << "producer:started..." << endl;

	int post_count = 0;
	while( post_count < TOTAL_NOTES ) {
		// 消费者尚未处理完前一个通知
		if( s_post_time.load( memory_order_acquire ) != 0 )
			continue;

		s_post_time.store( rdtscp(), memory_order_release );
		++post_count;
		this_thread::sleep_for( SEND_INTERVEL );
	}

	cout << "producer:ended." << endl;
};

void ThreadConsumer() {
	cout << "consumer:started..." << endl;

	uint64_t stamp = 0;
	while( s_recv_count < TOTAL_NOTES ) {
		stamp = s_post_time.load( memory_order_acquire );
		if( stamp == 0 )
			continue;

		s_total_delay += rdtscp() - stamp;
		++s_recv_count;
		s_post_time.store( 0, memory_order_release );
	}

	auto last_st = rdtscp();	// 保证CacheLine已同步
	cout << "consumer:ended." << last_st << endl;
};

int main( void ) {
	cout << "main:Atomic busy check." << endl;

	thread consumer = thread( ThreadConsumer );
	this_thread::sleep_for( 100ms );

	ThreadProducer();
	consumer.join();
	auto last_st = rdtscp();	// 保证CacheLine已同步

	cout << last_st << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay / double( s_recv_count ) << endl;
	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
