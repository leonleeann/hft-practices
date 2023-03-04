#pragma once
#include <cstring>
#include <iostream>
#include <mqueue.h>
#include <mutex>
#include <string>

constexpr char		MQ_NAME[] = "/hft_practices";
constexpr size_t	MQ_MAX_SIZE = 4096;

std::mutex			s_output_mutex;

static inline void showMqStatus( mqd_t mq, const std::string& runner,
								 const std::string& comment ) {
	std::unique_lock<std::mutex> ulk( s_output_mutex );
	mq_attr attr {};
	if( mq_getattr( mq, &attr ) < 0 ) {
		std::cerr << runner << ':' << comment << ":获取属性也失败("
				  << strerror( errno ) << ")!" << std::endl;
		return;
	}
	std::cerr << runner << ':' << comment
			  << "\n.mq_flags  :" << attr.mq_flags
			  << "\n.mq_maxmsg :" << attr.mq_maxmsg
			  << "\n.mq_msgsize:" << attr.mq_msgsize
			  << "\n.mq_curmsgs:" << attr.mq_curmsgs
			  << std::endl;
	if( attr.mq_curmsgs >= attr.mq_maxmsg - 1 )
		std::cerr << runner << ":可能队列已满!" << std::endl;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
