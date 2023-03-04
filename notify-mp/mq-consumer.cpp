#include <cstring>
#include <iomanip>
#include <iostream>
#include <mqueue.h>

#include "common.hpp"
#include "mq.hpp"

using namespace std;

uint64_t	s_total_delay {};
int			s_recv_count {};

int main( void ) {
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
			return EXIT_FAILURE;
		}

		s_total_delay += recv_tsc - buf.as_tstamp;
		++s_recv_count;
	}
	cout << "consumer:ended." << endl;

	cout << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay / double( s_recv_count ) << endl;
	mq_close( mq );
	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
