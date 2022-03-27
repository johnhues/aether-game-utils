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

} // ae

#endif
