#include <iomanip>
#include <iostream>
#include <rpc/client.h>
#include <string>
#include <thread>

#include "common.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std;

int main( void ) {
	rpc::client rpc_clt( "127.0.0.1", RPC_PORT );
	cout << "producer:started..." << endl;
	for( int j = 0; j < TOTAL_NOTES; ++j ) {
		rpc_clt.call( RPC_NAME, steady_clock::now().time_since_epoch().count() );
		this_thread::sleep_for( SEND_INTERVEL );
	}
	cout << "producer:ended." << endl;

	return EXIT_SUCCESS;
};
