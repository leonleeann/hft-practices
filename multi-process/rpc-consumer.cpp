#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <rpc/server.h>
#include <thread>

#include "common.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std;

int main( void ) {
	atomic_int64_t	total_delay { 0 };
	atomic_int32_t	recv_count { 0 };

	rpc::server rpc_svr( "127.0.0.1", RPC_PORT );
	rpc_svr.bind( RPC_NAME, [&]( int64_t ns ) {
		total_delay += steady_clock::now().time_since_epoch().count() - ns;
		++recv_count;
	} );
	rpc_svr.async_run();
	cout << "consumer:started..." << endl;

	while( recv_count.load() < TOTAL_NOTES )
		this_thread::sleep_for( 1ms );
	rpc_svr.stop();
	cout << "consumer:ended." << endl;

	cout << "msgs:" << recv_count.load()
		 << ", rate:" << total_delay.load() / double( recv_count ) << endl;
	return EXIT_SUCCESS;
};
