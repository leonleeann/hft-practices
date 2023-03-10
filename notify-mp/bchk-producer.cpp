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

using namespace std;

int main( void ) {
	auto shm_fd = shm_open( SHM_NAME, O_RDWR, S_IRUSR | S_IWUSR );
	if( shm_fd < 0 ) {
		cerr << "producer:shm_open error:" << shm_fd << endl;
		exit( EXIT_FAILURE );
	}
	if( ftruncate( shm_fd, sizeof( atomic_uint64_t ) ) != 0 ) {
		cerr << "producer:ftruncate error:" << shm_fd << endl;
		exit( EXIT_FAILURE );
	};
	auto shm_pt = mmap( NULL, sizeof( atomic_uint64_t ),
						PROT_READ | PROT_WRITE, MAP_SHARED_VALIDATE, shm_fd, 0 );
	if( shm_pt == MAP_FAILED ) {
		std::cerr << "producer:mmap error:" << shm_pt;
		exit( EXIT_FAILURE );
	};
	atomic_uint64_t* p_post_time = reinterpret_cast<atomic_uint64_t*>( shm_pt );
	cout << "producer:shared memory created." << endl;

	cout << "producer:started..." << endl;
	int post_count = 0;
	while( post_count < TOTAL_NOTES ) {
		// 消费者尚未处理完前一个通知
		if( p_post_time->load( memory_order_acquire ) != 0 )
			continue;

		p_post_time->store( rdtscp(), memory_order_release );
		++post_count;
		this_thread::sleep_for( SEND_INTERVEL );
	}

	cout << "producer:ended." << endl;

	munmap( shm_pt, sizeof( atomic_uint64_t ) );
	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
