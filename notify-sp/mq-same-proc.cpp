#include <cstring>
#include <iomanip>
#include <iostream>
#include <thread>

#include "common.hpp"
#include "mq.hpp"

using namespace std;

uint64_t	s_total_delay {};
int			s_recv_count {};

void ThreadProducer() {
	cout << "producer:started..." << endl;

	auto mq = mq_open( MQ_NAME, O_WRONLY | O_NONBLOCK );
	showMqStatus( mq, "producer", "初创状态" );

	Msg_u buf;
	for( int j = 0; j < TOTAL_NOTES; ++j ) {
		buf.as_tstamp = rdtscp();
		if( mq_send( mq, buf.as_buffer, sizeof( Msg_u ), 0 ) != 0 ) {
			showMqStatus( mq, "producer", strerror( errno ) );
			break;
		}
		this_thread::sleep_for( SEND_INTERVEL );
	}

	cout << "producer:ended." << endl;
	mq_close( mq );
};

void ThreadConsumer() {
	cout << "consumer:started..." << endl;

	mq_attr cfg {};
//	cfg.mq_curmsgs = 0;
//	cfg.mq_flags = O_CREAT | O_RDONLY | O_WRONLY;
	cfg.mq_maxmsg = 16;
	cfg.mq_msgsize = sizeof( Msg_u );

	auto mq = mq_open( MQ_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &cfg );
	showMqStatus( mq, "consumer", "初创状态" );

	Msg_u buf;
	uint64_t recv_tsc;
	while( s_recv_count < TOTAL_NOTES ) {
		auto bytes = mq_receive( mq, buf.as_buffer, sizeof( Msg_u ), 0 );
		// 先在第一时间获取时戳
		recv_tsc = rdtscp();
		if( bytes != sizeof( Msg_u ) ) {
			showMqStatus( mq, "consumer", strerror( errno ) );
			mq_close( mq );
			return;
		}

		s_total_delay += recv_tsc - buf.as_tstamp;
		++s_recv_count;
	}
	cout << "consumer:ended." << endl;

	mq_close( mq );
};

int main( void ) {
	cout << "main:MsgQue in same process." << endl;

	mq_unlink( MQ_NAME );
	thread consumer = thread( ThreadConsumer );
	this_thread::sleep_for( 100ms );

	ThreadProducer();
	consumer.join();

	cout << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay / double( s_recv_count ) << endl;
	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
