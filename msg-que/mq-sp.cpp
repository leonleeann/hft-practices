#include <atomic>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <thread>

#include "common.hpp"
#include "mq.hpp"

using namespace std;

atomic_uint64_t	s_push_delay { 0 };
atomic_uint64_t	s_push_count { 0 };
atomic_uint64_t	s_pop_delay { 0 };
atomic_uint64_t	s_pop_count { 0 };
atomic_bool		s_should_run { true };
atomic_bool		s_failed { false };

void ThreadProducer( int my_id ) {
	auto post_mq = mq_open( MQ_NAME, O_WRONLY | O_NONBLOCK );
	if( post_mq < 0 ) {
		cerr << "producer" << my_id << ":创建失败:" << strerror( errno ) << endl;
		s_failed.store( true );
		s_should_run.store( false );
		return;
	}

	Msg_u		msg;
	size_t		count = 0;
	uint64_t	delay = 0;
	while( s_should_run.load() && count < MQ_MAX_SIZE - 2 ) {
		msg.as_tstamp = rdtscp();
		if( mq_send( post_mq, msg.as_buffer, sizeof( Msg_u ), 0 ) != 0 ) {
			showMqStatus( post_mq, "producer" + to_string( my_id ), strerror( errno ) );
			s_failed.store( true );
			s_should_run.store( false );
			return;
		}
		delay += rdtscp() - msg.as_tstamp;
		++count;
	}
	s_push_delay += delay;
	s_push_count += count;
	mq_close( post_mq );
};

void ThreadConsumer( int my_id ) {
	mq_attr cfg {};
//	cfg.mq_curmsgs = 0;
//	cfg.mq_flags = O_CREAT | O_RDONLY | O_WRONLY;
	cfg.mq_maxmsg = MQ_MAX_SIZE;
	cfg.mq_msgsize = sizeof( Msg_u );
	auto recv_mq = mq_open( MQ_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &cfg );
	if( recv_mq < 0 ) {
		cerr << "consumer" << my_id << ":创建失败:" << strerror( errno ) << endl;
		s_failed.store( true );
		s_should_run.store( false );
		return;
	}

	Msg_u		msg;
	timespec	ts {};
	uint64_t	recv_bef, recv_aft;
	while( s_should_run.load() ) {
		recv_bef = rdtscp();
		// 全零的timespec,会导致mq_timedreceive立即超时,相当于busy check
		auto bytes = mq_timedreceive( recv_mq, msg.as_buffer, sizeof( Msg_u ), 0, &ts );
		recv_aft = rdtscp();
		if( bytes <= 0 )
			continue;
		else if( bytes != sizeof( Msg_u ) ) {
			showMqStatus( recv_mq, "consumer" + to_string( my_id ), strerror( errno ) );
			s_failed.store( true );
			s_should_run.store( false );
			return;
		}
		s_pop_delay += recv_aft - recv_bef;
		++s_pop_count;
	}
	mq_close( recv_mq );
};

int main( void ) {
	cout << "main:MsgQue in same process." << endl;
	mq_unlink( MQ_NAME );

	forward_list<thread> consumers;
	for( int j = 0; j < THREADS; ++j )
		consumers.emplace_front( ThreadConsumer, j );

	forward_list<thread> producers;
	for( int j = 0; j < THREADS; ++j )
		producers.emplace_front( ThreadProducer, j );

	for( auto& thrd : producers )
		thrd.join();
	s_should_run.store( false );
	for( auto& thrd : consumers )
		thrd.join();

	if( s_failed.load() )
		exit( EXIT_FAILURE );

	cout << "pushed:" << s_push_count.load()
		 << ", rate:" << s_push_delay.load() / double( s_push_count.load() )
		 << endl;
	cout << "popped:" << s_pop_count.load()
		 << ", rate:" << s_pop_delay.load() / double( s_pop_count.load() )
		 << endl;

	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
