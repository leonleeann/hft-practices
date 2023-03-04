cmake_minimum_required(VERSION 3.0)

set( CMAKE_INCLUDE_CURRENT_DIR ON )
include_directories( "." )

######## POSIX消息队列 ##############################################################
add_executable( mq-sp mq-sp.cpp )
target_link_libraries( mq-sp rt Threads::Threads )
install(TARGETS mq-sp RUNTIME DESTINATION . )

######## CraflinRQ #################################################################
add_executable( rq-sp rq-sp.cpp )
target_link_libraries( rq-sp Threads::Threads )
install(TARGETS rq-sp RUNTIME DESTINATION . )

######## forward_list ##############################################################
add_executable( lk-sp lk-sp.cpp )
target_link_libraries( lk-sp Threads::Threads )
install(TARGETS lk-sp RUNTIME DESTINATION . )
