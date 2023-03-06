#include <atomic>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

#include "SHMRQ.hpp"
#include "common.hpp"

using namespace std;

SHMRQ_t<uint64_t>	s_rq( SHQ_NAME, TOTAL_NOTES, true );

forward_list<uint64_t>	s_all_pushed;
forward_list<uint64_t>	s_all_popped;
uint64_t	s_push_delay { 0 };
uint64_t	s_push_count { 0 };
uint64_t	s_pop_delay { 0 };
uint64_t	s_pop_count { 0 };

int			s_consumers { 1 };
int			s_producers { 1 };
mutex		s_writ_mutex;
atomic_bool	s_should_run { true };

void ThreadProducer() {
	forward_list<uint64_t>	pushed;
	uint64_t				count = 0, delay = 0, stamp;
	while( count < TOTAL_NOTES ) {
		stamp = rdtscp();
		if( s_rq.enque( stamp ) ) {
			delay += rdtscp() - stamp;
			pushed.push_front( stamp );
			++count;
		}
	}

	{
		unique_lock<mutex> lk( s_writ_mutex );
		s_push_delay += delay;
		s_push_count += count;
		s_all_pushed.merge( pushed );
	}
};

void ThreadConsumer() {
	forward_list<uint64_t>	popped;
	uint64_t	push_stamp, stamp, count = 0, delay = 0;
	while( true ) {
		stamp = rdtscp();
		if( s_rq.deque( push_stamp ) ) {
			delay += rdtscp() - stamp;
			popped.push_front( push_stamp );
			++count;
		} else if( ! s_should_run.load() )
			break;
	}

	{
		unique_lock<mutex> lk( s_writ_mutex );
		s_pop_delay += delay;
		s_pop_count += count;
		s_all_popped.merge( popped );
	}
};

int main( int argc, const char* const* const args ) {
	bool opt_err = false;
	for( int i = 1; i < argc; ++i ) {
		string argv = args[i];
//-------- 一般选项 ------------------------------------------------
		if( argv == "-C" || argv == "--consumers" ) {
			if( !( opt_err = ++i >= argc ) )
				s_consumers = atoi( args[i] );
		} else if( argv == "-P" || argv == "--producers" ) {
			if( !( opt_err = ++i >= argc ) )
				s_producers = atoi( args[i] );
//-------- 未知的选项 --------
		} else
			opt_err = true;

		if( opt_err ) {
			cerr << '"' << argv << "\" 解析出错,无法继续!" << endl;
			exit( EXIT_FAILURE );
		}
	}
	cout << "main:SHMRQ in "
		 << s_consumers << " consumers with "
		 << s_producers << " producers." << endl;
	forward_list<thread> consumers;
	for( int j = 0; j < s_consumers; ++j )
		consumers.emplace_front( ThreadConsumer );

	forward_list<thread> producers;
	for( int j = 0; j < s_producers; ++j )
		producers.emplace_front( ThreadProducer );

	for( auto& thrd : producers )
		thrd.join();
	s_should_run.store( false );
	for( auto& thrd : consumers )
		thrd.join();

	cout << "pushed:" << s_push_count
		 << ", rate:" << s_push_delay / double( s_push_count )
		 << endl;
	cout << "popped:" << s_pop_count
		 << ", rate:" << s_pop_delay / double( s_pop_count )
		 << endl;

	s_all_popped.sort();
	s_all_pushed.sort();
	if( s_all_popped == s_all_pushed )
		cout << "接收与发送完全相等!" << endl;
	else
		cout << "接收与发送不相等!!!" << endl;

	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
