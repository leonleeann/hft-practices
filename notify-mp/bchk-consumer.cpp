#include <atomic>
#include <cstring>
#include <fcntl.h>		//O_RDONLY, S_IRUSR, S_IWUSR
#include <iomanip>
#include <iostream>
#include <sys/mman.h>	// shm_open
#include <unistd.h>		// ftruncate

#include "common.hpp"

using namespace std;

uint64_t		s_total_delay { 0 };
int				s_recv_count { 0 };

int main( void ) {
	auto shm_fd = shm_open( SHM_NAME, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR );
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
	atomic_uint64_t* p_post_time = reinterpret_cast<atomic_uint64_t*>( shm_pt );
	*p_post_time = 0;
	cout << "consumer:shared memory created." << endl;

	cout << "consumer:started..." << endl;
	uint64_t stamp = 0;
	while( s_recv_count < TOTAL_NOTES ) {
		stamp = p_post_time->load( memory_order_acquire );
		if( stamp == 0 )
			continue;

		s_total_delay += rdtscp() - stamp;
		++s_recv_count;
		p_post_time->store( 0, memory_order_release );
	}
	cout << "consumer:ended." << endl;

	cout << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay / double( s_recv_count ) << endl;

	munmap( shm_pt, sizeof( atomic_uint64_t ) );
	shm_unlink( SHM_NAME );
	return EXIT_SUCCESS;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
