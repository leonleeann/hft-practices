#include <atomic>
#include <chrono>
#include <cstring>
#include <fcntl.h>		//O_RDONLY, S_IRUSR, S_IWUSR
#include <iomanip>
#include <iostream>
#include <sys/mman.h>	// shm_open
#include <thread>
#include <unistd.h>		// ftruncate

#include "common.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std;

int main( void ) {
	auto shm_fd = shm_open( SM_NAME, O_RDWR, S_IRUSR | S_IWUSR );
	if( shm_fd < 0 ) {
		cerr << "producer:shm_open error:" << shm_fd << endl;
		exit( EXIT_FAILURE );
	}
	if( ftruncate( shm_fd, sizeof( atomic_int64_t ) ) != 0 ) {
		cerr << "producer:ftruncate error:" << shm_fd << endl;
		exit( EXIT_FAILURE );
	};
	auto shm_pt = mmap( NULL, sizeof( atomic_int64_t ),
						PROT_READ | PROT_WRITE, MAP_SHARED_VALIDATE, shm_fd, 0 );
	if( shm_pt == MAP_FAILED ) {
		std::cerr << "producer:mmap error:" << shm_pt;
		exit( EXIT_FAILURE );
	};
	atomic_int64_t* post_time = reinterpret_cast<atomic_int64_t*>( shm_pt );
	cout << "producer:shared memory created." << endl;

	cout << "producer:started..." << endl;
	for( int j = 0; j < TOTAL_NOTES; ++j ) {
		post_time->store( steady_clock::now().time_since_epoch().count() );
		this_thread::sleep_for( SEND_INTERVEL );
	}
	cout << "producer:ended." << endl;

	munmap( shm_pt, sizeof( steady_clock::time_point ) );
	return EXIT_SUCCESS;
};
