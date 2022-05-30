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
// ae::OpaquePool class
//------------------------------------------------------------------------------
class OpaquePool
{
public:
	OpaquePool( const ae::Tag& tag, uint32_t objectSize, uint32_t objectAlignment, uint32_t poolSize, bool paged );
	//! All objects allocated with ae::OpaquePool::Allocate/New() must be destroyed before
	//! the ae::OpaquePool is destroyed.
	~OpaquePool();

	//! Returns a pointer to a freshly constructed object T or null if there
	//! are no free objects. Call ae::OpaquePool::Delete() to destroy the object.
	//! ae::OpaquePool::Delete() must be called on every object returned
	//! by ae::OpaquePool::New() although it is safe to mix calls to ae::OpaquePool::Allocate/New()
	//! and ae::OpaquePool::Free/Delete() as long as constructors and destructors
	//! are called manually with ae::OpaquePool::Allocate() and ae::OpaquePool::Free().
	template < typename T > T* New();
	//! Destructs and releases the object \p obj for future use. It is safe for \p obj to be null.
	template < typename T > void Delete( T* obj );
	//! Destructs and releases all objects for future use.
	template < typename T > void DeleteAll();

	// //! Returns the first allocated object in the pool or null if the pool is empty.
	// template < typename T = void > const T* GetFirst() const;
	// //! Returns the next allocated object after \p obj or null if there are no more objects.
	// //! Null will be returned if \p obj is null.
	// template < typename T = void > const T* GetNext( const T* obj ) const;
	// //! Returns the first allocated object in the pool or null if the pool is empty.
	// template < typename T = void > T* GetFirst();
	// //! Returns the next allocated object after \p obj or null if there are no more objects.
	// //! Null will be returned if \p obj is null.
	// template < typename T = void > T* GetNext( T* obj );

	//! Returns a pointer to an object or null if there are no free objects. The
	//! user is responsible for any constructor calls. ae::OpaquePool::Free() must
	//! be called on every object returned by ae::OpaquePool::Allocate(). It is safe
	//! to mix calls to ae::OpaquePool::Allocate/New() and ae::OpaquePool::Free/Delete()
	//! as long as constructors and destructors are called manually with
	//! ae::OpaquePool::Allocate() and ae::OpaquePool::Free().
	void* Allocate();
	//! Releases the object \p obj for future use. It is safe for \p obj to be null.
	void Free( void* obj );

	//! Releases all objects for future use by ae::OpaquePool::Allocate().
	//! THIS FUNCTION DOES NOT CALL THE OBJECTS DESTRUCTORS, so please use with caution!
	void FreeAll();
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
	uint32_t Length() const { return m_length; }
	//! Returns the total number of objects in the pool. Note that this number
	//! can grow and shrink for paged pools.
	uint32_t Size() const { return m_pageSize * m_pages.Length(); }
	//! Returns the maximum number of objects per page.
	uint32_t PageSize() const { return m_pageSize; }

private:
	// @TODO: Disable copy constructor etc or fix list on copy.
	struct Page
	{
		// Pages are deleted by the pool when empty, so it's safe to
		// assume pages always contain at least one object.
		Page( const ae::Tag& tag, uint32_t size ) : freeList( tag, size ) {}
		ae::ListNode< Page > node = this; // List node.
		ae::FreeList<> freeList; // Free object information.
		void* objects; // Pointer to array of objects in this page.
	};
	ae::Tag m_tag;
	uint32_t m_pageSize; // Number of objects per page.
	bool m_paged; // If true, pool can be infinitely big.
	uint32_t m_objectSize; // Size of each object.
	uint32_t m_objectAlignment; // Alignment of each object.
	uint32_t m_length; // Number of actively allocated objects.
	ae::List< Page > m_pages;
};

template < typename T >
T* OpaquePool::New()
{
	AE_ASSERT( sizeof( T ) == m_objectSize );
	AE_ASSERT( alignof( T ) == m_objectAlignment );
	void* obj = Allocate();
	if( obj )
	{
		return new( obj ) T();
	}
	return nullptr;
}

template < typename T >
void OpaquePool::Delete( T* obj )
{
	AE_ASSERT( sizeof( T ) == m_objectSize );
	AE_ASSERT( alignof( T ) == m_objectAlignment );
	if( obj )
	{
		obj->~T();
		Free( obj );
	}
}

template < typename T >
void OpaquePool::DeleteAll()
{
	AE_ASSERT( sizeof( T ) == m_objectSize );
	AE_ASSERT( alignof( T ) == m_objectAlignment );
	for ( void* p = GetFirst(); p; p = GetNext( p ) )
	{
		( (T*)p )->~T();
	}
	FreeAll();
}

} // ae

#endif
