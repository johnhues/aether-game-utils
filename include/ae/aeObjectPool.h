//------------------------------------------------------------------------------
// aeObjectPool.h
//------------------------------------------------------------------------------
// Copyright (c) 2020 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
#ifndef AEOBJECTPOOL_H
#define AEOBJECTPOOL_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeList.h"

namespace ae {

//------------------------------------------------------------------------------
// ae::FreeList class
//------------------------------------------------------------------------------
template< uint32_t N >
class FreeList
{
public:
	FreeList();

	//! Returns (0 <= index < N) on success, and negative on failure.
	int32_t Allocate();
	//! Releases \p idx for future calls to ae::FreeList::Allocate(). \p idx must
	//! be an allocated index or negative (a result of ae::FreeList::Allocate() failure).
	void Free( int32_t idx );
	//! Frees all allocated indices.
	void FreeAll();

	//! Returns the index of the first allocated object. Returns a negative value
	//! if there are no allocated objects.
	int32_t GetFirst() const;
	//! Returns the index of the next allocated object after \p idx. Returns a
	//! negative value if there are no more allocated objects. \p idx must
	//! be an allocated index or negative. A negative value will be returned
	//! if \p idx is negative.
	int32_t GetNext( int32_t idx ) const;

	//! Returns true if the given \p idx is currently allocated. \p idx must be
	//! negative or less than N.
	bool IsAllocated( int32_t idx ) const;
	//! Returns true if the next Allocate() will succeed.
	bool HasFree() const;
	//! Returns the number of allocated elements.
	uint32_t Length() const;
	//! Returns the maximum length of the list.
	constexpr uint32_t Size() const { return N; }

private:
	struct Entry { Entry* next; };
	uint32_t m_length;
	Entry* m_free;
	Entry m_pool[ N ];
};

//------------------------------------------------------------------------------
// ae::ObjectPool class
//------------------------------------------------------------------------------
template< typename T, uint32_t N >
class ObjectPool
{  
public:
	~ObjectPool();

	//! Returns a pointer to a freshly constructed object T or null if there
	//! are no free objects. Call ae::ObjectPool::Delete() to destroy the object.
	//! ae::ObjectPool::Delete() must be called on every object returned
	//! by ae::ObjectPool::New().
	T* New();
	//! Destructs and releases the object \p obj for future use by ae::ObjectPool::New().
	//! It is safe for the \p obj parameter to be null.
	void Delete( T* obj );
	//! Destructs and releases all objects for future use by ae::ObjectPool::New().
	void DeleteAll();

	//! Returns the first allocated object in the pool or null if the pool is empty.
	const T* GetFirst() const;
	//! Returns the next allocated object after \p obj or null if there are no more objects.
	//! Null will be returned if \p obj is null.
	const T* GetNext( const T* obj ) const;
	//! Returns the first allocated object in the pool or null if the pool is empty.
	T* GetFirst();
	//! Returns the next allocated object after \p obj or null if there are no more objects.
	//! Null will be returned if \p obj is null.
	T* GetNext( T* obj );

	//! Returns true if the object \p obj is currently allocated. False will be returned
	//! if \p obj is null.
	bool IsAllocated( const T* obj ) const;
	//! Returns true if the pool has any unallocated objects available.
	bool HasFree() const;
	//! Returns the number of allocated objects.
	uint32_t Length() const;
	//! Returns the total number of objects in the pool.
	constexpr uint32_t Size() const { return N; }

private:
	typedef typename std::aligned_storage< sizeof(T), alignof(T) >::type AlignedStorageT;
	ae::FreeList< N > m_freeList;
	AlignedStorageT m_objects[ N ];
public:
	// @NOTE: This is a hack to allow PagedObjectPools to search for the correct ObjectPool
	bool _HACK_IsOnPage( const T* obj ) const;
};

//------------------------------------------------------------------------------
// ae::PagedObjectPool class
//------------------------------------------------------------------------------
template < typename T, uint32_t N >
class PagedObjectPool
{
public:
	PagedObjectPool( Tag tag );
	~PagedObjectPool();

