cmake_minimum_required(VERSION 3.0)

set( CMAKE_INCLUDE_CURRENT_DIR ON )
include_directories( "." )

######## bchk ######################################################################
add_executable( bchk-consumer bchk-consumer.cpp )
target_link_libraries( bchk-consumer rt )
install(TARGETS bchk-consumer RUNTIME DESTINATION . )

add_executable( bchk-producer bchk-producer.cpp )
target_link_libraries( bchk-producer rt )
install(TARGETS bchk-producer RUNTIME DESTINATION . )

######## POSIX消息队列 ##############################################################
add_executable( mq-consumer mq-consumer.cpp )
target_link_libraries( mq-consumer rt )
install(TARGETS mq-consumer RUNTIME DESTINATION . )

add_executable( mq-producer mq-producer.cpp )
target_link_libraries( mq-producer rt )
install(TARGETS mq-producer RUNTIME DESTINATION . )

######## rpc #######################################################################
add_executable( rpc-consumer rpc-consumer.cpp )
target_link_libraries( rpc-consumer rt Threads::Threads ${RPCLIB_LIBS} )
install(TARGETS rpc-consumer RUNTIME DESTINATION . )

add_executable( rpc-producer rpc-producer.cpp )
target_link_libraries( rpc-producer rt Threads::Threads ${RPCLIB_LIBS} )
install(TARGETS rpc-producer RUNTIME DESTINATION . )

######## Linux有名信号量 #############################################################
add_executable( sem-consumer sem-consumer.cpp )
target_link_libraries( sem-consumer rt Threads::Threads )
install(TARGETS sem-consumer RUNTIME DESTINATION . )

add_executable( sem-producer sem-producer.cpp )
target_link_libraries( sem-producer rt Threads::Threads )
install(TARGETS sem-producer RUNTIME DESTINATION . )

#[[####### atomic_wait #############################################################
add_executable( wait-consumer wait-consumer.cpp )
target_link_libraries( wait-consumer rt )
install(TARGETS wait-consumer RUNTIME DESTINATION . )

add_executable( wait-producer wait-producer.cpp )
target_link_libraries( wait-producer rt )
install(TARGETS wait-producer RUNTIME DESTINATION . )
]]
