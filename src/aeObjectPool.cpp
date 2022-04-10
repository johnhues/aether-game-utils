#include "ae/aeObjectPool.h"

namespace ae {

// struct Page
// {
// 	Page( const ae::Tag& tag, uint32_t size ) : freeList( tag, size ) {}
// 	ae::ListNode< Page > node = this;
// 	ae::FreeList<> freeList;
// 	void* objects;
// };
// ae::Tag m_tag;
// uint32_t m_pageSize;
// bool m_paged;
// uint32_t m_objectSize;
// uint32_t m_objectAlignment;
// uint32_t m_length;
// ae::List< Page > m_pages;

#define _AE_POOL_ELEMENT( _arr, _idx ) ( (uint8_t*)_arr + _idx * m_objectSize )

OpaquePool::OpaquePool( const ae::Tag& tag, uint32_t objectSize, uint32_t objectAlignment, uint32_t poolSize, bool paged )
{
	AE_ASSERT( tag != ae::Tag() );
	AE_ASSERT( poolSize > 0 );
	m_tag = tag;
	m_pageSize = poolSize;
	m_paged = paged;
	m_objectSize = objectSize;
	m_objectAlignment = objectAlignment;
	m_length = 0;
}

OpaquePool::~OpaquePool()
{
	AE_ASSERT( Length() == 0 );
}

void* OpaquePool::Allocate()
{
	Page* page = m_pages.FindFn( []( const Page* page ) { return page->freeList.HasFree(); } );
	if ( !page && ( !m_pages.Length() || m_paged ) )
	{
		page = ae::New< Page >( m_tag, m_tag, m_pageSize );
		page->objects = ae::Allocate( m_tag, m_pageSize * m_objectSize, m_objectAlignment );
		m_pages.Append( page->node );
	}
	if ( page )
	{
		int32_t index = page->freeList.Allocate();
		AE_ASSERT( index >= 0 );
		m_length++;
		return _AE_POOL_ELEMENT( page->objects, index );
	}
	return nullptr;
}

void OpaquePool::Free( void* obj )
{
	if ( !obj ) { return; }
	if ( (intptr_t)obj % m_objectAlignment != 0 ) { return; } // @TODO: Should this be an assert?

	int32_t index;
	Page* page = m_pages.GetFirst();
	while ( page )
	{
		index = (int32_t)( (uint8_t*)obj - page->objects );
#if _AE_DEBUG_
		AE_ASSERT( index % m_objectSize == 0 );
#endif
		index /= m_objectSize;
		if ( 0 <= index && index < m_pageSize )
		{
			break;
		}
		page = page->node.GetNext();
	}
	if ( page )
	{
#if _AE_DEBUG_
		AE_ASSERT( _AE_POOL_ELEMENT( page->objects, index ) == obj );
		AE_ASSERT( page->freeList.IsAllocated( index ) );
		memset( obj, 0xDD, m_objectSize );
#endif
		page->freeList.Free( index );
		m_length--;

		if ( m_paged && page->freeList.Length() == 0 )
		{
			ae::Free( page->objects );
			ae::Delete( page );
		}
	}
}

void OpaquePool::FreeAll()
{
	// auto deleteAllFn = []( Page* page )
	// {
	// 	for ( uint32_t i = 0; i < m_pageSize; i++ )
	// 	{
	// 		if ( page->freeList.IsAllocated( i ) )
	// 		{
	// 			( (T*)&page->objects[ i ] )->~T();
	// 		}
	// 	}
	// 	page->freeList.FreeAll();
	// };
	// if ( m_paged )
	// {
		Page* page = m_pages.GetLast();
		while ( page )
		{
			Page* prev = page->node.GetPrev();
			// deleteAllFn( page );
			ae::Delete( page );
			page = prev;
		}
	// }
	// else
	// {
	// 	deleteAllFn( m_firstPage.Get() );
	// }
	m_length = 0;
}

const void* OpaquePool::GetFirst() const
{
	if ( m_paged )
	{
		const Page* page = m_pages.GetFirst();
		if ( page )
		{
			AE_ASSERT( page->freeList.Length() );
			return page->freeList.Length() ? _AE_POOL_ELEMENT( page->objects, page->freeList.GetFirst() ) : nullptr;
		}
	}
	else if ( !m_paged && m_length )
	{
		int32_t index = m_firstPage.Get()->freeList.GetFirst();
		AE_ASSERT( index >= 0 );
		return (const T*)&m_firstPage.Get()->objects[ index ];
	}
	AE_ASSERT( m_length == 0 );
	return nullptr;
}

const void* OpaquePool::GetNext( const void* obj ) const
{
	if ( !obj ) { return nullptr; }
	const Page* page = m_pages.GetFirst();
	while ( page )
	{
		AE_ASSERT( !m_paged || page->freeList.Length() );
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

void* OpaquePool::GetFirst()
{
	return const_cast< void* >( const_cast< const OpaquePool* >( this )->GetFirst() );
}

void* OpaquePool::GetNext( void* obj )
{
	return const_cast< void* >( const_cast< const OpaquePool* >( this )->GetNext( obj ) );
}

bool OpaquePool::HasFree() const
{
	return m_paged || !m_pages.Length() || m_pages.GetFirst()->freeList.HasFree();
}

} // namespace ae
