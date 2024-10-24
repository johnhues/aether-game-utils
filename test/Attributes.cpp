
#include "aether.h"

struct Attribute : public ae::Inheritor< ae::Attribute, Attribute >
{
	Attribute() = default;
	std::string fieldPath;
};
AE_REGISTER_ATTRIBUTE( Attribute );

struct CategoryInfoAttribute : public ae::Inheritor< ae::Attribute, CategoryInfoAttribute >
{
	int sortOrder;
	std::string name;
};
AE_REGISTER_ATTRIBUTE( CategoryInfoAttribute );

struct DisplayName : public ae::Inheritor< Attribute, DisplayName >
{
	std::string name;
};
AE_REGISTER_ATTRIBUTE( DisplayName );

struct AssetType : public ae::Inheritor< ae::Attribute, AssetType >
{
	AssetType() = default;
	AssetType( const char* assetType ) : assetType( assetType ) {}
	AssetType( const char* assetType, uint32_t idx ) : assetType( assetType ), idx( idx ) {}
	std::string assetType;
	uint32_t idx = 0;
};
AE_REGISTER_ATTRIBUTE( AssetType );

class GameObject : public ae::Inheritor< ae::Object, GameObject >
{
public:
	uint32_t id = 0;
};
AE_REGISTER_CLASS( GameObject );
AE_REGISTER_CLASS_ATTRIBUTE( GameObject, AssetType, ( "ba.texture_asset" ) );
AE_REGISTER_CLASS_VAR( GameObject, id );
AE_REGISTER_CLASS_VAR_ATTRIBUTE( GameObject, id, CategoryInfoAttribute, ({ .sortOrder = 1, .name = "General" }) );
AE_REGISTER_CLASS_VAR_ATTRIBUTE( GameObject, id, DisplayName, ({ .name = "ID" }) );
AE_REGISTER_CLASS_VAR_ATTRIBUTE( GameObject, id, AssetType, ( "ba.texture_asset" ) );

int main()
{
	GameObject obj;
	obj.id = 1;

	const ae::Type* gameObjectType = ae::GetType< GameObject >();
	const ae::Var* idVar = gameObjectType->GetVarByName( "id", false );
	AE_INFO( "id:#", idVar->GetObjectValueAsString( &obj ) );

	if( const AssetType* assetTypeAttrib = gameObjectType->attributes.TryGet< AssetType >() )
	{
		AE_INFO( "AssetType: '#'", assetTypeAttrib->assetType );
	}

	const CategoryInfoAttribute* categoryInfoAttrib = gameObjectType->attributes.TryGet< CategoryInfoAttribute >();
	AE_LOG( "GameObject # a 'CategoryInfoAttribute'", categoryInfoAttrib ? "has" : "does not have" );

	if( const CategoryInfoAttribute* categoryInfoAttrib = idVar->attributes.TryGet< CategoryInfoAttribute >() )
	{
		AE_INFO( "CategoryInfoAttribute::name: '#'", categoryInfoAttrib->name );
		AE_INFO( "CategoryInfoAttribute::sortOrder: '#'", categoryInfoAttrib->sortOrder );
	}

	return 0;
}
