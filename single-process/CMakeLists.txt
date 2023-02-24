cmake_minimum_required(VERSION 3.0)

set( CMAKE_INCLUDE_CURRENT_DIR ON )
include_directories( "." )

# 原子变量盲目等待
add_executable( bchk-same-proc bchk-same-proc.cpp )
target_link_libraries( bchk-same-proc Threads::Threads )
install(TARGETS bchk-same-proc RUNTIME DESTINATION . )

#[[ 原子变量wait/notify
add_executable( wait-same-proc wait-same-proc.cpp )
target_link_libraries( wait-same-proc Threads::Threads )
install(TARGETS wait-same-proc RUNTIME DESTINATION . )
]]

# 同进程内 STL条件变量
add_executable( cvex-same-proc cvex-same-proc.cpp )
target_link_libraries( cvex-same-proc Threads::Threads )
install(TARGETS cvex-same-proc RUNTIME DESTINATION . )

# 同进程内 Linux信号量
add_executable( sem-same-proc sem-same-proc.cpp )
target_link_libraries( sem-same-proc rt Threads::Threads )
install(TARGETS sem-same-proc RUNTIME DESTINATION . )

# 同进程内 POSIX消息队列
add_executable( mq-same-proc mq-same-proc.cpp )
target_link_libraries( mq-same-proc rt Threads::Threads )
install(TARGETS mq-same-proc RUNTIME DESTINATION . )

# 同进程内 RPC
add_executable( rpc-same-proc rpc-same-proc.cpp )
target_link_libraries( rpc-same-proc rt Threads::Threads ${RPCLIB_LIBS} )
install(TARGETS rpc-same-proc RUNTIME DESTINATION . )
