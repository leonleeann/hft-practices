#pragma once
#include <atomic>
#include <cmath>
#include <cstddef>

inline constexpr std::memory_order mo_acq_rel = std::memory_order::acq_rel;
inline constexpr std::memory_order mo_acquire = std::memory_order::acquire;
inline constexpr std::memory_order mo_relaxed = std::memory_order::relaxed;
inline constexpr std::memory_order mo_release = std::memory_order::release;
inline constexpr std::memory_order mo_seq_cst = std::memory_order::seq_cst;

using std::atomic_size_t;

/* 一个多线程无需用锁的环状队列
 * 特点: 允许"多生产者/多消费者"并发出入队(MPMC)
 */
template <typename T>
class LeonRQ_t {
public:
	struct Node_t {
		alignas( 64 )T data;
		atomic_size_t  tail;
		atomic_size_t  head;
	};

	static constexpr size_t MAX_MEM_USAGE = 0x40000000;  //1GB
	static constexpr size_t LEAST_ELEMNTS = 4;

	explicit LeonRQ_t( size_t capacity );
	~LeonRQ_t();

	size_t capacity() const noexcept;
	size_t size() const;
	bool enque( const T& data );
	bool enque( T&& data );
	bool deque( T& data );

protected:
	// 对齐到64字节(一个CPU cache line的大小),保证以下变量在同一个cache line里面
	alignas( 64 )Node_t* const	_base = nullptr;
	const size_t				_capa = 0;
	const size_t				_mask = 0;
	char*						_rawb = nullptr;

	// 对齐到64字节(一个CPU cache line的大小),保证_tail,_head不在同一个line里面
	alignas( 64 )atomic_size_t	_tail = {};
	alignas( 64 )atomic_size_t	_head = {};
};

template <typename T>
inline LeonRQ_t<T>::LeonRQ_t( size_t capacity ) {
	// 让容量刚好是2的整数次幂, _mask必须是全1
	capacity = std::pow( 2, std::ceil( std::log2( capacity ) ) );

	// 大小限制
	size_t MOST_ELEMENTS = std::log2( MAX_MEM_USAGE / sizeof( Node_t ) );
	MOST_ELEMENTS = std::pow( 2, MOST_ELEMENTS );
	capacity = std::min( capacity, MOST_ELEMENTS );
	capacity = std::max( capacity, LEAST_ELEMNTS );
	const_cast<size_t&>( _capa ) = capacity;
	const_cast<size_t&>( _mask ) = capacity - 1;

	// 分配内存
	_rawb = new char[ sizeof( Node_t ) * capacity + 64 ];

	// 对齐到整64字节边界
	uintptr_t aligned = 63, raw_buf = reinterpret_cast<uintptr_t>( _rawb );
	aligned = ( raw_buf & ~aligned ) + 64;

	const_cast<Node_t*&>( _base ) = reinterpret_cast<Node_t*>( aligned );

	Node_t* pNode = _base;
	for( size_t i = 0; i < _capa; ++i, ++pNode ) {
		pNode->tail.store( i, mo_relaxed );
		pNode->head.store( -1, mo_relaxed );
	}

	_tail.store( 0, mo_relaxed );
	_head.store( 0, mo_relaxed );
};

template <typename T>
inline LeonRQ_t<T>::~LeonRQ_t() {
	for( auto j = _head.load(); j != _tail.load(); ++j )
		( &( _base[ j & _mask ].data ) )->~T();

	delete [] _rawb;
};

template <typename T>
inline size_t LeonRQ_t<T>::capacity() const noexcept {
	return _capa;
};

template <typename T>
inline size_t LeonRQ_t<T>::size() const {
	size_t head = _head.load( mo_acquire );
	return _tail.load( mo_relaxed ) - head;
};

template <typename T>
inline bool LeonRQ_t<T>::enque( const T& src ) {
	Node_t* node;
	size_t tail = _tail.load( mo_relaxed );
	for( ;; ) {
		node = &_base[tail & _mask];
		if( node->tail.load( mo_relaxed ) != tail )
			return false;

		if( _tail.compare_exchange_weak(
					tail, tail + 1, mo_relaxed ) )
			break;
	}

	new( &node->data ) T( src );
	node->head.store( tail, mo_release );
	return true;
};

template <typename T>
inline bool LeonRQ_t<T>::enque( T&& src ) {
	Node_t* node;
	size_t tail = _tail.load( mo_relaxed );
	for( ;; ) {
		node = &_base[tail & _mask];
		if( node->tail.load( mo_relaxed ) != tail )
			return false;

		if( _tail.compare_exchange_weak(
					tail, tail + 1, mo_relaxed ) )
			break;
	}

	new( &node->data ) T( std::move( src ) );
	node->head.store( tail, mo_release );
	return true;
};

template <typename T>
inline bool LeonRQ_t<T>::deque( T& dest ) {
	Node_t* node;
	size_t head = _head.load( mo_relaxed );
	for( ;; ) {
		node = &_base[head & _mask];
		if( node->head.load( mo_relaxed ) != head )
			return false;

		if( _head.compare_exchange_weak(
					head, head + 1, mo_relaxed ) )
			break;
	}

	dest = std::move( node->data );
	( &node->data )->~T();
	node->tail.store( head + _capa, mo_release );
	return true;
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
