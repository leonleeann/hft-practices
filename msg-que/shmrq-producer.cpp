#include <atomic>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <thread>

#include "SHMRQ.hpp"
#include "common.hpp"

using namespace std;

SHMRQ_t<Msg_u>	s_rq( SHQ_NAME, TOTAL_NOTES, false );
atomic_uint64_t	s_push_delay { 0 };
atomic_uint64_t	s_push_count { 0 };

void ThreadProducer() {
	Msg_u		msg;
	uint64_t	count = 0, delay = 0;
	while( count < TOTAL_NOTES ) {
		msg.as_tstamp = rdtscp();
		if( s_rq.enque( msg ) ) {
			delay += rdtscp() - msg.as_tstamp;
			++count;
		}
	}
	s_push_delay += delay;
	s_push_count += count;
};

int main( void ) {
	cout << "main:SHMRQ producers." << endl;

	forward_list<thread> producers;
	for( int j = 0; j < THREADS; ++j )
		producers.emplace_front( ThreadProducer );

	for( auto& thrd : producers )
		thrd.join();

	cout << "pushed:" << s_push_count.load()
		 << ", rate:" << s_push_delay.load() / double( s_push_count.load() )
		 << endl;

	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
