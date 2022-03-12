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
	~FreeList();

	//! Negative on failure. 0 <= index < N on success.
	int32_t Allocate();
	//! Releases \p idx for future calls to ae::FreeList::Allocate().
	bool Free( int32_t idx );
	//! Frees all allocated indices. Returns the number of indices freed.
	uint32_t FreeAll();

	//! Returns the index of the first allocated object. Returns a negative value
	//! if there are no allocated objects.
	int32_t GetFirst() const;
	//! Returns the index of the next allocated object after \p idx. Returns a
	//! negative value if there are no more allocated objects.
	int32_t GetNext( int32_t idx ) const;

	//! Returns true is the given \p idx is currently allocated.
	bool IsAllocated( int32_t idx ) const;
	//! Returns true if the next Allocate() will succeed.
	bool HasFree() const;
	//! Returns the number of allocated elements.
	uint32_t Length() const;

private:
	struct Entry { Entry* next; bool allocated; };
	uint32_t m_length;
	Entry* m_open;
	Entry m_pool[ N ];
};

//------------------------------------------------------------------------------
// ae::ObjectPool class
//------------------------------------------------------------------------------
template< typename T, uint32_t N >
class ObjectPool
{  
public:
	ObjectPool();
	~ObjectPool();

	T* New();
	bool Delete( T* p );

	const T* GetFirst() const;
	const T* GetNext( const T* p ) const;
	T* GetFirst();
	T* GetNext( T* p );
	bool Has( const T* p ) const;

	bool HasFree() const;
	uint32_t Length() const;

private:
	typedef typename std::aligned_storage< sizeof(T), alignof(T) >::type AlignedStorageT;
	ae::FreeList< N > m_freeList;
	AlignedStorageT m_objects[ N ];
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

	T* Allocate();
	bool Free( T* p );
	void FreeAll();

	const T* GetFirst() const;
	const T* GetNext( const T* p ) const;
	T* GetFirst();
	T* GetNext( T* p );

	uint32_t Length() const;
	
private:
	struct Page
	{
		Page() : node( this ) {}
		aeListNode< Page > node;
		ObjectPool< T, N > pool;
	};
	
	uint32_t m_length;
	aeList< Page > m_pages;
	Tag m_tag;
};

//------------------------------------------------------------------------------
// ae::FreeList member functions
//------------------------------------------------------------------------------
template < typename T, uint32_t N >
FreeList< T, N >::FreeList()
{
	m_length = 0;

	for ( uint32_t i = 0; i < N - 1; i++ )
	{
		m_pool[ i ].next = &m_pool[ i + 1 ];
		m_pool[ i ].allocated = false;
	}
	m_pool[ N - 1 ].next = nullptr;
	m_pool[ N - 1 ].allocated = false;
	
	m_open = &m_pool[ 0 ];
}

template < typename T, uint32_t N >
FreeList< T, N >::~FreeList()
{
	for ( uint32_t i = 0; i < N; i++ )
	{
		Entry* entry = &m_pool[ i ];
		if ( entry->allocated )
		{
			T* object = (T*)entry->object;
			object->~T();
#if _AE_DEBUG_
			memset( object, 0xDD, sizeof(*object) );
#endif
			entry->next = nullptr;
			entry->allocated = false;
		}
	}
	m_open = nullptr;
	m_length = 0;
}

template < typename T, uint32_t N >
T* FreeList< T, N >::Allocate()
{
	if ( !m_open )
	{
		return nullptr;
	}

	Entry* entry = m_open;
	m_open = m_open->next;
	entry->next = nullptr;
	entry->allocated = true;

	m_length++;

	return new( (T*)entry->object ) T();
}

template < typename T, uint32_t N >
bool FreeList< T, N >::Free( T* p )
{
	if ( !p )
	{
		return false;
	}

	Entry* entry = m_GetEntry( p );
	if ( !entry )
	{
		return false;
	}

#if _AE_DEBUG_
	AE_ASSERT( entry->check == 0xCDCDCDCD );
	AE_ASSERT( entry->allocated );
#endif
	
	p->~T();
#if _AE_DEBUG_
	memset( p, 0xDD, sizeof(*p) );
#endif
	
	entry->next = m_open;
	entry->allocated = false;
	m_open = entry;
	m_length--;

#if _AE_DEBUG_
	if ( !m_length )
	{
		for ( uint32_t i = 0; i < N; i++ )
		{
			AE_ASSERT( !m_pool[ i ].allocated );
		}
	}
#endif
	
	return true;
}

