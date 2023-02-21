#include <chrono>

using namespace std::chrono_literals;
using std::chrono::steady_clock;

union Msg_u {
	steady_clock::time_point	send_time {};
	char		as_buffer[ sizeof( steady_clock::time_point ) ];
	uint64_t	as_signal;
};

constexpr uint64_t	END_SIGNAL { 0xFFFFFFFFFFFFFFFF };
constexpr int		TOTAL_NOTES { 100000 };
// constexpr size_t	BUF_ELEMENTS { 1024 };
// constexpr size_t	BUFFER_BYTES { BUF_ELEMENTS * sizeof( Msg_u ) };

constexpr steady_clock::duration SEND_INTERVEL { 1ms };

const char MQ_NAME[] = "/comp_notify";