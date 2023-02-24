#include <chrono>
#include <cstring>
#include <fcntl.h>		//O_RDONLY, S_IRUSR, S_IWUSR
#include <iomanip>
#include <iostream>
#include <semaphore.h>	// sem_init
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
	if( ftruncate( shm_fd, sizeof( steady_clock::time_point ) ) != 0 ) {
		cerr << "producer:ftruncate error:" << shm_fd << endl;
		exit( EXIT_FAILURE );
	};
	auto shm_pt = mmap( NULL, sizeof( steady_clock::time_point ),
						PROT_READ | PROT_WRITE, MAP_SHARED_VALIDATE, shm_fd, 0 );
	if( shm_pt == MAP_FAILED ) {
		std::cerr << "producer:mmap error:" << shm_pt;
		exit( EXIT_FAILURE );
	};
	steady_clock::time_point* post_time
		= reinterpret_cast<steady_clock::time_point*>( shm_pt );
	cout << "producer:shared memory created." << endl;

	sem_t*	note_sem = sem_open( SEM_NAME, O_CREAT, S_IRUSR | S_IWUSR, 0 );
	if( note_sem == SEM_FAILED ) {
		cerr << "producer:failed on create semaphore!" << endl;
		return EXIT_FAILURE;
	}
	cout << "producer:semaphore created." << endl;

	cout << "producer:started..." << endl;
	for( int j = 0; j < TOTAL_NOTES; ++j ) {
		*post_time = steady_clock::now();
		if( sem_post( note_sem ) != 0 ) {
			cerr << "producer:" << strerror( errno ) << endl;
			break;
		}
		this_thread::sleep_for( SEND_INTERVEL );
	}
	cout << "producer:ended." << endl;

	sem_close( note_sem );
	munmap( shm_pt, sizeof( steady_clock::time_point ) );
	return EXIT_SUCCESS;
};