template < typename T, uint32_t N >
const T* FreeList< T, N >::GetFirst() const
{
	if ( !m_length )
	{
		return nullptr;
	}
	const Entry* entry = &m_pool[ 0 ];
	const T* p = (const T*)entry->object;
	return entry->allocated ? p : GetNext( p );
}

template < typename T, uint32_t N >
const T* FreeList< T, N >::GetNext( const T* p ) const
{
	if ( !p )
	{
		return nullptr;
	}
	
	const Entry* entry = m_GetEntry( p );
	if ( !entry )
	{
		return nullptr;
	}
	
	for ( uint32_t i = entry - m_pool + 1; i < N; i++ )
	{
		if ( m_pool[ i ].allocated )
		{
			return (const T*)m_pool[ i ].object;
		}
	}

	return nullptr;
}

template < typename T, uint32_t N >
T* FreeList< T, N >::GetFirst()
{
	return const_cast< T* >( const_cast< const FreeList< T, N >* >( this )->GetFirst() );
}

template < typename T, uint32_t N >
T* FreeList< T, N >::GetNext( T* p )
{
	return const_cast< T* >( const_cast< const FreeList< T, N >* >( this )->GetNext( p ) );
}

template < typename T, uint32_t N >
bool FreeList< T, N >::Has( const T* p ) const
{
	if ( !p )
	{
		return false;
	}
	return m_GetEntry( p );
}

template < typename T, uint32_t N >
bool FreeList< T, N >::HasFree() const
{
	return m_open != nullptr;
}

template < typename T, uint32_t N >
uint32_t FreeList< T, N >::Length() const
{
	return m_length;
}

//------------------------------------------------------------------------------
// ae::ObjectPool member functions
//------------------------------------------------------------------------------
template < typename T, uint32_t N >
ObjectPool< T, N >::ObjectPool()
{
	m_length = 0;

	for ( uint32_t i = 0; i < N - 1; i++ )
	{
		m_pool[ i ].next = &m_pool[ i + 1 ];
		m_pool[ i ].allocated = false;
	}
	m_pool[ N - 1 ].next = nullptr;
	m_pool[ N - 1 ].allocated = false;
	
	m_open = &m_pool[ 0 ];
}

template < typename T, uint32_t N >
ObjectPool< T, N >::~ObjectPool()
{
	for ( uint32_t i = 0; i < N; i++ )
	{
		Entry* entry = &m_pool[ i ];
		if ( entry->allocated )
		{
			T* object = (T*)entry->object;
			object->~T();
#if _AE_DEBUG_
			memset( object, 0xDD, sizeof(*object) );
#endif
			entry->next = nullptr;
			entry->allocated = false;
		}
	}
	m_open = nullptr;
	m_length = 0;
}

template < typename T, uint32_t N >
T* ObjectPool< T, N >::Allocate()
{
	if ( !m_open )
	{
		return nullptr;
	}

	Entry* entry = m_open;
	m_open = m_open->next;
	entry->next = nullptr;
	entry->allocated = true;

	m_length++;

	return new( (T*)entry->object ) T();
}

template < typename T, uint32_t N >
bool ObjectPool< T, N >::Free( T* p )
{
	if ( !p )
	{
		return false;
	}

	Entry* entry = m_GetEntry( p );
	if ( !entry )
	{
		return false;
	}

#if _AE_DEBUG_
	AE_ASSERT( entry->check == 0xCDCDCDCD );
	AE_ASSERT( entry->allocated );
#endif
	
	p->~T();
#if _AE_DEBUG_
	memset( p, 0xDD, sizeof(*p) );
#endif
	
	entry->next = m_open;
	entry->allocated = false;
	m_open = entry;
	m_length--;

#if _AE_DEBUG_
	if ( !m_length )
	{
		for ( uint32_t i = 0; i < N; i++ )
		{
			AE_ASSERT( !m_pool[ i ].allocated );
		}
	}
#endif
	
	return true;
}

