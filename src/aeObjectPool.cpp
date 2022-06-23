#include "ae/aeObjectPool.h"

namespace ae {

#define _AE_POOL_ELEMENT( _arr, _idx ) ( (uint8_t*)_arr + _idx * m_objectSize )

OpaquePool::OpaquePool( const ae::Tag& tag, uint32_t objectSize, uint32_t objectAlignment, uint32_t poolSize, bool paged ) :
	m_firstPage( tag, poolSize )
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
	if ( !page )
	{
		if ( !m_firstPage.node.GetList() )
		{
#if _AE_DEBUG_
			AE_ASSERT( m_firstPage.freeList.Length() == 0 );
#endif
			page = &m_firstPage;
			page->objects = ae::Allocate( m_tag, m_pageSize * m_objectSize, m_objectAlignment );
			m_pages.Append( page->node );
		}
		else if ( m_paged )
		{
			page = ae::New< Page >( m_tag, m_tag, m_pageSize );
			page->objects = ae::Allocate( m_tag, m_pageSize * m_objectSize, m_objectAlignment );
			m_pages.Append( page->node );
		}
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
	if ( !obj )
	{
		return;
	}
#if _AE_DEBUG_
	AE_ASSERT( (intptr_t)obj % m_objectAlignment == 0 );
#endif

	int32_t index = -1;
	Page* page = m_pages.GetFirst();
	while ( page )
	{
		index = ( (uint8_t*)obj - (uint8_t*)page->objects ) / m_objectSize;
		bool found = ( 0 <= index && index < (int32_t)m_pageSize );
		if ( found )
		{
			break;
		}
		page = page->node.GetNext();
	}
	if ( page )
	{
#if _AE_DEBUG_
		AE_ASSERT( m_length > 0 );
		AE_ASSERT( _AE_POOL_ELEMENT( page->objects, index ) == obj );
		AE_ASSERT( page->freeList.IsAllocated( index ) );
		memset( obj, 0xDD, m_objectSize );
#endif
		page->freeList.Free( index );
		m_length--;

		if ( page->freeList.Length() == 0 )
		{
			ae::Free( page->objects );
			if ( page == &m_firstPage )
			{
				m_firstPage.node.Remove();
				m_firstPage.freeList.FreeAll();
			}
			else
			{
				ae::Delete( page );
			}
		}
		return;
	}
#if _AE_DEBUG_
	AE_FAIL_MSG( "Object '#' not found in pool '#:#:#:#'", m_objectSize, m_objectAlignment, m_pageSize, m_paged );
#endif
}

void OpaquePool::FreeAll()
{
	Page* page = m_pages.GetLast();
	while ( page )
	{
		Page* prev = page->node.GetPrev();
		ae::Free( page->objects );
		if ( page == &m_firstPage )
		{
			m_firstPage.node.Remove();
			m_firstPage.freeList.FreeAll();
		}
		else
		{
			ae::Delete( page );
		}
		page = prev;
	}
	m_length = 0;
}

const void* OpaquePool::GetFirst() const
{
	if ( const Page* page = m_pages.GetFirst() )
	{
#if _AE_DEBUG_
		AE_ASSERT( m_length > 0 );
		AE_ASSERT( page->freeList.Length() );
#endif
		return _AE_POOL_ELEMENT( page->objects, page->freeList.GetFirst() );
	}
#if _AE_DEBUG_
	AE_ASSERT( m_length == 0 );
#endif
	return nullptr;
}

const void* OpaquePool::GetNext( const void* obj ) const
{
	if ( !obj ) { return nullptr; }
	const Page* page = m_pages.GetFirst();
	while ( page )
	{
#if _AE_DEBUG_
		AE_ASSERT( m_length > 0 );
		AE_ASSERT( page->freeList.Length() );
#endif
		int32_t index = ( (uint8_t*)obj - (uint8_t*)page->objects ) / m_objectSize;
		bool found = ( 0 <= index && index < (int32_t)m_pageSize );
		if ( found )
		{
#if _AE_DEBUG_
			AE_ASSERT( _AE_POOL_ELEMENT( page->objects, index ) == obj );
			AE_ASSERT( page->freeList.IsAllocated( index ) );
#endif
			int32_t next = page->freeList.GetNext( index );
			if ( next >= 0 )
			{
				return _AE_POOL_ELEMENT( page->objects, next );
			}
		}
		page = page->node.GetNext();
		if ( found && page )
		{
#if _AE_DEBUG_
			AE_ASSERT( page->freeList.Length() > 0 );
#endif
			// Given object is last element of previous page so return the first element on next page
			int32_t next = page->freeList.GetFirst();
#if _AE_DEBUG_
			AE_ASSERT( 0 <= next && next < (int32_t)m_pageSize );
#endif
			return _AE_POOL_ELEMENT( page->objects, next );
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
