# `Meta` 

## Members

#### `define `[`AE_FORCE_LINK_CLASS`](#group___meta_1ga5858ee77bc33f4635ac5a52cbb10e452) 

#### `define `[`AE_REGISTER_CLASS`](#group___meta_1ga07afa3dae66b3f1514ff6e8b6b3b485f) 

#### `define `[`AE_REGISTER_CLASS_PROPERTY`](#group___meta_1ga53387eb5da156362c645666c0d45d426) 

#### `define `[`AE_REGISTER_CLASS_PROPERTY_VALUE`](#group___meta_1gaa38aba066ec964be6238cc9a85a5cedd) 

#### `define `[`AE_REGISTER_CLASS_VAR`](#group___meta_1gabeb0841bae6090d9c874b5882593a329) 

#### `define `[`AE_REGISTER_CLASS_VAR_PROPERTY`](#group___meta_1ga6e57cf376c52eb705a9d8d620616d483) 

#### `define `[`AE_REGISTER_CLASS_VAR_PROPERTY_VALUE`](#group___meta_1gaef8363eea0ee92816949c4b0858f858d) 

#### `define `[`AE_DEFINE_ENUM_CLASS`](#group___meta_1ga48c722eae52adaf7df0e7aaf4e9f0aa5) 

Define a new enum (must register with AE_REGISTER_ENUM_CLASS)

#### `define `[`AE_REGISTER_ENUM_CLASS`](#group___meta_1ga8c3af799a1095b904696d5db529b431f) 

Register an enum defined with AE_DEFINE_ENUM_CLASS.

#### `define `[`AE_REGISTER_ENUM`](#group___meta_1gac1d647c4bb004b713baffe3439434854) 

Register an already defined c-style enum type.

#### `define `[`AE_REGISTER_ENUM_PREFIX`](#group___meta_1ga2f11949e0bd608caefb633f6a4ea46dd) 

Register an already defined c-style enum type where each value has a prefix.

#### `define `[`AE_REGISTER_ENUM_VALUE`](#group___meta_1ga940257d9c97d66d82376b4828fff7c2f) 

Register c-style enum value.

#### `define `[`AE_REGISTER_ENUM_VALUE_NAME`](#group___meta_1ga58fcc2d8fee3fe2aea62bcb85b98a36a) 

Register c-style enum value with a manually specified name.

#### `define `[`AE_REGISTER_ENUM_CLASS2`](#group___meta_1ga95fce8fa8005e4f41265bbc439071800) 

Register an already defined enum class type.

#### `define `[`AE_REGISTER_ENUM_CLASS2_VALUE`](#group___meta_1gacd8871a931b04aa103faa537906ff8df) 

Register enum class value.

#### `enum `[`BasicType`](#group___meta_1ga21bd3dd536e7f0c5bda369c790a2c145) 

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
String            | 
UInt8            | 
UInt16            | 
UInt32            | 
UInt64            | 
Int8            | 
Int16            | 
Int32            | 
Int64            | 
Int2            | 
Int3            | 
Bool            | 
Float            | 
Double            | 
Vec2            | 
Vec3            | 
Vec4            | 
Matrix4            | 
Color            | 
Enum            | 
Pointer            | 
CustomRef            | 

#### `public uint32_t `[`GetTypeCount`](#group___meta_1ga6eeb5fdd0649c0fc626afbef2ebae4df)`()` 

Get the number of registered ae::Type's.

#### `public const Type * `[`GetTypeByIndex`](#group___meta_1gaba8f1a231a28d39bf56df2c2bc3828dc)`(uint32_t i)` 

Get a registered ae::Type by index.

#### `public const Type * `[`GetTypeById`](#group___meta_1ga48b9186859f64e443acfcb951c28d3e7)`(ae::TypeId id)` 

Get a registered ae::Type by id. Same as ae::Type::GetId()

#### `public const Type * `[`GetTypeByName`](#group___meta_1ga9fa05ccb881d9bfa07fd5b8fcf96f42a)`(const char * typeName)` 

