#include "aether.h"

struct Attribute : public ae::Inheritor< ae::Attribute, Attribute >
{
	Attribute() = default;
	ae::Str128 fieldPath;
};
AE_REGISTER_CLASS( Attribute );

struct CategoryInfoAttribute final : public ae::Inheritor< ae::Attribute, CategoryInfoAttribute >
{
	int sortOrder;
	ae::Str128 name;
};
AE_REGISTER_CLASS( CategoryInfoAttribute );
AE_REGISTER_CLASS_VAR( CategoryInfoAttribute, sortOrder );
AE_REGISTER_CLASS_VAR( CategoryInfoAttribute, name );

struct DisplayName final : public ae::Inheritor< Attribute, DisplayName >
{
	ae::Str128 name;
};
AE_REGISTER_CLASS( DisplayName );

struct RequiresAttrib final : public ae::Inheritor< Attribute, RequiresAttrib >
{
	RequiresAttrib( const char* name ) : name( name ) {}
	ae::Str128 name;
};
AE_REGISTER_CLASS( RequiresAttrib );

class GameObject : public ae::Inheritor< ae::Object, GameObject >
{
public:
	uint32_t id = 0;
};
AE_REGISTER_CLASS( GameObject );
AE_REGISTER_CLASS_ATTRIBUTE( GameObject, RequiresAttrib, ( "Something" ) );
AE_REGISTER_CLASS_ATTRIBUTE( GameObject, RequiresAttrib, ( "SomethingElse" ) );
AE_REGISTER_CLASS_ATTRIBUTE( GameObject, RequiresAttrib, ( "SomethingSomethingElse" ) );
AE_REGISTER_CLASS_ATTRIBUTE( GameObject, RequiresAttrib, ( "SomethingSomethingElseElse" ) );
AE_REGISTER_CLASS_VAR( GameObject, id );
AE_REGISTER_CLASS_VAR_ATTRIBUTE( GameObject, id, CategoryInfoAttribute, ({ .sortOrder = 1, .name = "General" }) );
AE_REGISTER_CLASS_VAR_ATTRIBUTE( GameObject, id, DisplayName, ({ .name = "ID" }) );
AE_REGISTER_CLASS_VAR_ATTRIBUTE( GameObject, id, DisplayName, ({ .name = "ID2" }) );

namespace xyz
{
	class Util : public ae::Inheritor< ae::Object, Util >
	{
	public:
		uint32_t id = 0;
	};
}
AE_REGISTER_CLASS( xyz, Util );
AE_REGISTER_CLASS_ATTRIBUTE( xyz, Util, RequiresAttrib, ( "Something" ) );
AE_REGISTER_CLASS_ATTRIBUTE( xyz, Util, RequiresAttrib, ( "SomethingElse" ) );
AE_REGISTER_CLASS_ATTRIBUTE( xyz, Util, RequiresAttrib, ( "SomethingSomethingElse" ) );
AE_REGISTER_CLASS_ATTRIBUTE( xyz, Util, RequiresAttrib, ( "SomethingSomethingElseElse" ) );
AE_REGISTER_CLASS_VAR( xyz, Util, id );
AE_REGISTER_CLASS_VAR_ATTRIBUTE( xyz, Util, id, CategoryInfoAttribute, ({ .sortOrder = 1, .name = "General" }) );
AE_REGISTER_CLASS_VAR_ATTRIBUTE( xyz, Util, id, DisplayName, ({ .name = "ID" }) );
AE_REGISTER_CLASS_VAR_ATTRIBUTE( xyz, Util, id, DisplayName, ({ .name = "ID2" }) );

int main()
{
	const ae::ClassType* gameObjectType = ae::GetClassType< GameObject >();
	if( const ae::SourceFileAttribute* sourceFileAttrib = gameObjectType->attributes.TryGet< ae::SourceFileAttribute >() )
	{
		AE_INFO( "# source: '#'", gameObjectType->GetName(), *sourceFileAttrib );
	}
	const ae::ClassVar* idVar = gameObjectType->GetVarByName( "id", false );

	if( const ae::SourceFileAttribute* sourceFileAttrib = idVar->attributes.TryGet< ae::SourceFileAttribute >() )
	{
		AE_INFO( "#::#: '#'", gameObjectType->GetName(), idVar->GetName(), *sourceFileAttrib );
	}

	GameObject obj;
	obj.id = 1;
	AE_INFO( "id:#", idVar->GetObjectValueAsString( &obj ) );

	const CategoryInfoAttribute* categoryInfoAttrib = gameObjectType->attributes.TryGet< CategoryInfoAttribute >();
	AE_LOG( "GameObject # a 'CategoryInfoAttribute'", categoryInfoAttrib ? "has" : "does not have" );

	if( const CategoryInfoAttribute* categoryInfoAttrib = idVar->attributes.TryGet< CategoryInfoAttribute >() )
	{
		AE_INFO( "CategoryInfoAttribute::name: '#'", categoryInfoAttrib->name );
		AE_INFO( "CategoryInfoAttribute::sortOrder: '#'", categoryInfoAttrib->sortOrder );
	}

	gameObjectType->attributes.Has< RequiresAttrib >();

	for( uint32_t i = 0; i < gameObjectType->attributes.GetCount< ae::Attribute >(); i++ )
	{
		const ae::Attribute* attribute = gameObjectType->attributes.TryGet< ae::Attribute >( i );
		const ae::ClassType* attributeType = ae::GetClassTypeById( attribute->_metaTypeId ); // @TODO: ae::GetClassTypeFromObject( attribute );
		AE_LOG( "# attribute: '#'", gameObjectType->GetName(), attributeType->GetName() );
		if( const RequiresAttrib* requiresAttrib = ae::Cast< RequiresAttrib >( attribute ) )
		{
			AE_INFO( "RequiresAttrib::name: '#'", requiresAttrib->name );
		}
		else if( const ae::SourceFileAttribute* sourceFileAttrib = ae::Cast< ae::SourceFileAttribute >( attribute ) )
		{
			AE_INFO( "SourceFileAttribute::path: '#:#'", sourceFileAttrib->path, sourceFileAttrib->line );
		}
	}

	for( uint32_t i = 0; i < idVar->attributes.GetCount< ae::Attribute >(); i++ )
	{
		const ae::Attribute* attribute = idVar->attributes.TryGet< ae::Attribute >( i );
		const ae::ClassType* attributeType = ae::GetClassTypeById( attribute->_metaTypeId ); // @TODO: ae::GetClassTypeFromObject( attribute );
		AE_LOG( "# attribute: '#'", idVar->GetName(), attributeType->GetName() );
		if( const CategoryInfoAttribute* categoryInfoAttrib = ae::Cast< CategoryInfoAttribute >( attribute ) )
		{
			AE_INFO( "CategoryInfoAttribute::name: '#'", categoryInfoAttrib->name );
			AE_INFO( "CategoryInfoAttribute::sortOrder: '#'", categoryInfoAttrib->sortOrder );
		}
		else if( const ae::SourceFileAttribute* sourceFileAttrib = ae::Cast< ae::SourceFileAttribute >( attribute ) )
		{
			AE_INFO( "SourceFileAttribute::path: '#:#'", sourceFileAttrib->path, sourceFileAttrib->line );
		}
	}

	return 0;
}
