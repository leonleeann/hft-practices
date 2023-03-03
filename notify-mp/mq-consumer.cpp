#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <mqueue.h>
#include <string>

#include "common.hpp"

using namespace std;

uint64_t	s_total_delay {};
int			s_recv_count {};

int main( void ) {
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
		if( bytes != sizeof( Msg_u ) ) {
			cerr << "consumer:" << strerror( errno ) << endl;
			mq_close( mq );
			return EXIT_FAILURE;
		}

		s_total_delay += recv_tsc - r_buf.as_tstamp;
		++s_recv_count;
	}

	cout << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay / double( s_recv_count ) << endl;
	mq_close( mq );
	return EXIT_SUCCESS;
};
