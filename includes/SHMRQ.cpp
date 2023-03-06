#include <atomic>
#include <cmath>		// abs, ceil, floor, isnan, log, log10, log2, pow, round, sqrt
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
#include <fcntl.h>		// O_RDONLY, S_IRUSR, S_IWUSR
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>	// shm_open, PROT_READ, PROT_WRITE, MAP_PRIVATE, MAP_ANON
#include <unistd.h>		// syscall, ftruncate

#include "SHMRQ.hpp"
#include "common.hpp"

using std::atomic_size_t;
using std::string;

template <typename T>
struct SHMRQ_t<T>::Imp_t {
	struct Node_t {
		alignas( 64 )
		atomic_size_t	head;
		atomic_size_t	tail;
		T				data;
	};

	Imp_t( const string& shm, size_t capa, bool consuming );
	~Imp_t();
	bool grabTail( Node_t*& node, size_t& tail );
	bool enque( const T& src );
	bool enque( T&& src );
	bool deque( T& dest );

	// 对齐到64字节(一个CPU cache line的大小),保证以下变量在同一个cache line里面
	alignas( 64 )
	// 基址
	Node_t* const	_base = nullptr;
	// 容量
	const size_t	_capa = 0;
	// 访问掩码
	const size_t	_mask = 0;
	// raw buffer
	void*			_rawb = nullptr;
	// 共享内存名称
	const string	_shmn;
	// 我是消费者(owner)
	bool			_ownr;

	// 对齐到64字节(一个CPU cache line的大小),保证_tail,_head不在同一个line里面
	alignas( 64 )
	atomic_size_t	_head = {};
	alignas( 64 )
	atomic_size_t	_tail = {};
};

constexpr std::memory_order acq_rel = std::memory_order::acq_rel;
constexpr std::memory_order acquire = std::memory_order::acquire;
constexpr std::memory_order relaxed = std::memory_order::relaxed;
constexpr std::memory_order release = std::memory_order::release;
constexpr std::memory_order seq_cst = std::memory_order::seq_cst;

template <typename T>
SHMRQ_t<T>::Imp_t::Imp_t( const string& shm_, size_t capa_, bool consuming_ )
	: _shmn( shm_ ), _ownr( consuming_ ) {
	// 让容量刚好是2的整数次幂, 为了保证_mask必须是全1
	capa_ = std::pow( 2, std::ceil( std::log2( capa_ ) ) );

	// 最大内存占用不能超过 MAX_MEM_USAGE 字节
	size_t MOST_ELEMENTS = std::log2( MAX_MEM_USAGE / sizeof( Node_t ) );
	MOST_ELEMENTS = std::pow( 2, MOST_ELEMENTS );
	capa_ = std::min( capa_, MOST_ELEMENTS );
	capa_ = std::max( capa_, LEAST_ELEMNTS );
	const_cast<size_t&>( _capa ) = capa_;
	const_cast<size_t&>( _mask ) = capa_ - 1;
	size_t bytes = _capa * sizeof( Node_t );

	// 分配内存
	int shm_fd;
	if( _ownr )
		shm_fd = shm_open( _shmn.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR );
	else
		shm_fd = shm_open( _shmn.c_str(), O_RDWR, S_IRUSR | S_IWUSR );
	if( shm_fd < 0 )
		throw std::runtime_error( _shmn + ":shm_open error:" + strerror( errno ) );
	if( ftruncate( shm_fd, bytes ) != 0 )
		throw std::runtime_error( _shmn + ":ftruncate error:" + strerror( errno ) );
	_rawb = mmap( NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED_VALIDATE, shm_fd, 0 );
	if( _rawb == MAP_FAILED )
		throw std::runtime_error( _shmn + ":mmap error:" + strerror( errno ) );
	if( close( shm_fd ) != 0 )
		throw std::runtime_error( _shmn + ":close(shm_fd) error:" + strerror( errno ) );

	// 对齐到整64字节边界
	if( ( reinterpret_cast<uintptr_t>( _rawb ) & 63 ) != 0 )
		throw std::runtime_error( _shmn + ":地址未从64字节整倍数开始!" );
	const_cast<Node_t*&>( _base ) = reinterpret_cast<Node_t*>( _rawb );

	// 初始化
	Node_t* pNode = _base;
	for( size_t i = 0; i < _capa; ++i, ++pNode ) {
		pNode->head.store( -1, relaxed );
		pNode->tail.store( i, relaxed );
	}
	_head.store( 0, relaxed );
	_tail.store( 0, relaxed );
};

