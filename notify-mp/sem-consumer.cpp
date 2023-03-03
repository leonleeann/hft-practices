#include <chrono>
#include <cstring>
#include <fcntl.h>		//O_RDONLY, S_IRUSR, S_IWUSR
#include <iomanip>
#include <iostream>
#include <semaphore.h>	// sem_init
#include <sys/mman.h>	// shm_open
#include <unistd.h>		// ftruncate

#include "common.hpp"

using namespace std;

uint64_t	s_total_delay {};
int			s_recv_count {};

int main( void ) {
	auto shm_fd = shm_open( SM_NAME, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR );
	if( shm_fd < 0 ) {
		cerr << "consumer:shm_open error:" << shm_fd << endl;
		exit( EXIT_FAILURE );
	}
	if( ftruncate( shm_fd, sizeof( uint64_t ) ) != 0 ) {
		cerr << "consumer:ftruncate error:" << strerror( errno ) << endl;
		exit( EXIT_FAILURE );
	};
	auto shm_pt = mmap( NULL, sizeof( uint64_t ),
						PROT_READ, MAP_SHARED_VALIDATE, shm_fd, 0 );
	if( shm_pt == MAP_FAILED ) {
		std::cerr << "consumer:mmap error:" << shm_pt;
		exit( EXIT_FAILURE );
	};
	uint64_t* post_time = reinterpret_cast<uint64_t*>( shm_pt );
	cout << "consumer:shared memory created." << endl;

	sem_t*		note_sem = sem_open( SEM_NAME, O_CREAT, S_IRUSR | S_IWUSR, 0 );
	if( note_sem == SEM_FAILED ) {
		cerr << "consumer:failed on create semaphore!" << endl;
		return EXIT_FAILURE;
	}
	cout << "consumer:semaphore created." << endl;

	cout << "consumer:started..." << endl;
	while( s_recv_count < TOTAL_NOTES ) {
		if( sem_wait( note_sem ) != 0 ) {
			cerr << "consumer:" << strerror( errno ) << endl;
			return EXIT_FAILURE;
		}

		s_total_delay += rdtscp() - *post_time;
		++s_recv_count;
	}
	cout << "consumer:ended." << endl;

	cout << "msgs:" << s_recv_count
		 << ", rate:" << s_total_delay / double( s_recv_count ) << endl;

	sem_close( note_sem );
	sem_unlink( SEM_NAME );
	munmap( shm_pt, sizeof( uint64_t ) );
	shm_unlink( SM_NAME );
	return EXIT_SUCCESS;
};
