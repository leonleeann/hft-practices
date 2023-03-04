#include <iomanip>
#include <iostream>
#include <rpc/client.h>
#include <rpc/server.h>
#include <thread>

#include "common.hpp"

using namespace std;

uint64_t	s_total_delay {};
int			s_recv_count {};

int main() {
	rpc::server rpc_svr( "127.0.0.1", RPC_PORT );
	rpc_svr.bind( RPC_NAME, [&]( uint64_t post_stamp ) {
		s_total_delay += rdtscp() - post_stamp;
		++ s_recv_count;
	} );
	rpc_svr.async_run();
	cout << "consumer:started..." << endl;

	rpc::client rpc_clt( "127.0.0.1", RPC_PORT );
	cout << "producer:started..." << endl;
	for( int j = 0; j < TOTAL_NOTES; ++j ) {
		rpc_clt.call( RPC_NAME, rdtscp() );
		this_thread::sleep_for( SEND_INTERVEL );
	}
	cout << "producer:ended." << endl;

	rpc_svr.stop();
	auto last_st = rdtscp();	// 保证CacheLine已同步
	cout << "consumer:ended." << last_st << endl;

	cout << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay / double( s_recv_count ) << endl;
	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
