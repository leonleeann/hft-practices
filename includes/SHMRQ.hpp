#pragma once
#include <string>

/* 一个基于共享内存可跨进程使用的无锁环状队列
 * 特点:
 *   1.无锁
 *   2.环状队列,整个生命周期不会追加/释放内存
 *   3.多生产者/多消费者(MPMC)
 *   4.基于共享内存,可多进程使用,也可多线程使用
 */
template <typename T>
class SHMRQ_t {
public:
	// 最大内存占用(单位:字节)
	static constexpr size_t MAX_MEM_USAGE = 0x40000000;  // 1G Bytes

	// 最少容量(单位:元素个数)
	static constexpr size_t LEAST_ELEMNTS = 4;

	// 由consumer来构造,会创建共享内存,反之producer只会访问
	explicit SHMRQ_t( const std::string& shm_name, size_t capacity, bool consuming );
	~SHMRQ_t();

	// 队列容量(单位:元素个数)
	size_t capa() const;

	// 当前占用(单位:元素个数)
	size_t size() const;

	// 复制入队
	bool enque( const T& data );
	// 移动入队
	bool enque( T&& data );

	// 移动出队
	bool deque( T& data );

protected:
	struct Imp_t;
	Imp_t* pimp;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
