#include <atomic>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

#include "common.hpp"

using namespace std;
using FwdListMQ_t = forward_list<Msg_u>;

mutex					s_mutex;
FwdListMQ_t				s_que;
FwdListMQ_t::iterator	s_tail = s_que.before_begin();
atomic_uint64_t	s_push_delay { 0 };
atomic_uint64_t	s_push_count { 0 };
atomic_uint64_t	s_pop_delay { 0 };
atomic_uint64_t	s_pop_count { 0 };
atomic_bool		s_should_run { true };

void ThreadProducer() {
	Msg_u		msg;
	uint64_t	count = 0, delay = 0;
	while( s_should_run.load() && count < TOTAL_NOTES ) {
		msg.as_tstamp = rdtscp();
		{
			unique_lock<mutex> ulk( s_mutex );
			if( s_que.empty() )
				s_tail = s_que.before_begin();
			s_tail = s_que.insert_after( s_tail, msg );
		}
		delay += rdtscp() - msg.as_tstamp;
		++count;
	}
	s_push_delay += delay;
	s_push_count += count;
};

void ThreadConsumer() {
	Msg_u msg;
	uint64_t	recv_bef, count = 0, delay = 0;
	while( true ) {
		recv_bef = rdtscp();
		{
			unique_lock<mutex> ulk( s_mutex );
			if( s_que.empty() ) {
				if( s_should_run.load() )
					continue;
				else
					break;
			}
			msg = s_que.front();
			s_que.pop_front();
		}
		delay += rdtscp() - recv_bef;
		++count;
	}
	s_pop_delay += delay;
	s_pop_count += count;
};

int main( void ) {
	cout << "main:FwdList with lock in same process." << endl;

	forward_list<thread> producers;
	for( int j = 0; j < THREADS; ++j )
		producers.emplace_front( ThreadProducer );

	forward_list<thread> consumers;
	for( int j = 0; j < THREADS; ++j )
		consumers.emplace_front( ThreadConsumer );

	for( auto& thrd : producers )
		thrd.join();
	s_should_run.store( false );
	for( auto& thrd : consumers )
		thrd.join();

	cout << "pushed:" << s_push_count.load()
		 << ", rate:" << s_push_delay.load() / double( s_push_count.load() )
		 << endl;
	cout << "popped:" << s_pop_count.load()
		 << ", rate:" << s_pop_delay.load() / double( s_pop_count.load() )
		 << endl;

	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
