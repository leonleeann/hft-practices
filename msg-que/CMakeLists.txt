cmake_minimum_required(VERSION 3.0)

set( CMAKE_INCLUDE_CURRENT_DIR ON )
include_directories( "." )
add_library( objSHMRQ OBJECT ../includes/SHMRQ.cpp )

######## POSIX消息队列 ##############################################################
add_executable( mq-sp mq-sp.cpp )
target_link_libraries( mq-sp rt Threads::Threads )
install(TARGETS mq-sp RUNTIME DESTINATION . )

######## LeonRQ ####################################################################
add_executable( leonq-sp leonq-sp.cpp )
target_link_libraries( leonq-sp Threads::Threads )
install(TARGETS leonq-sp RUNTIME DESTINATION . )

######## SHMRQ Single Process ######################################################
add_executable( shmrq-sp shmrq-sp.cpp )
target_link_libraries( shmrq-sp objSHMRQ Threads::Threads rt )
install(TARGETS shmrq-sp RUNTIME DESTINATION . )

######## SHMRQ Multi-Process #######################################################
add_executable( shmrq-consumer shmrq-consumer.cpp )
target_link_libraries( shmrq-consumer objSHMRQ Threads::Threads rt )
install(TARGETS shmrq-consumer RUNTIME DESTINATION . )
add_executable( shmrq-producer shmrq-producer.cpp )
target_link_libraries( shmrq-producer objSHMRQ Threads::Threads rt )
install(TARGETS shmrq-producer RUNTIME DESTINATION . )

######## forward_list ##############################################################
add_executable( lock-list-sp lock-list-sp.cpp )
target_link_libraries( lock-list-sp Threads::Threads )
install(TARGETS lock-list-sp RUNTIME DESTINATION . )
