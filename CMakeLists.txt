cmake_minimum_required( VERSION 3.16.0 )

set( PROJECT_VERSION_MAJOR 1 )   # 主版本
set( PROJECT_VERSION_MINOR 2 )   # 次版本
set( PROJECT_VERSION_PATCH 0 )   # 修订号(Bug fix)

# 版本全串
set( PROJECT_VERSION
   ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# 版本说明
set( PROJECT_VERSION_NOTES "换用rdtscp取时戳试试" )

# 项目描述
set( PROJECT_DESCRIPTION "HFT中各种对比试验" )

project( comp-notifys
   VERSION ${PROJECT_VERSION}
   DESCRIPTION ${PROJECT_DESCRIPTION}
   LANGUAGES CXX
)

################################################################################

include( CheckCXXCompilerFlag )
check_cxx_compiler_flag( -Wall HAVE_WALL )
if ( HAVE_WALL )
	# gcc 警告全开
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
endif()

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_POSITION_INDEPENDENT_CODE ON )
include( GNUInstallDirs )

set( THREADS_PREFER_PTHREAD_FLAG ON )
find_package( Threads REQUIRED )
find_package( rpclib REQUIRED )

set( CMAKE_INCLUDE_CURRENT_DIR ON )
include_directories( "includes" )

################################################################################

add_subdirectory( "notify-sp" )
add_subdirectory( "notify-mp" )
