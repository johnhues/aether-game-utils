//------------------------------------------------------------------------------
// Reference.h
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
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
#ifndef AE_REFERENCE_H
#define AE_REFERENCE_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

namespace ae {
template< typename T > class ReferenceCounted;
class ReferenceCountedBase;

//------------------------------------------------------------------------------
// ae::Reference
//------------------------------------------------------------------------------
template< typename T >
class Reference
{
public:
	Reference() : m_counter( nullptr ) {}
	template< typename U > Reference( ae::ReferenceCounted< U >& counter );
	template< typename U > Reference& operator=( ae::ReferenceCounted< U >& counter );
	template< typename U > Reference( const Reference< U >& other );
	template< typename U > Reference( Reference< U >&& other ) noexcept;
	template< typename U > Reference& operator=( const Reference< U >& other );
	template< typename U > Reference& operator=( Reference< U >&& other ) noexcept;
	template< typename U, typename V > Reference< U > Cast( V* object ) const;
	Reference( std::nullptr_t );
	~Reference();

	explicit operator bool() const;
	T* operator->() const;
	T& operator*() const;
	T* Get() const;
	void Clear();

private:
	template< typename U > friend class Reference;
	ae::ReferenceCountedBase* m_counter;
};

//------------------------------------------------------------------------------
// ae::ReferenceCountedBase
//------------------------------------------------------------------------------
class ReferenceCountedBase
{
public:
	ReferenceCountedBase( void* _this ) : m_this( _this ) {}

	void* Get() const { return m_this; }
	uint32_t GetReferenceCount() const { return m_references; }
	void Increment() { m_references++; }
	void Decrement() { m_references--; }

private:
	ReferenceCountedBase( const ReferenceCountedBase& ) = delete;
	ReferenceCountedBase( ReferenceCountedBase&& ) = delete;
	ReferenceCountedBase& operator=( const ReferenceCountedBase& ) = delete;
	ReferenceCountedBase& operator=( ReferenceCountedBase&& ) = delete;
	uint32_t m_references = 0;
	void* m_this;
};

//------------------------------------------------------------------------------
// ae::ReferenceCounted
//------------------------------------------------------------------------------
template< typename T >
class ReferenceCounted : public ReferenceCountedBase
{
public:
	ReferenceCounted( T* _this );
	~ReferenceCounted();
	T* Get() const;

private:
	friend class Reference< T >;
};

//------------------------------------------------------------------------------
//
//
// Implementation below this point
//
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// ae::Reference member functions
//------------------------------------------------------------------------------
template< typename T >
template< typename U >
ae::Reference< T >::Reference( ae::ReferenceCounted< U >& counter ) :
	m_counter( &counter )
{
	static_assert( std::is_convertible_v< U*, T* >, "U must be convertible to T" );
	counter.Increment();
}

template< typename T >
template< typename U >
ae::Reference< T >& ae::Reference< T >::operator=( ae::ReferenceCounted< U >& counter )
{
	static_assert( std::is_convertible_v< U*, T* >, "U must be convertible to T" );
	if( &counter != m_counter )
	{
		if( m_counter )
		{
			m_counter->Decrement();
		}
		m_counter = &counter;
		counter.Increment();
	}
	return *this;
}

template< typename T >
template< typename U > 
ae::Reference< T >::Reference( const Reference< U >& other ) : m_counter( other.m_counter )
{
	static_assert( std::is_convertible_v< U*, T* >, "U must be convertible to T" );
	if( m_counter )
	{
		m_counter->Increment();
	}
}

template< typename T >
template< typename U > 
ae::Reference< T >::Reference( Reference< U >&& other ) noexcept : m_counter( other.m_counter )
{
	static_assert( std::is_convertible_v< U*, T* >, "U must be convertible to T" );
	other.m_counter = nullptr;
}

template< typename T >
template< typename U > 
ae::Reference< T >& ae::Reference< T >::operator=( const ae::Reference< U >& other )
{
	static_assert( std::is_convertible_v< U*, T* >, "U must be convertible to T" );
	if( m_counter != other.m_counter )
	{
		if( m_counter )
		{
			m_counter->Decrement();
		}
		m_counter = other.m_counter;
		if( m_counter )
		{
			m_counter->Increment();
		}
	}
	return *this;
}

template< typename T >
template< typename U > 
ae::Reference< T >& ae::Reference< T >::operator=( ae::Reference< U >&& other ) noexcept
{
	static_assert( std::is_convertible_v< U*, T* >, "U must be convertible to T" );
	if( this != &other )
	{
		m_counter = other.m_counter;
		other.m_counter = nullptr;
	}
	return *this;
}

template< typename T >
ae::Reference< T >::Reference( std::nullptr_t ) :
	m_counter( nullptr )
{}

template< typename T >
ae::Reference< T >::~Reference()
{
	Clear();
}

template< typename T >
ae::Reference< T >::operator bool() const
{
	return m_counter != nullptr;
}
template< typename T >
T* ae::Reference< T >::operator->() const
{
	return reinterpret_cast< T* >( m_counter->Get() );
}
template< typename T >
T& ae::Reference< T >::operator*() const
{
	return *reinterpret_cast< T* >( m_counter->Get() );
}
template< typename T >
T* ae::Reference< T >::Get() const
{
	return m_counter ? reinterpret_cast< T* >( m_counter->Get() ) : nullptr;
}

template< typename T >
template< typename U, typename V >
ae::Reference< U > ae::Reference< T >::Cast( V* object ) const
{
	static_assert( std::is_convertible_v< V*, T* >, "V must be convertible to T" );
	static_assert( std::is_convertible_v< V*, U* >, "V must be convertible to U" );
	if( object && m_counter->Get() == object )
	{
		ae::Reference< U > ref;
		ref.m_counter = m_counter;
		ref.m_counter->Increment();
		return ref;
	}
	return {};
}

template< typename T >
void ae::Reference< T >::Clear()
{
	if( m_counter )
	{
		m_counter->Decrement();
		m_counter = nullptr;
	}
}

//------------------------------------------------------------------------------
// ae::ReferenceCounted
//------------------------------------------------------------------------------
template< typename T >
ae::ReferenceCounted< T >::ReferenceCounted( T* _this ) : ReferenceCountedBase( _this )
{}

template< typename T >
ae::ReferenceCounted< T >::~ReferenceCounted()
{
	AE_DEBUG_ASSERT_MSG( GetReferenceCount() == 0, "'#' instance was destroyed while # reference(s) were still active", ae::GetTypeName< T >(), GetReferenceCount() );
}

template< typename T >
T* ae::ReferenceCounted< T >::Get() const
{
	return reinterpret_cast< T* >( ReferenceCountedBase::Get() );
}

} // ae namespace

#endif // AE_REFERENCE_H