template <typename T>
SHMRQ_t<T>::Imp_t::~Imp_t() {
	for( auto j = _head.load(); j != _tail.load(); ++j )
// 		( &( _base[ j & _mask ].data ) )->~T();
		_base[ j & _mask ].data.~T();

	// 析构器里面不能抛异常
	if( munmap( _rawb, _capa * sizeof( Node_t ) ) != 0 )
		std::cerr << _shmn << ":munmap:" << strerror( errno ) << std::endl;
	if( _ownr && shm_unlink( _shmn.c_str() ) != 0 )
		std::cerr << _shmn << ":shm_unlink:" << strerror( errno ) << std::endl;
};

template <typename T>
bool SHMRQ_t<T>::Imp_t::grabTail( Node_t*& node_, size_t& tail_ ) {
	tail_ = _tail.load( relaxed );
	do {
		node_ = & _base[tail_ & _mask];
		if( node_->tail.load( relaxed ) != tail_ )
			return false;
	} while( ! _tail.compare_exchange_weak( tail_, tail_ + 1, relaxed ) );
	return true;
}

template <typename T>
bool SHMRQ_t<T>::Imp_t::enque( const T& src_ ) {
	Node_t* node;
	size_t tail;
	if( ! grabTail( node, tail ) )
		return false;

	new( & node->data ) T( src_ );
	node->head.store( tail, release );
	return true;
};

template <typename T>
bool SHMRQ_t<T>::Imp_t::enque( T&& src_ ) {
	Node_t* node;
	size_t tail;
	if( ! grabTail( node, tail ) )
		return false;

	new( & node->data ) T( std::move( src_ ) );
	node->head.store( tail, release );
	return true;
};

template <typename T>
bool SHMRQ_t<T>::Imp_t::deque( T& dest_ ) {
	Node_t* node;
	size_t head = _head.load( relaxed );
	do {
		node = & _base[head & _mask];
		if( node->head.load( relaxed ) != head )
			return false;
	} while( ! _head.compare_exchange_weak( head, head + 1, relaxed ) );

	dest_ = std::move( node->data );
	node->data.~T();
	node->tail.store( head + _capa, release );
	return true;
};

template <typename T>
SHMRQ_t<T>::SHMRQ_t( const string& shmn_, size_t capa_, bool consuming_ ) {
	pimp = new Imp_t( shmn_, capa_, consuming_ );
};

template <typename T>
SHMRQ_t<T>::~SHMRQ_t() {
	delete pimp;
};

template <typename T>
size_t SHMRQ_t<T>::capa() const {
	return pimp->_capa;
};

template <typename T>
size_t SHMRQ_t<T>::size() const {
	size_t head = pimp->_head.load( acquire );
	return pimp->_tail.load( relaxed ) - head;
};

template <typename T>
bool SHMRQ_t<T>::enque( const T& src_ ) {
	return pimp->enque( src_ );
};

template <typename T>
bool SHMRQ_t<T>::enque( T&& src_ ) {
	return pimp->enque( src_ );
};

template <typename T>
bool SHMRQ_t<T>::deque( T& dest_ ) {
	return pimp->deque( dest_ );
};

// 将模板显式实例化
template class SHMRQ_t<Msg_u>;

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
