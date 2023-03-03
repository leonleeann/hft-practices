#include <atomic>
#include <chrono>
#include <cstring>
#include <fcntl.h>		//O_RDONLY, S_IRUSR, S_IWUSR
#include <iomanip>
#include <iostream>
#include <sys/mman.h>	// shm_open
#include <unistd.h>		// ftruncate

#include "common.hpp"

using namespace std;

int main( void ) {
	auto shm_fd = shm_open( SM_NAME, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR );
	if( shm_fd < 0 ) {
		cerr << "consumer:shm_open error:" << shm_fd << endl;
		exit( EXIT_FAILURE );
	}
	if( ftruncate( shm_fd, sizeof( atomic_uint64_t ) ) != 0 ) {
		cerr << "consumer:ftruncate error:" << strerror( errno ) << endl;
		exit( EXIT_FAILURE );
	};
	auto shm_pt = mmap( NULL, sizeof( atomic_uint64_t ),
						PROT_READ | PROT_WRITE, MAP_SHARED_VALIDATE, shm_fd, 0 );
	if( shm_pt == MAP_FAILED ) {
		std::cerr << "consumer:mmap error:" << shm_pt;
		exit( EXIT_FAILURE );
	};
	atomic_uint64_t* post_time = reinterpret_cast<atomic_uint64_t*>( shm_pt );
	cout << "consumer:shared memory created." << endl;

	cout << "consumer:started..." << endl;
	uint64_t	total_delay {}, last_post {}, new_post {};
	int		recv_count = 0;
	while( recv_count < TOTAL_NOTES ) {
		new_post = post_time->load();
		if( new_post == last_post )
			continue;

		total_delay += rdtscp() - new_post;
		last_post = new_post;
		++recv_count;
	}
	cout << "consumer:ended." << endl;

	cout << "msgs:" << recv_count
		 << ", rate:" << total_delay / double( recv_count ) << endl;

	munmap( shm_pt, sizeof( atomic_uint64_t ) );
	shm_unlink( SM_NAME );
	return EXIT_SUCCESS;
};
