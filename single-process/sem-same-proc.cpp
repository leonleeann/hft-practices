// #include <atomic>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <semaphore.h>
#include <thread>

#include "common.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std;

// atomic<steady_clock::time_point> s_post_time { steady_clock::time_point { 0s } };
steady_clock::time_point s_post_time {};
sem_t	s_note_sem;

int						s_recv_count = 0;
steady_clock::duration	s_total_delay {};

void ThreadProducer() {
	cout << "producer:started..." << endl;
	for( int j = 0; j < TOTAL_NOTES; ++j ) {
// 		s_post_time.store( steady_clock::now(), std::memory_order_release );
		s_post_time = steady_clock::now();
		if( sem_post( &s_note_sem ) != 0 ) {
			cerr << "producer:" << strerror( errno ) << endl;
			break;
		}
		this_thread::sleep_for( SEND_INTERVEL );
	}
	cout << "producer:ended." << endl;
};

void ThreadConsumer() {
	cout << "consumer:started..." << endl;
	while( s_recv_count < TOTAL_NOTES ) {
		if( sem_wait( &s_note_sem ) != 0 ) {
			cerr << "consumer:" << strerror( errno ) << endl;
			return;
		}

		auto recv_time = steady_clock::now();
// 		s_total_delay += recv_time - s_post_time.load( std::memory_order_acquire );
		s_total_delay += recv_time - s_post_time;
		++s_recv_count;
	}
	cout << "consumer:ended." << endl;
};

int main( void ) {
	cout << "main:Hello!" << endl;
	if( sem_init( &s_note_sem, 0, 0 ) != 0 ) {
		cerr << "main:failed on create semaphore!" << endl;
		return EXIT_FAILURE;
	}
	cout << "main:semaphore created." << endl;

	thread consumer = thread( ThreadConsumer );
	this_thread::sleep_for( 100ms );

	ThreadProducer();
	consumer.join();
	sem_destroy( &s_note_sem );

	cout << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay.count() / double( s_recv_count ) << endl;
	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
