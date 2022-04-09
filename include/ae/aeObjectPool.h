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
	OpaquePool( const ae::Tag& tag, uint32_t poolSize, bool paged, uint32_t objectSize, uint32_t objectAlignment );
	//! All objects allocated with ae::OpaquePool::Allocate/New() must be destroyed before
	//! the ae::OpaquePool is destroyed.
	~OpaquePool();

	// //! Returns a pointer to a freshly constructed object T or null if there
	// //! are no free objects. Call ae::OpaquePool::Delete() to destroy the object.
	// //! ae::OpaquePool::Delete() must be called on every object returned
	// //! by ae::OpaquePool::New() although it is safe to mix calls to ae::OpaquePool::Allocate/New()
	// //! and ae::OpaquePool::Free/Delete() as long as constructors and destructors
	// //! are called manually with ae::OpaquePool::Allocate() and ae::OpaquePool::Free().
	// template < typename T = void > T* New();
	// //! Destructs and releases the object \p obj for future use. It is safe for \p obj to be null.
	// template < typename T = void > void Delete( T* obj );
	// //! Destructs and releases all objects for future use.
	// template < typename T = void > void DeleteAll();
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

} // ae

#endif
