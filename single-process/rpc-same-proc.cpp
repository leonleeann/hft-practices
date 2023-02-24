#include <atomic>
#include <iomanip>
#include <iostream>
#include <rpc/client.h>
#include <rpc/server.h>
#include <string>
#include <thread>

#include "common.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std;

int main() {
	atomic_int64_t total_delay { 0 };
	atomic_int64_t recv_count { 0 };
	rpc::server rpc_svr( "127.0.0.1", RPC_PORT );
	rpc_svr.bind( RPC_NAME, [&]( int64_t ns ) {
		total_delay += steady_clock::now().time_since_epoch().count() - ns;
		++recv_count;
	} );
	rpc_svr.async_run();
	cout << "consumer:started..." << endl;

	rpc::client rpc_clt( "127.0.0.1", RPC_PORT );
	cout << "producer:started..." << endl;
	for( int j = 0; j < TOTAL_NOTES; ++j ) {
		rpc_clt.call( RPC_NAME, steady_clock::now().time_since_epoch().count() );
		this_thread::sleep_for( SEND_INTERVEL );
	}
	cout << "producer:ended." << endl;

	rpc_svr.stop();
	cout << "consumer:ended." << endl;

	cout << "msgs:" << recv_count.load()
		 << ", rate:" << total_delay.load() / double( recv_count.load() ) << endl;
	return EXIT_SUCCESS;
};
