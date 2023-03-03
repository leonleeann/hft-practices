#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <mqueue.h>
#include <string>
#include <thread>

#include "common.hpp"

using namespace std;

int main( void ) {
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

	Msg_u buf;
	for( int j = 0; j < TOTAL_NOTES; ++j ) {
		buf.as_tstamp = rdtscp();
		if( mq_send( mq, buf.as_buffer, sizeof( Msg_u ), 0 ) != 0 ) {
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
	return EXIT_SUCCESS;
};
