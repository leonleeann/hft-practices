#include <chrono>
#include <cstring>
#include <fcntl.h>		//O_RDONLY, S_IRUSR, S_IWUSR
#include <iomanip>
#include <iostream>
#include <semaphore.h>	// sem_init
#include <sys/mman.h>	// shm_open
#include <unistd.h>		// ftruncate

#include "common.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std;

int main( void ) {
	auto shm_fd = shm_open( SM_NAME, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR );
	if( shm_fd < 0 ) {
		cerr << "consumer:shm_open error:" << shm_fd << endl;
		exit( EXIT_FAILURE );
	}
	if( ftruncate( shm_fd, sizeof( steady_clock::time_point ) ) != 0 ) {
		cerr << "consumer:ftruncate error:" << strerror( errno ) << endl;
		exit( EXIT_FAILURE );
	};
	auto shm_pt = mmap( NULL, sizeof( steady_clock::time_point ),
						PROT_READ, MAP_SHARED_VALIDATE, shm_fd, 0 );
	if( shm_pt == MAP_FAILED ) {
		std::cerr << "consumer:mmap error:" << shm_pt;
		exit( EXIT_FAILURE );
	};
	steady_clock::time_point* post_time
		= reinterpret_cast<steady_clock::time_point*>( shm_pt );
	cout << "consumer:shared memory created." << endl;

	steady_clock::duration		total_delay {};
	int		recv_count = 0;
	sem_t*	note_sem = sem_open( SEM_NAME, O_CREAT, S_IRUSR | S_IWUSR, 0 );
	if( note_sem == SEM_FAILED ) {
		cerr << "consumer:failed on create semaphore!" << endl;
		return EXIT_FAILURE;
	}
	cout << "consumer:semaphore created." << endl;

	cout << "consumer:started..." << endl;
	while( recv_count < TOTAL_NOTES ) {
		if( sem_wait( note_sem ) != 0 ) {
			cerr << "consumer:" << strerror( errno ) << endl;
			return EXIT_FAILURE;
		}

		auto recv_time = steady_clock::now();
		total_delay += recv_time - *post_time;
		++recv_count;
	}
	cout << "consumer:ended." << endl;

	cout << "msgs:" << recv_count
		 << ", rate:" << total_delay.count() / double( recv_count ) << endl;
	sem_close( note_sem );
	sem_unlink( SEM_NAME );
	munmap( shm_pt, sizeof( steady_clock::time_point ) );
	shm_unlink( SM_NAME );
	return EXIT_SUCCESS;
};
