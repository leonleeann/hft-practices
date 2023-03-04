#include <atomic>
#include <iomanip>
#include <iostream>
#include <rpc/server.h>
#include <thread>

#include "common.hpp"

using namespace std;

uint64_t	s_total_delay {};
atomic_int	s_recv_count {};

int main( void ) {
	rpc::server rpc_svr( "127.0.0.1", RPC_PORT );
	rpc_svr.bind( RPC_NAME, [&]( uint64_t post_stamp ) {
		s_total_delay += rdtscp() - post_stamp;
		++ s_recv_count;
	} );
	rpc_svr.async_run();
	cout << "consumer:started..." << endl;

	while( s_recv_count.load() < TOTAL_NOTES )
		this_thread::sleep_for( 1ms );
	rpc_svr.stop();
	cout << "consumer:ended." << endl;

	cout << "msgs:" << s_recv_count.load()
		 << ", rate:" << s_total_delay / double( s_recv_count.load() ) << endl;
	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
