// #include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <mqueue.h>
#include <thread>

#include "common.hpp"
#include "rdtscp.hpp"

// using namespace std::chrono;
// using namespace std::chrono_literals;
using namespace std;

int s_recv_count {};
uint64_t s_total_delay {};

void ThreadProducer() {
	auto mq = mq_open( MQ_NAME, O_WRONLY | O_NONBLOCK );
	mq_attr attr {};
	if( mq_getattr( mq, &attr ) == 0 )
		cout << "\nproducer.mq_flags  :" << attr.mq_flags
			 << "\nproducer.mq_maxmsg :" << attr.mq_maxmsg
			 << "\nproducer.mq_msgsize:" << attr.mq_msgsize
			 << "\nproducer.mq_curmsgs:" << attr.mq_curmsgs
			 << endl;
	else
		cerr << "producer:" << strerror( errno ) << endl;

	Msg_u s_buf;
	for( int j = 0; j < TOTAL_NOTES; ++j ) {
// 		s_buf.post_time = steady_clock::now();
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
			 << "\nconsumer.mq_maxmsg :" << cfg.mq_maxmsg
			 << "\nconsumer.mq_msgsize:" << cfg.mq_msgsize
			 << "\nconsumer.mq_curmsgs:" << cfg.mq_curmsgs
			 << endl;
	else
		cerr << "consumer:" << strerror( errno ) << endl;

	Msg_u r_buf;
	uint64_t recv_tsc;
	while( s_recv_count < TOTAL_NOTES ) {
		auto bytes = mq_receive( mq, r_buf.as_buffer, sizeof( Msg_u ), 0 );
// 		auto recv_time = steady_clock::now();
		recv_tsc = rdtscp();
		if( bytes != sizeof( Msg_u ) ) {
			cerr << "consumer:" << strerror( errno ) << endl;
			mq_close( mq );
			return;
		}

// 		s_total_delay += recv_time - r_buf.post_time;
		s_total_delay += recv_tsc - r_buf.as_tstamp;
		++s_recv_count;
	}

	mq_close( mq );
	cout << "consumer:ended." << endl;
};

int main( void ) {
	cout << "main:Hello!" << endl;
	thread consumer = thread( ThreadConsumer );
	this_thread::sleep_for( 100ms );
	cout << "Consumer has been started...";

	ThreadProducer();
	consumer.join();

	cout << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay.count() / double( s_recv_count ) << endl;
	return EXIT_SUCCESS;
};