	T* New();
	bool Delete( T* obj );
	void DeleteAll();

	const T* GetFirst() const;
	const T* GetNext( const T* obj ) const;
	T* GetFirst();
	T* GetNext( T* obj );

	bool IsAllocated( const T* obj ) const;
	uint32_t Length() const;
	constexpr uint32_t PageSize() const { return N; }
	
private:
	struct Page
	{
		Page() : node( this ) {}
		ae::ListNode< Page > node;
		ObjectPool< T, N > pool;
	};
	
	uint32_t m_length;
	ae::List< Page > m_pages;
	Tag m_tag;
};

//------------------------------------------------------------------------------
// ae::FreeList member functions
//------------------------------------------------------------------------------
template < uint32_t N >
FreeList< N >::FreeList()
{
	m_length = 0;
	for ( uint32_t i = 0; i < N - 1; i++ )
	{
		m_pool[ i ].next = &m_pool[ i + 1 ];
	}
	// Last element points to itself so it can be used as a sentinel.
	m_pool[ N - 1 ].next = &m_pool[ N - 1 ];
	m_free = &m_pool[ 0 ];
}

template < uint32_t N >
int32_t FreeList< N >::Allocate()
{
	if ( !m_free ) { return -1; }
	Entry* entry = m_free;
	// Advance the free pointer until the sentinel is reached.
	m_free = ( m_free->next == m_free ) ? nullptr : m_free->next;
	entry->next = nullptr;
	m_length++;
	return (int32_t)( entry - m_pool );
}

template < uint32_t N >
void FreeList< N >::Free( int32_t idx )
{
	if ( idx < 0 ) { return; }
	Entry* entry = &m_pool[ idx ];
#if _AE_DEBUG_
	AE_ASSERT( m_length );
	AE_ASSERT( 0 <= idx && idx < N );
	AE_ASSERT( !entry->next );
#endif
	
	// List length of 1, last element points to itself.
	entry->next = m_free ? m_free : entry;
	m_free = entry;
	m_length--;

#if _AE_DEBUG_
	if ( !m_length )
	{
		for ( uint32_t i = 0; i < N; i++ )
		{
			AE_ASSERT( m_pool[ i ].next );
		}
	}
#endif
}

template < uint32_t N >
void FreeList< N >::FreeAll()
{
	m_length = 0;
	m_free = nullptr;
	memset( m_pool, 0, sizeof( m_pool ) );
}

template < uint32_t N >
int32_t FreeList< N >::GetFirst() const
{
	if ( !m_length )
	{
		return -1;
	}
	for ( uint32_t i = 0; i < N; i++ )
	{
		if ( !m_pool[ i ].next )
		{
			return (int32_t)i;
		}
	}
#if _AE_DEBUG_
	AE_FAIL();
#endif
	return -1;
}

template < uint32_t N >
int32_t FreeList< N >::GetNext( int32_t idx ) const
{
	if ( idx < 0 )
	{
		return -1;
	}
	for ( uint32_t i = idx + 1; i < N; i++ )
	{
		if ( !m_pool[ i ].next )
		{
			return (int32_t)i;
		}
	}
	return -1;
}

template < uint32_t N >
bool FreeList< N >::IsAllocated( int32_t idx ) const
{
	if ( idx < 0 )
	{
		return false;
	}
#if _AE_DEBUG_
	AE_ASSERT( idx < N );
#endif
	return !m_pool[ idx ].next;
}

template < uint32_t N >
bool FreeList< N >::HasFree() const
{
	return m_free;
}

template < uint32_t N >
uint32_t FreeList< N >::Length() const
{
	return m_length;
}

//------------------------------------------------------------------------------
// ae::ObjectPool member functions
//------------------------------------------------------------------------------
template < typename T, uint32_t N >
ObjectPool< T, N >::~ObjectPool()
{
	AE_ASSERT( Length() == 0 );
}

template < typename T, uint32_t N >
T* ObjectPool< T, N >::New()
{
	int32_t index = m_freeList.Allocate();
	if ( index >= 0 )
	{
		return new ( &m_objects[ index ] ) T();
	}
	return nullptr;
}

template < typename T, uint32_t N >
void ObjectPool< T, N >::Delete( T* obj )
{
	if ( !obj ) { return; }
	int32_t index = (int32_t)( obj - (T*)m_objects );
#if _AE_DEBUG_
	AE_ASSERT( 0 <= index && index < N );
	AE_ASSERT( (T*)&m_objects[ index ] == obj );
	AE_ASSERT( m_freeList.IsAllocated( index ) );
#endif
	obj->~T();
#if _AE_DEBUG_
	memset( obj, 0xDD, sizeof(*obj) );
#endif
	m_freeList.Free( index );
}

template < typename T, uint32_t N >
void ObjectPool< T, N >::DeleteAll()
{
	for ( uint32_t i = 0; i < N; i++ )
	{
		if ( m_freeList.IsAllocated( i ) )
		{
			( (T*)&m_objects[ i ] )->~T();
		}
	}
	m_freeList.FreeAll();
}

template < typename T, uint32_t N >
const T* ObjectPool< T, N >::GetFirst() const
{
	return m_freeList.Length() ? (const T*)&m_objects[ m_freeList.GetFirst() ] : nullptr;
}

template < typename T, uint32_t N >
const T* ObjectPool< T, N >::GetNext( const T* obj ) const
{
	if ( !obj ) { return nullptr; }
	int32_t index = (int32_t)( obj - (const T*)m_objects );
#if _AE_DEBUG_
	AE_ASSERT( 0 <= index && index < N );
	AE_ASSERT( (const T*)&m_objects[ index ] == obj );
	AE_ASSERT( m_freeList.IsAllocated( index ) );
#endif
	int32_t nextIndex = m_freeList.GetNext( index );
	return ( nextIndex >= 0 ) ? (T*)&m_objects[ nextIndex ] : nullptr;
}

template < typename T, uint32_t N >
T* ObjectPool< T, N >::GetFirst()
{
	return const_cast< T* >( const_cast< const ObjectPool< T, N >* >( this )->GetFirst() );
}

template < typename T, uint32_t N >
T* ObjectPool< T, N >::GetNext( T* obj )
{
	return const_cast< T* >( const_cast< const ObjectPool< T, N >* >( this )->GetNext( obj ) );
}

template < typename T, uint32_t N >
bool ObjectPool< T, N >::IsAllocated( const T* obj ) const
{
	if ( !obj ) { return false; }
	int32_t index = (int32_t)( obj - (const T*)m_objects );
#if _AE_DEBUG_
	AE_ASSERT( 0 <= index && index < N );
	AE_ASSERT( (const T*)&m_objects[ index ] == obj );
#endif
	return m_freeList.IsAllocated( index );
}

template < typename T, uint32_t N >
bool ObjectPool< T, N >::HasFree() const
{
	return m_freeList.HasFree();
}

template < typename T, uint32_t N >
uint32_t ObjectPool< T, N >::Length() const
{
	return m_freeList.Length();
}

template < typename T, uint32_t N >
bool ObjectPool< T, N >::_HACK_IsOnPage( const T* obj ) const
{
	if ( !obj ) { return false; }
	if ( (intptr_t)obj % alignof(T) != 0 ) { return false; }
	int32_t index = (int32_t)( obj - (const T*)m_objects );
	return 0 <= index && index < N;
}

//------------------------------------------------------------------------------
// ae::PagedObjectPool member functions
//------------------------------------------------------------------------------
template < typename T, uint32_t N >
PagedObjectPool< T, N >::PagedObjectPool( Tag tag ) :
	m_tag( tag )
{
	m_length = 0;
}

template < typename T, uint32_t N >
PagedObjectPool< T, N >::~PagedObjectPool()
{
	AE_ASSERT( !m_length );
}

template < typename T, uint32_t N >
T* PagedObjectPool< T, N >::New()
{
	m_length++;
	
	Page* page = m_pages.GetFirst();
	while ( page )
	{
		if ( T* obj = page->pool.New() )
		{
			return obj;
		}
		page = page->node.GetNext();
	}
	
	page = ae::New< Page >( m_tag );
	m_pages.Append( page->node );
	T* obj = page->pool.New();
	AE_ASSERT( obj );
	return obj;
}

template < typename T, uint32_t N >
bool PagedObjectPool< T, N >::Delete( T* obj )
{
	Page* page = m_pages.GetFirst();
	while ( page )
	{
		if ( page->pool._HACK_IsOnPage( obj ) )
		{
			page->pool.Delete( obj );
			if ( page->pool.Length() == 0 )
			{
				ae::Delete( page );
			}
			m_length--;
			return true;
		}
		page = page->node.GetNext();
	}
	return false;
}

template < typename T, uint32_t N >
void PagedObjectPool< T, N >::DeleteAll()
{
	Page* page = m_pages.GetLast();
	while ( page )
	{
		Page* prev = page->node.GetPrev();
		page->pool.DeleteAll();
		ae::Delete( page );
		page = prev;
	}
	m_length = 0;
}

template < typename T, uint32_t N >
const T* PagedObjectPool< T, N >::GetFirst() const
{
	const Page* page = m_pages.GetFirst();
	if ( page )
	{
		AE_ASSERT( page->pool.Length() );
		return page->pool.GetFirst();
	}
	AE_ASSERT( m_length == 0 );
	return nullptr;
}

template < typename T, uint32_t N >
const T* PagedObjectPool< T, N >::GetNext( const T* obj ) const
{
	if ( !obj )
	{
		return nullptr;
	}
	
	const Page* page = m_pages.GetFirst();
	while ( page )
	{
		AE_ASSERT( page->pool.Length() );
		bool inPrev = page->pool._HACK_IsOnPage( obj );
		if ( inPrev )
		{
			if ( const T* o = page->pool.GetNext( obj ) )
			{
				return o;
			}
		}
		page = page->node.GetNext();
		if ( inPrev && page )
		{
			// Given object is last element of previous page
			return page->pool.GetFirst();
		}
	}
	
	return nullptr;
}

template < typename T, uint32_t N >
T* PagedObjectPool< T, N >::GetFirst()
{
	return const_cast< T* >( const_cast< const PagedObjectPool< T, N >* >( this )->GetFirst() );
}

template < typename T, uint32_t N >
T* PagedObjectPool< T, N >::GetNext( T* obj )
{
	return const_cast< T* >( const_cast< const PagedObjectPool< T, N >* >( this )->GetNext( obj ) );
}

template < typename T, uint32_t N >
bool PagedObjectPool< T, N >::IsAllocated( const T* obj ) const
{
	if ( obj )
	{
		const Page* page = m_pages.GetFirst();
		while ( page )
		{
			if ( page->pool._HACK_IsOnPage( obj ) && page->pool.IsAllocated( obj ) )
			{
				return true;
			}
			page = page->node.GetNext();
		}
	}
	return false;
}

template < typename T, uint32_t N >
uint32_t PagedObjectPool< T, N >::Length() const
{
#if _AE_DEBUG_
	uint32_t length = 0;
	for ( const Page* page = m_pages.GetFirst(); page; page = page->node.GetNext() )
	{
		length += page->pool.Length();
	}
	AE_ASSERT( m_length == length );
#endif
	return m_length;
}

} // ae

#endif
