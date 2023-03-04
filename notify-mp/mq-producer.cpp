#include <cstring>
#include <iomanip>
#include <iostream>
#include <mqueue.h>
#include <thread>

#include "common.hpp"
#include "mq.hpp"

using namespace std;

int main( void ) {
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
	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