Get a registered ae::Type from a type name.

#### `public const Type * `[`GetTypeFromObject`](#group___meta_1ga22467d8b903fef18528f9aa6be14c2fa)`(const `[`ae::Object`](Meta.md#classae_1_1_object)` & obj)` 

Get a registered ae::Type from an [ae::Object](Meta.md#classae_1_1_object).

#### `public const Type * `[`GetTypeFromObject`](#group___meta_1ga9b1b3210371f82d15cf6c1890fefbf61)`(const `[`ae::Object`](Meta.md#classae_1_1_object)` * obj)` 

Get a registered ae::Type from a pointer to an [ae::Object](Meta.md#classae_1_1_object).

#### `public template<>`  <br/>`const Type * `[`GetType`](#group___meta_1gacb422361d977a20fba7f99b523109519)`()` 

Get a registered ae::Type directly from a type.

#### `public const class Enum * `[`GetEnum`](#group___meta_1ga0cde75f00f4a00aea660439f8b8c40a7)`(const char * enumName)` 

Get a registered ae::Enum by name.

#### `public ae::TypeId `[`GetObjectTypeId`](#group___meta_1ga178718ca7a6c8fe4e695cacf318ace77)`(const `[`ae::Object`](Meta.md#classae_1_1_object)` * obj)` 

Get a registered ae::TypeId from an [ae::Object](Meta.md#classae_1_1_object).

#### `public ae::TypeId `[`GetTypeIdFromName`](#group___meta_1ga64241f197a65e225be26ba49174a3bc8)`(const char * name)` 

Get a registered ae::TypeId from a type name.

#### `public template<>`  <br/>`const T * `[`Cast`](#group___meta_1ga71ea1f5f60bd9551e5f5fdb3e7deea64)`(const C * obj)` 

#### `public template<>`  <br/>`T * `[`Cast`](#group___meta_1gaccc5aa2edcfd0c896b561f30d0bc2da3)`(C * obj)` 

<br></br>
# `ae::Object` class 

Base class for all meta registered objects.

Inherit from this using ae::Inheritor and register your classes with AE_REGISTER_CLASS.

#### `public ae::TypeId `[`_metaTypeId`](#classae_1_1_object_1a23c0b6e453a15b4dbfe3b172a99c3700) 

#### `public `[`ae::Str32`](DataStructures.md#classae_1_1_str)` `[`_typeName`](#classae_1_1_object_1ad16619a5831019816ac137eaa799a359) 

#### `public inline virtual  `[`~Object`](#classae_1_1_object_1aa520b26b199e7bab5dc8ab53abfa1cca)`()` 

#### `public inline ae::TypeId `[`GetTypeId`](#classae_1_1_object_1a852fa4a4422667d56da6e5f8ae90ad11)`() const` 

<br></br>
# `ae::Inheritor` class 

```
class ae::Inheritor
  : public Parent
```  

#### `public  `[`Inheritor`](#classae_1_1_inheritor_1a7d0ddc6326ed2effd6d576f189da2ddd)`()` 

#### `typedef `[`aeBaseType`](#classae_1_1_inheritor_1a5fc8d88cd27c8e99cef6b24bef449453) 

<br></br>
# `ae::Enum` class 

#### `public inline const char * `[`GetName`](#classae_1_1_enum_1a08e3004baf752bf73b7708e64182630e)`() const` 

#### `public inline uint32_t `[`TypeSize`](#classae_1_1_enum_1a4ab9e37ad4cb664d8739a53a46b97246)`() const` 

#### `public inline bool `[`TypeIsSigned`](#classae_1_1_enum_1a32d8a3125442d7293b768fce2a2eee57)`() const` 

#### `public template<>`  <br/>`std::string `[`GetNameByValue`](#classae_1_1_enum_1a81795626d6e0115f9f54fc7ecfabc7d0)`(T value) const` 

#### `public template<>`  <br/>`bool `[`GetValueFromString`](#classae_1_1_enum_1ab076d10cfcc07e05039893544d8226ea)`(const char * str,T * valueOut) const` 

#### `public template<>`  <br/>`T `[`GetValueFromString`](#classae_1_1_enum_1ac94009c63e700c3a310a8a5e993057c7)`(const char * str,T defaultValue) const` 

#### `public template<>`  <br/>`bool `[`HasValue`](#classae_1_1_enum_1a5568786bd7333a80a416af5f2f2fffd6)`(T value) const` 

#### `public int32_t `[`GetValueByIndex`](#classae_1_1_enum_1a01cf86b1d7df946ac424c8e01d4f5767)`(int32_t index) const` 

#### `public std::string `[`GetNameByIndex`](#classae_1_1_enum_1a8016d73b190c9b21444e9e138734f7d7)`(int32_t index) const` 

#### `public uint32_t `[`Length`](#classae_1_1_enum_1a55be02619d1ff8487f9bbc1aeb2fee18)`() const` 

#### `public  `[`Enum`](#classae_1_1_enum_1ad129c7c72ee1edf107028022be5de5b4)`(const char * name,uint32_t size,bool isSigned)` 

#### `public void `[`m_AddValue`](#classae_1_1_enum_1a77949db53e65a18e1b56a34d7864887f)`(const char * name,int32_t value)` 

<br></br>
# `ae::Var` class 

Information about a member variable registered with AE_REGISTER_CLASS_VAR().

#### `public const ae::Type * `[`m_owner`](#classae_1_1_var_1a73484a0ffc0e819e4918099033cc5414) 

#### `public `[`ae::Str32`](DataStructures.md#classae_1_1_str)` `[`m_name`](#classae_1_1_var_1ada7d737f8a92d3058ee796c540ed4632) 

#### `public BasicType `[`m_type`](#classae_1_1_var_1a6280cb6ece7f8b20db9999fb3d93208c) 

#### `public `[`ae::Str32`](DataStructures.md#classae_1_1_str)` `[`m_typeName`](#classae_1_1_var_1ace84fa64d6d3dde96da5d5713caf5a9e) 

#### `public uint32_t `[`m_offset`](#classae_1_1_var_1ab372b3a5f34b75b149d7e808da881a74) 

#### `public uint32_t `[`m_size`](#classae_1_1_var_1a72cf90cacc6b9858725a3d4e27344424) 

#### `public const ae::VarTypeBase * `[`m_varType`](#classae_1_1_var_1a5a96362b3f32fa3ea47d2d31a230cb2e) 

#### `public ae::TypeId `[`m_subTypeId`](#classae_1_1_var_1a63ad1676d5460a21eaf26e77a58543f9) 

#### `public const class Enum * `[`m_enum`](#classae_1_1_var_1a141609b924b038d9b00a85e7996b271f) 

#### `public const ArrayAdapter * `[`m_arrayAdapter`](#classae_1_1_var_1a407b96f32b50adb0dbc7ab4559b2e1c3) 

#### `public ae::Map< `[`ae::Str32](DataStructures.md#classae_1_1_str), ae::Array< [ae::Str32`](DataStructures.md#classae_1_1_str)`, kMaxMetaPropListLength >, kMaxMetaProps > `[`m_props`](#classae_1_1_var_1aa08b6690a5bfcd665e0a00090aa05e9e) 

#### `public const char * `[`GetName`](#classae_1_1_var_1a0e36f6061c5be3a957ad11a8bc54219b)`() const` 

#### `public BasicType `[`GetType`](#classae_1_1_var_1af29062f12a367d1ea6bba8507a6c4051)`() const` 

#### `public const char * `[`GetTypeName`](#classae_1_1_var_1a31c053121098f75fcabefddd2dbb35f8)`() const` 

#### `public uint32_t `[`GetOffset`](#classae_1_1_var_1aa9a84f6b7d0058117ac2429dfa425fca)`() const` 

#### `public uint32_t `[`GetSize`](#classae_1_1_var_1a42a40d7d61c84e5076eb8a59b3bf9359)`() const` 

#### `public std::string `[`GetObjectValueAsString`](#classae_1_1_var_1ae0825c5ef6af69ad089c9cedc8f899f8)`(const `[`ae::Object`](Meta.md#classae_1_1_object)` * obj,int32_t arrayIdx) const` 

Get the value of this variable from the given `obj`.

If the type of this variable is a reference then ae::SetSerializer() must be called in advance, otherwise this function will assert. 
#### Parameters
* `obj` The object to get the value from. 

* `arrayIdx` Must be negative for non-array types. For array types this specifies which array element to return. Must be a valid array index, less than ae::Var::GetGetArrayLength(). 

#### Returns
Returns a string representation of the value of this variable from the given `obj`.

#### `public bool `[`SetObjectValueFromString`](#classae_1_1_var_1a6444250ec5555eb663f5f40cdc446afc)`(`[`ae::Object`](Meta.md#classae_1_1_object)` * obj,const char * value,int32_t arrayIdx) const` 

Set the value of this variable on the given `obj`.

If the type of this variable is a reference then ae::SetSerializer() must be called in advance, otherwise this function will assert. 
#### Parameters
* `obj` The object to set the value on. 

* `value` A string representation of the value to set. 

* `arrayIdx` Must be negative for non-array types. For array types this specifies which array element to set. Must be a valid array index, less than ae::Var::GetGetArrayLength(). 

#### Returns
True if `obj` was modified, and false otherwise. Reference types fail to be set if the value does not represent null or a valid reference to an existing object of the correct type.

#### `public template<>`  <br/>`bool `[`GetObjectValue`](#classae_1_1_var_1af3d633697ceba7d707679f58d85db43f)`(`[`ae::Object`](Meta.md#classae_1_1_object)` * obj,T * valueOut,int32_t arrayIdx) const` 

Get the value of this variable from the given `obj`.

If the type of this variable is a reference then ae::SetSerializer() must be called in advance, otherwise this function will assert. 
#### Parameters
* `T` The type of the value to return. 

#### Parameters
* `obj` The object to get the value from. 

* `valueOut` A pointer to the value to set. Will only be set if the type matches this variable's type. 

* `arrayIdx` Must be negative for non-array types. For array types this specifies which array element to return. Must be a valid array index, less than ae::Var::GetGetArrayLength(). 

#### Returns
Returns true if `valueOut` was set, and false otherwise.

#### `public template<>`  <br/>`bool `[`SetObjectValue`](#classae_1_1_var_1ae735c8880f6270618bd9368a1a755740)`(`[`ae::Object`](Meta.md#classae_1_1_object)` * obj,const T & value,int32_t arrayIdx) const` 

Set the value of this variable on the given `obj`.

If the type of this variable is a reference then ae::SetSerializer() must be called in advance, otherwise this function will assert. 
#### Parameters
* `T` The type of the value to set. 

#### Parameters
* `obj` The object to set the value on. 

* `value` The value to set. 

* `arrayIdx` Must be negative for non-array types. For array types this specifies which array element to set. Must be a valid array index, less than ae::Var::GetGetArrayLength(). 

#### Returns
True if `obj` was modified, and false otherwise. Reference types fail to be set if the value does not represent null or a valid reference to an existing object of the correct type.

#### `public const class Enum * `[`GetEnum`](#classae_1_1_var_1a0a174564f427846ec0424d35d8905603)`() const` 

#### `public const ae::Type * `[`GetSubType`](#classae_1_1_var_1a092a8470ab11e6f41ae02d97c248b989)`() const` 

For Ref and Array types.

#### `public bool `[`IsArray`](#classae_1_1_var_1abd85113f0354b7f4c0d01ac282de2b3c)`() const` 

#### `public bool `[`IsArrayFixedLength`](#classae_1_1_var_1a91571467084dc4a80cdf7c67960a9872)`() const` 

#### `public uint32_t `[`SetArrayLength`](#classae_1_1_var_1a9a9cd904483db0130cf3c00238438e05)`(`[`ae::Object`](Meta.md#classae_1_1_object)` * obj,uint32_t length) const` 

Returns new length of array.

#### `public uint32_t `[`GetArrayLength`](#classae_1_1_var_1a635a05eafc153df5c775b6c636da0463)`(const `[`ae::Object`](Meta.md#classae_1_1_object)` * obj) const` 

#### `public uint32_t `[`GetArrayMaxLength`](#classae_1_1_var_1a35eb2d315474f67436173f77dbb3b8b6)`() const` 

#### `public bool `[`HasProperty`](#classae_1_1_var_1ace9d942ea7305e043eaa7530dbbe21a1)`(const char * prop) const` 

#### `public int32_t `[`GetPropertyIndex`](#classae_1_1_var_1ad024b950e030016613ac9695b0d21885)`(const char * prop) const` 

#### `public int32_t `[`GetPropertyCount`](#classae_1_1_var_1a77698f4ae34db4d503597a354e4057c8)`() const` 

#### `public const char * `[`GetPropertyName`](#classae_1_1_var_1a4d7a03bf4ae6aefca9c6eddb636a0b12)`(int32_t propIndex) const` 

#### `public uint32_t `[`GetPropertyValueCount`](#classae_1_1_var_1a0f872bfba4de1ea5cdd4011575525db4)`(int32_t propIndex) const` 

#### `public uint32_t `[`GetPropertyValueCount`](#classae_1_1_var_1a36f898b6c8b18d1001a93e6803d654dc)`(const char * propName) const` 

#### `public const char * `[`GetPropertyValue`](#classae_1_1_var_1a587a68f4fa225157713cfcac4a28b3da)`(int32_t propIndex,uint32_t valueIndex) const` 

#### `public const char * `[`GetPropertyValue`](#classae_1_1_var_1a46284146fc60c067ccaf6ec60030fe8f)`(const char * propName,uint32_t valueIndex) const` 

#### `public void `[`m_AddProp`](#classae_1_1_var_1a7f185f699b862beaa2096a9e9cb579fa)`(const char * prop,const char * value)` 

<br></br>
# `ae::Type` class 

#### `public ae::TypeId `[`GetId`](#classae_1_1_type_1aa3643fd068d275c9ace7eddae515902f)`() const` 

#### `public bool `[`HasProperty`](#classae_1_1_type_1a08d000b37aa650bba33aefb632e49f18)`(const char * property) const` 

Check if this ae::Type has a `property` registered with AE_REGISTER_CLASS_PROPERTY() or AE_REGISTER_CLASS_PROPERTY_VALUE().

#### Returns
True if `property` is found.

#### `public const Type * `[`GetTypeWithProperty`](#classae_1_1_type_1af73e0db2daf1ee59eae302a2d876498a)`(const char * property) const` 

Search for an inherited type (starting from this one) that has a `property` registered with AE_REGISTER_CLASS_PROPERTY() or AE_REGISTER_CLASS_PROPERTY_VALUE().

#### Returns
The first ae::Type found with `property`.

#### `public int32_t `[`GetPropertyIndex`](#classae_1_1_type_1a1b31385721f716c4c05e1565d42f6bb1)`(const char * prop) const` 

#### `public int32_t `[`GetPropertyCount`](#classae_1_1_type_1a53d8f244e1f2775c8214a3b6b4bae3f4)`() const` 

#### `public const char * `[`GetPropertyName`](#classae_1_1_type_1aca4d52e8e572c1ea7a084ddd509e69c5)`(int32_t propIndex) const` 

#### `public uint32_t `[`GetPropertyValueCount`](#classae_1_1_type_1aa43eaaf54bad0cae236c7e59f33b5ac6)`(int32_t propIndex) const` 

#### `public uint32_t `[`GetPropertyValueCount`](#classae_1_1_type_1a8a82792921b3b26f2bb96d551b5fe193)`(const char * propName) const` 

#### `public const char * `[`GetPropertyValue`](#classae_1_1_type_1ad7f037d0b92b0261b89a3429386e166c)`(int32_t propIndex,uint32_t valueIndex) const` 

#### `public const char * `[`GetPropertyValue`](#classae_1_1_type_1ad090dafd90c5b1970f2f64c0d67e295c)`(const char * propName,uint32_t valueIndex) const` 

#### `public uint32_t `[`GetVarCount`](#classae_1_1_type_1adb146e24629a22decc8d8c7b7324b145)`(bool parents) const` 

#### `public const `[`ae::Var`](Meta.md#classae_1_1_var)` * `[`GetVarByIndex`](#classae_1_1_type_1a39776df00126791d8399dbbca3762e19)`(uint32_t i,bool parents) const` 

#### `public const `[`ae::Var`](Meta.md#classae_1_1_var)` * `[`GetVarByName`](#classae_1_1_type_1aaa9dca76274794d7f38731c646a36966)`(const char * name,bool parents) const` 

#### `public template<>`  <br/>`T * `[`New`](#classae_1_1_type_1a6bc7995a43c0954a8f527e5e6e695467)`(void * obj) const` 

#### `public uint32_t `[`GetSize`](#classae_1_1_type_1a912df9573cef83d51c4dd4a520d66f75)`() const` 

#### `public uint32_t `[`GetAlignment`](#classae_1_1_type_1a6b9e2829d9539019b3df80bc6764d475)`() const` 

#### `public const char * `[`GetName`](#classae_1_1_type_1ada13a14e330e95acb0dd8bc21e39766a)`() const` 

#### `public bool `[`HasNew`](#classae_1_1_type_1a773a8fbdce95ec7556a10242acedfab4)`() const` 

#### `public bool `[`IsAbstract`](#classae_1_1_type_1ae9beed66ac1c9f885a88f6cd28fe079a)`() const` 

#### `public bool `[`IsPolymorphic`](#classae_1_1_type_1adebdc7ba61f5855cd2246057d73416e4)`() const` 

#### `public bool `[`IsDefaultConstructible`](#classae_1_1_type_1a882df193b384819c677a601c3d55ba9a)`() const` 

#### `public bool `[`IsFinal`](#classae_1_1_type_1a1f3de1e53d923b8ca3f3a2aa5946e38c)`() const` 

#### `public const char * `[`GetParentTypeName`](#classae_1_1_type_1ade734fc49260e09335f46c5bf7280852)`() const` 

#### `public const Type * `[`GetParentType`](#classae_1_1_type_1a67dc5cf1fbb8c21fc4c51fad132a5026)`() const` 

#### `public bool `[`IsType`](#classae_1_1_type_1aec75320b7330a5f596f63acb0d165072)`(const Type * otherType) const` 

#### `public template<>`  <br/>`bool `[`IsType`](#classae_1_1_type_1ac6bd7c448170ed27f42566dd8f66fc5b)`() const` 

#### `public template<>`  <br/>`std::enable_if< !std::is_abstract< T >::value &&std::is_default_constructible< T >::value, void >::type `[`Init`](#classae_1_1_type_1aca32d638de32b22764f3b3f1e1c0c8c3)`(const char * name,uint32_t index)` 

#### `public template<>`  <br/>`std::enable_if< std::is_abstract< T >::value||!std::is_default_constructible< T >::value, void >::type `[`Init`](#classae_1_1_type_1ab8975c5d248f60308de80cdec146c16a)`(const char * name,uint32_t index)` 

#### `public void `[`m_AddProp`](#classae_1_1_type_1acea9f70f0f86c04975251e9fecf8bc80)`(const char * prop,const char * value)` 

#### `public void `[`m_AddVar`](#classae_1_1_type_1acd64bca9824b7b93ebebf8a5afae91f1)`(const `[`Var`](Meta.md#classae_1_1_var)` & var)` 

