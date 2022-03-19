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
#include "aether.h"

namespace ae {

//------------------------------------------------------------------------------
// ae::FreeList class
//------------------------------------------------------------------------------
template< uint32_t N = 0 >
class FreeList
{
public:
	FreeList();
	FreeList( const ae::Tag& tag, uint32_t size );

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
	//! Returns the maximum length of the list (constxpr for static ae::FreeList's).
	_AE_STATIC_SIZE static constexpr uint32_t Size() { return N; }
	//! Returns the maximum length of the list.
	_AE_DYNAMIC_SIZE uint32_t Size(...) const { return m_pool.Length(); }

private:
	struct Entry { Entry* next; };
	uint32_t m_length;
	Entry* m_free;
	ae::Array< Entry, N > m_pool;
};

//------------------------------------------------------------------------------
// ae::ObjectPool class
//------------------------------------------------------------------------------
template< typename T, uint32_t N, bool Paged = false >
class ObjectPool
{
public:
	//! Constructor for static ae::ObjectPool's only.
	ObjectPool();
	//! Constructor for paged ae::ObjectPool's only.
	ObjectPool( const ae::Tag& tag );
	//! All objects allocated with ae::ObjectPool::New() must be destroyed before
	//! the ae::ObjectPool is destroyed.
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

	//! Returns true if the pool has any unallocated objects available.
	bool HasFree() const;
	//! Returns the number of allocated objects.
	uint32_t Length() const;
	//! Returns the total number of objects in the pool.
	constexpr uint32_t Size() const { return N; }
	
	_AE_STATIC_SIZE static constexpr uint32_t Size() { return N; }
	_AE_DYNAMIC_SIZE uint32_t Size(...) const { return N * m_pages.Length(); }

private:
	// @TODO: Disable copy constructor etc or fix list on copy.
	typedef typename std::aligned_storage< sizeof(T), alignof(T) >::type AlignedStorageT;
	struct Page
	{
		ae::ListNode< Page > node = this;
		ae::FreeList< N > freeList;
		AlignedStorageT objects[ N ];
	};
	template < bool Allocate > struct ConditionalPage {
		Page* Get() { return nullptr; }
		const Page* Get() const { return nullptr; }
	};
	template <> struct ConditionalPage< false > {
		Page* Get() { return &page; }
		const Page* Get() const { return &page; }
		Page page;
	};
	ae::Tag m_tag;
	uint32_t m_length = 0;
	ae::List< Page > m_pages;
	ConditionalPage< Paged > m_firstPage;
};

//------------------------------------------------------------------------------
// ae::OpaquePool class
//------------------------------------------------------------------------------
class OpaquePool
{
public:
	OpaquePool( const ae::Tag& tag, uint32_t pageSize, bool paged, uint32_t objectSize, uint32_t objectAlignment );
	//! All objects allocated with ae::OpaquePool::New() must be destroyed before
	//! the ae::OpaquePool is destroyed.
	~OpaquePool();

	//! Returns a pointer to a freshly constructed object T or null if there
	//! are no free objects. Call ae::OpaquePool::Delete() to destroy the object.
	//! ae::OpaquePool::Delete() must be called on every object returned
	//! by ae::OpaquePool::New().
	void* New();
	//! Destructs and releases the object \p obj for future use by ae::OpaquePool::New().
	//! It is safe for the \p obj parameter to be null.
	void Delete( void* obj );
	//! Destructs and releases all objects for future use by ae::OpaquePool::New().
	void DeleteAll();

	//! Returns the first allocated object in the pool or null if the pool is empty.
	const void* GetFirst() const;
	//! Returns the next allocated object after \p obj or null if there are no more objects.
	//! Null will be returned if \p obj is null.
	const void* GetNext( const void* obj ) const;
	//! Returns the first allocated object in the pool or null if the pool is empty.
	void* GetFirst();
	//! Returns the next allocated object after \p obj or null if there are no more objects.
	//! Null will be returned if \p obj is null.
	void* GetNext( void* obj );

