// #include <stdint.h>

static inline uint64_t rdtscp( void ) {
	uint32_t eax, edx;

	asm volatile( "rdtscp"
						  : "=a"( eax ), "=d"( edx )
						  :
						  : "%ecx", "memory" );

	return ( ( uint64_t ) edx << 32 ) | eax;
};
