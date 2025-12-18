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

using ResourceId = ae::Str64;

//------------------------------------------------------------------------------
// ae::Resource class
//------------------------------------------------------------------------------
class Resource : public ae::Inheritor< ae::Object, Resource >
{
public:
	virtual ~Resource() {}
	virtual bool Load( const ae::File* file ) = 0;
	template< typename T > const T* GetParams() const { return ae::Cast< T >( m_params ); }

	ResourceId GetId() const { return m_id; }
	const char* GetPath() const { return m_path.c_str(); }
	bool IsLoaded() const { return m_isLoaded; }

protected:
	Resource() = default;
private:
	friend class ResourceManager;
	Resource( const Resource& ) = delete;
	void operator=( const Resource& ) = delete;
	ResourceId m_id;
	const ae::Object* m_params = nullptr;
	ae::ListNode< Resource > m_node = this;
	bool m_isLoaded = false;
	ae::Str256 m_path;
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
	//! Cleans up and invalidates all resources. This should be called before
	//! the file system is destroyed.
	void Terminate();

	//! Adds a resource to be later loaded by Load(). After loading the resource
	//! it can be accessed by the given \p name. The given \p filePath is relative
	//! to \p rootDir.
	bool Add( const char* type, ResourceId id, ae::FileSystem::Root rootDir, const char* filePath );
	//! Adds a resource to be later loaded by Load(). After loading the resource
	//! it can be accessed by the given \p name. Prefer specifying a root directory.
	bool Add( const char* type, ResourceId id, const char* filePath );
	//! ...
	template< typename T, typename P > bool Add( ResourceId id, ae::FileSystem::Root rootDir, const char* filePath, const P& params );
	//! ...
	template< typename T, typename P > bool Add( ResourceId id, const char* filePath, const P& params );
	//! Adds a manually loaded resource. ae::Resource::IsLoaded() will always
	//! return true for this resource.
	template< typename T > T* Add( ResourceId id );
	//! Reloads a resource that was previously added with Add().
	void Reload( const ae::Resource* resource );

	//! Loads all resources added with Add(). Returns true if there are no
	//! resources pending load.
	bool Load();
	//! Returns true if any resources were added but have not yet loaded.
	bool AnyPendingLoad() const;
	
	//! Returns a resource with the given \p name and type, or nullptr if it
	//! does not exist.
	template< typename T = ae::Resource > const T* TryGet( ResourceId id ) const;
	//! Returns a resource with the given \p name and type, or asserts if it
	//! does not exist.
	template< typename T = ae::Resource > const T& Get( ResourceId id ) const;
	//! Returns a resource with the given \p index, or null if the resource type
	//! does not match. Null is returned if the index is out of bounds.
	template< typename T = ae::Resource > const T* GetByIndex( uint32_t index );
	//! Returns the number of registered resources.
	uint32_t GetCount() const;

	//! Patches the vtable of ResourceManager and all added resources.
	void HotLoad();
	
private:
	struct FileInfo { ae::List< Resource > resources; };
	Resource* m_Register( const ae::ClassType* resourceType, ResourceId id, ae::FileSystem::Root rootDir, const char* filePath );
	const ae::Tag m_tag;
	ae::FileSystem* m_fs = nullptr;
	ae::Map< const ae::File*, FileInfo* > m_files;
	ae::Map< ResourceId, Resource* > m_resources;
	ae::List< Resource > m_resourcesToLoad; // Resources that aren't associated with a file
};

//------------------------------------------------------------------------------
// ResourceManager member functions
//------------------------------------------------------------------------------
template< typename T >
T* ResourceManager::Add( ResourceId id )
{
	return (T*)m_Register( ae::GetClassType< T >(), id, ae::FileSystem::Root::Data, "" );
}

template< typename T, typename P >
bool ResourceManager::Add( ResourceId id, ae::FileSystem::Root rootDir, const char* filePath, const P& params )
{
	if( T* resource = (T*)m_Register( ae::GetClassType< T >(), id, rootDir, filePath ) )
	{
		resource->m_params = ae::New< P >( m_tag, params );
		AE_INFO( "Queuing load '#'...", filePath );
		return true;
	}
	return false;
}

template< typename T, typename P >
bool ResourceManager::Add( ResourceId id, const char* filePath, const P& params )
{
	return Add< T >( id, ae::FileSystem::Root::Data, filePath, params );
}

template< typename T >
const T* ResourceManager::TryGet( ResourceId id ) const
{
	const Resource* resource = m_resources.Get( id, nullptr );
	if( !resource )
	{
		return nullptr;
	}
	const T* resourceT = ae::Cast< T >( resource );
	if( !resourceT )
	{
		AE_WARN( "Resource '#' does not match requested type '#'", id, ae::GetTypeName< T >() );
	}
	return resourceT;
}

template< typename T >
const T& ResourceManager::Get( ResourceId id ) const
{
	const T* resourceT = TryGet< T >( id );
	AE_ASSERT_MSG( resourceT, "No resource '#' of type '#'", id, ae::GetTypeName< T >() );
	return *resourceT;
}

template< typename T >
const T* ResourceManager::GetByIndex( uint32_t index )
{
	return ae::Cast< T >( m_resources.GetValue( index ) );
}

} // ae namespace

#endif
