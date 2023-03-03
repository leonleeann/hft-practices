#include <atomic>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <mqueue.h>
#include <thread>

#include "common.hpp"

using namespace std;

uint64_t	s_total_delay {};
int			s_recv_count {};
atomic_bool	s_ready { false };

void ThreadProducer() {
	auto mq = mq_open( MQ_NAME, O_WRONLY | O_NONBLOCK );
	mq_attr attr {};
	if( mq_getattr( mq, &attr ) == 0 )
		cout << "\nproducer.mq_flags  :" << attr.mq_flags
// 			 << "\nproducer.mq_maxmsg :" << attr.mq_maxmsg
// 			 << "\nproducer.mq_msgsize:" << attr.mq_msgsize
// 			 << "\nproducer.mq_curmsgs:" << attr.mq_curmsgs
			 << endl;
	else
		cerr << "producer:" << strerror( errno ) << endl;

	Msg_u s_buf;
	for( int j = 0; j < TOTAL_NOTES; ++j ) {
		if( s_ready.load( memory_order_acquire ) )	// 消费者尚未处理完前一个通知
			continue;

		s_buf.as_tstamp = rdtscp();
		if( mq_send( mq, s_buf.as_buffer, sizeof( Msg_u ), 0 ) != 0 ) {
			// 错误信息要先留存
			string send_err = strerror( errno );
			cerr << "producer:" << send_err << endl;
			if( mq_getattr( mq, &attr ) < 0 )
				cerr << "producer:获取属性也失败(" << strerror( errno ) << ")!" << endl;
			if( attr.mq_curmsgs >= attr.mq_maxmsg - 1 )
				cerr << "producer:可能队列已满!" << endl;
			break;
		}
		s_ready.store( true, memory_order_release );
		this_thread::sleep_for( SEND_INTERVEL );
	}

	mq_close( mq );
	cout << "producer:ended." << endl;
};

void ThreadConsumer() {
	mq_attr cfg {};
//	cfg.mq_curmsgs = 0;
//	cfg.mq_flags = O_CREAT | O_RDONLY | O_WRONLY;
	cfg.mq_maxmsg = 16;
	cfg.mq_msgsize = sizeof( Msg_u );

	auto mq = mq_open( MQ_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &cfg );
	if( mq_getattr( mq, &cfg ) == 0 )
		cout << "\nconsumer.mq_flags  :" << cfg.mq_flags
// 			 << "\nconsumer.mq_maxmsg :" << cfg.mq_maxmsg
// 			 << "\nconsumer.mq_msgsize:" << cfg.mq_msgsize
// 			 << "\nconsumer.mq_curmsgs:" << cfg.mq_curmsgs
			 << endl;
	else
		cerr << "consumer:" << strerror( errno ) << endl;

	Msg_u r_buf;
	uint64_t recv_tsc;
	while( s_recv_count < TOTAL_NOTES ) {
		auto bytes = mq_receive( mq, r_buf.as_buffer, sizeof( Msg_u ), 0 );
		// 先在第一时间获取时戳
		recv_tsc = rdtscp();
		if( bytes != sizeof( Msg_u ) || ! s_ready.load( memory_order_acquire ) ) {
			cerr << "consumer:" << strerror( errno ) << endl;
			mq_close( mq );
			return;
		}

		s_total_delay += recv_tsc - r_buf.as_tstamp;
		++s_recv_count;
		s_ready.store( false, memory_order_release );
	}

	mq_close( mq );
	cout << "consumer:ended." << endl;
};

int main( void ) {
	cout << "main:MsgQue in same process." << endl;
	thread consumer = thread( ThreadConsumer );
	this_thread::sleep_for( 100ms );
	cout << "Consumer has been started...";

	ThreadProducer();
	consumer.join();

	cout << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay / double( s_recv_count ) << endl;
	return EXIT_SUCCESS;
};
