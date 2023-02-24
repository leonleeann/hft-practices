#include <chrono>
#include <string>

using namespace std::chrono_literals;
using std::chrono::steady_clock;

union Msg_u {
	steady_clock::time_point	post_time {};
	char		as_buffer[ sizeof( steady_clock::time_point ) ];
	uint64_t	as_signal;
};

constexpr uint64_t	END_SIGNAL { 0xFFFFFFFFFFFFFFFF };
constexpr int		TOTAL_NOTES { 100000 };

constexpr uint16_t	RPC_PORT = 8888;

constexpr steady_clock::duration SEND_INTERVEL { 1ms };

constexpr char		MQ_NAME[] = "/hft_practices";
constexpr char		SM_NAME[] = "/post_time";
constexpr char		SEM_NAME[] = "/notify_sem";
const std::string	RPC_NAME = "notify_rpc";
