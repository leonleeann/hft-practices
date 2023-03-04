#pragma once
#include <chrono>
#include <string>

using namespace std::chrono;
using namespace std::chrono_literals;

union Msg_u {
	uint64_t	as_tstamp;
	char		as_buffer[ sizeof( uint64_t ) ];
};

constexpr steady_clock::duration SEND_INTERVEL { 1ms };

constexpr uint64_t	END_SIGNAL { 0xFFFFFFFFFFFFFFFF };
constexpr int		TOTAL_NOTES { 100000 };
constexpr int		THREADS { 8 };

constexpr uint16_t	RPC_PORT = 8888;

constexpr char		SHM_NAME[] = "/post_time";
constexpr char		SEM_NAME[] = "/notify_sem";
const std::string	RPC_NAME = "notify_rpc";

static inline uint64_t rdtscp( void ) {
	uint32_t eax, edx;
	asm volatile( "rdtscp"
				  : "=a"( eax ), "=d"( edx )
				  :
				  : "%ecx", "memory" );

	return ( ( uint64_t ) edx << 32 ) | eax;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
