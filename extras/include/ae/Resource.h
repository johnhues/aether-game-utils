//------------------------------------------------------------------------------
// Resource.h
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
#ifndef AE_RESOURCE_H
#define AE_RESOURCE_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

namespace ae {

//------------------------------------------------------------------------------
// ae::Resource class
//------------------------------------------------------------------------------
class Resource : public ae::Inheritor< ae::Object, Resource >
{
public:
	virtual ~Resource() {}
	virtual bool Load() = 0;
	const ae::File* GetFile() const { return m_file; }
	bool IsLoaded() const { return m_isLoaded; }
protected:
	Resource() = default;
private:
	friend class ResourceManager;
	Resource( const Resource& ) = delete;
	void operator=( const Resource& ) = delete;
	const ae::File* m_file = nullptr;
	bool m_isLoaded = false;
};

//------------------------------------------------------------------------------
// ae::ResourceManager class
//------------------------------------------------------------------------------
class ResourceManager
{
public:
	ResourceManager( const ae::Tag& tag );
	~ResourceManager();
	//! \p fileSystem must outlive this object.
	void Initialize( ae::FileSystem* fileSystem );
	void Terminate();

	//! Adds a resource to be later loaded by Load(). After loading the resource
	//! it can be accessed by the given \p name. The given \p filePath is relative
	//! to \p rootDir.
	bool Add( const char* type, const char* name, ae::FileSystem::Root rootDir, const char* filePath );
	//! Adds a resource to be later loaded by Load(). After loading the resource
	//! it can be accessed by the given \p name. Prefer specifying a root directory.
	bool Add( const char* type, const char* name, const char* filePath );
	//! Adds a manually loaded resource. ae::Resource::IsLoaded() will always
	//! return true for this resource.
	template< typename T > T* Add( const char* name );
	//! Reloads a resource that was previously added with Add().
	void Reload( const ae::Resource* resource );

	//! Loads all resources added with Add().
	bool Load();
	//! Returns true if any resources were added but have not yet loaded.
	bool AnyPendingLoad() const;
	
	//! Returns a resource with the given \p name and type, or nullptr if it
	//! does not exist.
	template< typename T = ae::Resource > const T* TryGet( const char* name ) const;
	//! Returns a resource with the given \p name and type, or asserts if it
	//! does not exist.
	template< typename T = ae::Resource > const T& Get( const char* name ) const;
	//! Returns a resource with the given \p index, or null if the resource type
	//! does not match. Null is returned if the index is out of bounds.
	template< typename T = ae::Resource > const T* GetByIndex( uint32_t index );
	//! Returns the number of registered resources.
	uint32_t GetCount() const;

	//! Patches the vtable of ResourceManager and all added resources.
	void HotLoad();
	
private:
	Resource* m_Register( const char* type, const char* name );
	const ae::Tag m_tag;
	ae::FileSystem* m_fs = nullptr;
	ae::Map< ae::Str64, Resource* > m_resources;
};

//------------------------------------------------------------------------------
// ResourceManager member functions
//------------------------------------------------------------------------------
template< typename T >
T* ResourceManager::Add( const char* name )
{
	return (T*)m_Register( ae::GetTypeName< T >(), name );
}

template< typename T >
const T* ResourceManager::TryGet( const char* name ) const
{
	const Resource* resource = m_resources.Get( name, nullptr );
	if ( !resource )
	{
		return nullptr;
	}
	const T* resourceT = ae::Cast< T >( resource );
	if ( !resourceT )
	{
		AE_WARN( "Resource '#' does not match requested type '#'", name, ae::GetTypeName< T >() );
	}
	return resourceT;
}

template< typename T >
const T& ResourceManager::Get( const char* name ) const
{
	const T* resourceT = TryGet< T >( name );
	AE_ASSERT_MSG( resourceT, "No resource '#' of type '#'", name, ae::GetTypeName< T >() );
	return *resourceT;
}

template< typename T >
const T* ResourceManager::GetByIndex( uint32_t index )
{
	return ae::Cast< T >( m_resources.GetValue( index ) );
}

} // ae namespace

#endif