	//! Returns true if the pool has any unallocated objects available.
	bool HasFree() const;
	//! Returns the number of allocated objects.
	uint32_t Length() const;
	//! Returns the total number of objects in the pool.
	uint32_t Size() const { return 0; }

private:
	// @TODO: Disable copy constructor etc or fix list on copy.
	struct Page
	{
		Page( const ae::Tag& tag, uint32_t size ) : freeList( tag, size ) {}
		ae::ListNode< Page > node = this;
		ae::FreeList<> freeList;
		void* objects;
	};
	ae::Tag m_tag;
	uint32_t m_pageSize;
	bool m_paged;
	uint32_t m_objectSize;
	uint32_t m_objectAlignment;
	uint32_t m_length;
	ae::List< Page > m_pages;
};

//------------------------------------------------------------------------------
// ae::FreeList member functions
//------------------------------------------------------------------------------
template < uint32_t N >
FreeList< N >::FreeList() :
	m_pool( N, Entry() )
{
	AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static arrays" );
	m_length = 0;
	for ( uint32_t i = 0; i < m_pool.Length() - 1; i++ )
	{
		m_pool[ i ].next = &m_pool[ i + 1 ];
	}
	// Last element points to itself so it can be used as a sentinel.
	m_pool[ m_pool.Length() - 1 ].next = &m_pool[ m_pool.Length() - 1 ];
	m_free = &m_pool[ 0 ];
}

template < uint32_t N >
FreeList< N >::FreeList( const ae::Tag& tag, uint32_t size ) :
	m_pool( tag, size, Entry() )
{
	AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
	m_length = 0;
	for ( uint32_t i = 0; i < m_pool.Length() - 1; i++ )
	{
		m_pool[ i ].next = &m_pool[ i + 1 ];
	}
	// Last element points to itself so it can be used as a sentinel.
	m_pool[ m_pool.Length() - 1 ].next = &m_pool[ m_pool.Length() - 1 ];
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
	return (int32_t)( entry - m_pool.Begin() );
}

template < uint32_t N >
void FreeList< N >::Free( int32_t idx )
{
	if ( idx < 0 ) { return; }
	Entry* entry = &m_pool[ idx ];
#if _AE_DEBUG_
	AE_ASSERT( m_length );
	AE_ASSERT( 0 <= idx && idx < m_pool.Length() );
	AE_ASSERT( !entry->next );
#endif
	
	// List length of 1, last element points to itself.
	entry->next = m_free ? m_free : entry;
	m_free = entry;
	m_length--;

#if _AE_DEBUG_
	if ( !m_length )
	{
		for ( uint32_t i = 0; i < m_pool.Length(); i++ )
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
	memset( m_pool.Begin(), 0, sizeof( *m_pool.Begin() ) * m_pool.Length() );
}

template < uint32_t N >
int32_t FreeList< N >::GetFirst() const
{
	if ( !m_length )
	{
		return -1;
	}
	for ( uint32_t i = 0; i < m_pool.Length(); i++ )
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
	for ( uint32_t i = idx + 1; i < m_pool.Length(); i++ )
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
	AE_ASSERT( idx < m_pool.Length() );
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
template < typename T, uint32_t N, bool Paged >
ObjectPool< T, N, Paged >::ObjectPool()
{
	AE_STATIC_ASSERT_MSG( !Paged, "Paged ae::ObjectPool requires an allocation tag" );
	m_pages.Append( m_firstPage.Get()->node );
}

template < typename T, uint32_t N, bool Paged >
ObjectPool< T, N, Paged >::ObjectPool( const ae::Tag& tag )
	: m_tag( tag )
{
	AE_STATIC_ASSERT_MSG( Paged, "Static ae::ObjectPool does not need an allocation tag" );
	AE_ASSERT( m_tag != ae::Tag() );
}

template < typename T, uint32_t N, bool Paged >
ObjectPool< T, N, Paged >::~ObjectPool()
{
	AE_ASSERT( Length() == 0 );
}

template < typename T, uint32_t N, bool Paged >
T* ObjectPool< T, N, Paged >::New()
{
	// @TODO: FindFn's parameter should be a reference to match ae::Array
	Page* page = m_pages.FindFn( []( const Page* page ) { return page->freeList.HasFree(); } );
	if ( Paged && !page )
	{
		page = ae::New< Page >( m_tag );
		m_pages.Append( page->node );
	}
	if ( page )
	{
		int32_t index = page->freeList.Allocate();
		if ( index >= 0 )
		{
			m_length++;
			return new ( &page->objects[ index ] ) T();
		}
	}
	return nullptr;
}

template < typename T, uint32_t N, bool Paged >
void ObjectPool< T, N, Paged >::Delete( T* obj )
{
	if ( !obj ) { return; }
	if ( (intptr_t)obj % alignof(T) != 0 ) { return; } // @TODO: Should this be an assert?

	int32_t index;
	Page* page = m_pages.GetFirst();
	while ( page )
	{
		index = (int32_t)( obj - (const T*)page->objects );
		if ( 0 <= index && index < N )
		{
			break;
		}
		page = page->node.GetNext();
	}
	if ( !Paged || page )
	{
#if _AE_DEBUG_
		AE_ASSERT( (T*)&page->objects[ index ] == obj );
		AE_ASSERT( page->freeList.IsAllocated( index ) );
#endif
		obj->~T();
#if _AE_DEBUG_
		memset( obj, 0xDD, sizeof(*obj) );
#endif
		page->freeList.Free( index );
		m_length--;

		if ( Paged && page->freeList.Length() == 0 )
		{
			ae::Delete( page );
		}
	}
}

template < typename T, uint32_t N, bool Paged >
void ObjectPool< T, N, Paged >::DeleteAll()
{
	auto deleteAllFn = []( Page* page )
	{
		for ( uint32_t i = 0; i < N; i++ )
		{
			if ( page->freeList.IsAllocated( i ) )
			{
				( (T*)&page->objects[ i ] )->~T();
			}
		}
		page->freeList.FreeAll();
	};
	if ( Paged )
	{
		Page* page = m_pages.GetLast();
		while ( page )
		{
			Page* prev = page->node.GetPrev();
			deleteAllFn( page );
			ae::Delete( page );
			page = prev;
		}
	}
	else
	{
		deleteAllFn( m_firstPage.Get() );
	}
	m_length = 0;
}

template < typename T, uint32_t N, bool Paged >
const T* ObjectPool< T, N, Paged >::GetFirst() const
{
	if ( Paged )
	{
		const Page* page = m_pages.GetFirst();
		if ( page )
		{
			AE_ASSERT( page->freeList.Length() );
			return page->freeList.Length() ? (const T*)&page->objects[ page->freeList.GetFirst() ] : nullptr;
		}
	}
	else if ( !Paged && m_length )
	{
		int32_t index = m_firstPage.Get()->freeList.GetFirst();
		AE_ASSERT( index >= 0 );
		return (const T*)&m_firstPage.Get()->objects[ index ];
	}
	AE_ASSERT( m_length == 0 );
	return nullptr;
}

template < typename T, uint32_t N, bool Paged >
const T* ObjectPool< T, N, Paged >::GetNext( const T* obj ) const
{
	if ( !obj ) { return nullptr; }
	const Page* page = m_pages.GetFirst();
	while ( page )
	{
		AE_ASSERT( !Paged || page->freeList.Length() );
		int32_t index = (int32_t)( obj - (const T*)page->objects );
		bool found = ( 0 <= index && index < N );
		if ( found )
		{
			AE_ASSERT( (const T*)&page->objects[ index ] == obj );
			AE_ASSERT( page->freeList.IsAllocated( index ) );
			int32_t next = page->freeList.GetNext( index );
			if ( next >= 0 )
			{
				return (const T*)&page->objects[ next ];
			}
		}
		page = page->node.GetNext();
		if ( found && page )
		{
			// Given object is last element of previous page
			int32_t next = page->freeList.GetFirst();
			AE_ASSERT( 0 <= next && next < N );
			return (const T*)&page->objects[ next ];
		}
	}
	return nullptr;
}

template < typename T, uint32_t N, bool Paged >
T* ObjectPool< T, N, Paged >::GetFirst()
{
	return const_cast< T* >( const_cast< const ObjectPool< T, N, Paged >* >( this )->GetFirst() );
}

template < typename T, uint32_t N, bool Paged >
T* ObjectPool< T, N, Paged >::GetNext( T* obj )
{
	return const_cast< T* >( const_cast< const ObjectPool< T, N, Paged >* >( this )->GetNext( obj ) );
}

template < typename T, uint32_t N, bool Paged >
bool ObjectPool< T, N, Paged >::HasFree() const
{
	return Paged || m_firstPage.Get()->freeList.HasFree();
}

template < typename T, uint32_t N, bool Paged >
uint32_t ObjectPool< T, N, Paged >::Length() const
{
	return m_length;
}

} // ae

#endif
