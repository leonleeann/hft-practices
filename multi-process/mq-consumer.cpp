#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <mqueue.h>
#include <string>

#include "common.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std;

int main( void ) {
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

	Msg_u buf;
	steady_clock::duration total_delay {};
	int recv_count = 0;
	while( recv_count < TOTAL_NOTES ) {
		auto bytes = mq_receive( mq, buf.as_buffer, sizeof( Msg_u ), 0 );
		auto recv_time = steady_clock::now();
		if( bytes != sizeof( Msg_u ) ) {
			cerr << "consumer:" << strerror( errno ) << endl;
			mq_close( mq );
			return EXIT_FAILURE;
		}
// 		if( buf.as_signal == END_SIGNAL )
// 			break;

		total_delay += recv_time - buf.post_time;
		++recv_count;
	}

	cout << "msgs:" << recv_count
		 << ", rate:" << total_delay.count() / double( recv_count ) << endl;
	mq_close( mq );
	return EXIT_SUCCESS;
};
