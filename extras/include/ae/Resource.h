//------------------------------------------------------------------------------
// Resource.h
//------------------------------------------------------------------------------
// Copyright (c) 2023 John Hughes
// Created by John Hughes on 5/16/23.
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
	template < typename T > T* Add( const char* name );

	//! Loads all resources added with Add().
	bool Load();
	//! Returns true if any resources were added but have not yet loaded.
	bool AnyPendingLoad() const;
	
	//! Returns a resource with the given \p name, or nullptr if it does not exist.
	template < typename T > const T* TryGet( const char* name ) const;
	//! Returns a resource with the given \p name, or asserts if it does not exist.
	template < typename T > const T& Get( const char* name ) const;

	//! Patches the vtable of ResourceManager and all added resources.
	void HotLoad();
	
private:
	Resource* m_Add( const char* type, const char* name );
	const ae::Tag m_tag;
	ae::FileSystem* m_fs = nullptr;
	ae::Map< ae::Str64, Resource* > m_resources;
};

//------------------------------------------------------------------------------
// ResourceManager member functions
//------------------------------------------------------------------------------
template < typename T >
T* ResourceManager::Add( const char* name )
{
	return (T*)m_Add( ae::GetTypeName< T >(), name );
}

template < typename T >
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

template < typename T >
const T& ResourceManager::Get( const char* name ) const
{
	const T* resourceT = TryGet< T >( name );
	AE_ASSERT_MSG( resourceT, "No resource '#' of type '#'", name, ae::GetTypeName< T >() );
	return *resourceT;
}

} // ae namespace

#endif