template < typename T, uint32_t N >
const T* ObjectPool< T, N >::GetFirst() const
{
	if ( !m_length )
	{
		return nullptr;
	}
	const Entry* entry = &m_pool[ 0 ];
	const T* p = (const T*)entry->object;
	return entry->allocated ? p : GetNext( p );
}

template < typename T, uint32_t N >
const T* ObjectPool< T, N >::GetNext( const T* p ) const
{
	if ( !p )
	{
		return nullptr;
	}
	
	const Entry* entry = m_GetEntry( p );
	if ( !entry )
	{
		return nullptr;
	}
	
	for ( uint32_t i = entry - m_pool + 1; i < N; i++ )
	{
		if ( m_pool[ i ].allocated )
		{
			return (const T*)m_pool[ i ].object;
		}
	}

	return nullptr;
}

template < typename T, uint32_t N >
T* ObjectPool< T, N >::GetFirst()
{
	return const_cast< T* >( const_cast< const ObjectPool< T, N >* >( this )->GetFirst() );
}

template < typename T, uint32_t N >
T* ObjectPool< T, N >::GetNext( T* p )
{
	return const_cast< T* >( const_cast< const ObjectPool< T, N >* >( this )->GetNext( p ) );
}

template < typename T, uint32_t N >
bool ObjectPool< T, N >::Has( const T* p ) const
{
	if ( !p )
	{
		return false;
	}
	return m_GetEntry( p );
}

template < typename T, uint32_t N >
bool ObjectPool< T, N >::HasFree() const
{
	return m_open != nullptr;
}

template < typename T, uint32_t N >
uint32_t ObjectPool< T, N >::Length() const
{
	return m_length;
}

template < typename T, uint32_t N >
const typename ObjectPool< T, N >::Entry* ObjectPool< T, N >::m_GetEntry( const T* p ) const
{
#if _AE_DEBUG_
	AE_ASSERT( p );
#endif
	Entry* entry = (Entry*)( (uint8_t*)p - offsetof( Entry, object ) );
	if ( entry < m_pool || m_pool + N <= entry )
	{
		return nullptr;
	}
#if _AE_DEBUG_
	AE_ASSERT( entry->check == 0xCDCDCDCD );
#endif
	return entry;
}

template < typename T, uint32_t N >
typename ObjectPool< T, N >::Entry* ObjectPool< T, N >::m_GetEntry( T* p )
{
	return const_cast< Entry* >( const_cast< const ObjectPool< T, N >* >( this )->m_GetEntry( p ) );
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
	FreeAll();
}

template < typename T, uint32_t N >
T* PagedObjectPool< T, N >::Allocate()
{
	m_length++;
	
	Page* page = m_pages.GetFirst();
	while ( page )
	{
		if ( T* obj = page->pool.Allocate() )
		{
			return obj;
		}
		page = page->node.GetNext();
	}
	
	page = ae::New< Page >( m_tag );
	m_pages.Append( page->node );
	T* obj = page->pool.Allocate();
	AE_ASSERT( obj );
	return obj;
}

template < typename T, uint32_t N >
bool PagedObjectPool< T, N >::Free( T* p )
{
	Page* page = m_pages.GetFirst();
	while ( page )
	{
		if ( page->pool.Free( p ) )
		{
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
void PagedObjectPool< T, N >::FreeAll()
{
	Page* page = m_pages.GetLast();
	while ( page )
	{
		Page* prev = page->node.GetPrev();
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
const T* PagedObjectPool< T, N >::GetNext( const T* p ) const
{
	if ( !p )
	{
		return nullptr;
	}
	
	const Page* page = m_pages.GetFirst();
	while ( page )
	{
		AE_ASSERT( page->pool.Length() );
		if ( const T* obj = page->pool.GetNext( p ) )
		{
			return obj;
		}
		
		bool inPrev = page->pool.Has( p );
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
T* PagedObjectPool< T, N >::GetNext( T* p )
{
	return const_cast< T* >( const_cast< const PagedObjectPool< T, N >* >( this )->GetNext( p ) );
}

template < typename T, uint32_t N >
uint32_t PagedObjectPool< T, N >::Length() const
{
	return m_length;
}

} // ae

#endif
