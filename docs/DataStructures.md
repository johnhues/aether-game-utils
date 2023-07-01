# `DataStructures` 

## Members

#### `enum `[`MapMode`](#group___data_structures_1ga373e9e4ff5f747ed151298d183c250bb) 

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
Fast            | 
Stable            | 

Set ae::Map to Fast mode to allow reording of elements.

Stable to maintain the order of inserted elements.

#### `public inline std::ostream & `[`operator<<`](#group___data_structures_1ga57da57125fcbf3407a192c731e2f60fe)`(std::ostream & os,const ae::Dict & dict)` 

<br></br>
# `ae::Str` class 

A fixed length string class.

The templated value is the total size of the string in memory.

#### `public  `[`Str`](#classae_1_1_str_1abb93494f3047ced926afda10c989f7c8)`()` 

#### `public template<>`  <br/>` `[`Str`](#classae_1_1_str_1af541c5df461ddf8c3f21a55d9f0178e1)`(const `[`Str`](#classae_1_1_str)`< N2 > & str)` 

#### `public  `[`Str`](#classae_1_1_str_1a3f45dc69a054a7065be8250f639e9a69)`(const char * str)` 

#### `public  `[`Str`](#classae_1_1_str_1a9be6d89c2cce8133c8f5182205a87fbe)`(uint32_t length,const char * str)` 

#### `public  `[`Str`](#classae_1_1_str_1a181d9a55a0e77dfe1a3f4d6f017c7657)`(uint32_t length,char c)` 

#### `public template<>`  <br/>` `[`Str`](#classae_1_1_str_1ae66865a04a71a6e7ef4ca0179de9c878)`(const char * format,Args... args)` 

#### `public  explicit `[`operator const char *`](#classae_1_1_str_1af2ed55c3cc80c1b2d807a9d9892ff952)`() const` 

#### `public template<>`  <br/>`void `[`operator=`](#classae_1_1_str_1a557abc2c1ad37c2d165beb46db0d6d18)`(const `[`Str`](#classae_1_1_str)`< N2 > & str)` 

#### `public template<>`  <br/>[`Str`](#classae_1_1_str)`< N > `[`operator+`](#classae_1_1_str_1a904aa0c787a913cc71b3dbb1a94fe686)`(const `[`Str`](#classae_1_1_str)`< N2 > & str) const` 

#### `public template<>`  <br/>`void `[`operator+=`](#classae_1_1_str_1ab0274ee71f8042cca9cb19dabf0f6978)`(const `[`Str`](#classae_1_1_str)`< N2 > & str)` 

#### `public template<>`  <br/>`bool `[`operator==`](#classae_1_1_str_1a0fdcbff31fe6d7c07df5ecd3ca47d2b2)`(const `[`Str`](#classae_1_1_str)`< N2 > & str) const` 

#### `public template<>`  <br/>`bool `[`operator!=`](#classae_1_1_str_1a5b58d65b208d331116bff2b773e9e82f)`(const `[`Str`](#classae_1_1_str)`< N2 > & str) const` 

#### `public template<>`  <br/>`bool `[`operator<`](#classae_1_1_str_1a13e9aa6ceb9749ae83511eb1666a45f4)`(const `[`Str`](#classae_1_1_str)`< N2 > & str) const` 

#### `public template<>`  <br/>`bool `[`operator>`](#classae_1_1_str_1a1052bb08db3c79ac20e2ddbf7a414214)`(const `[`Str`](#classae_1_1_str)`< N2 > & str) const` 

#### `public template<>`  <br/>`bool `[`operator<=`](#classae_1_1_str_1a6febe09a40a445f27daf4e860210c154)`(const `[`Str`](#classae_1_1_str)`< N2 > & str) const` 

#### `public template<>`  <br/>`bool `[`operator>=`](#classae_1_1_str_1ae027e36ab8706e69fc9ce5eab8bcda71)`(const `[`Str`](#classae_1_1_str)`< N2 > & str) const` 

#### `public `[`Str`](#classae_1_1_str)`< N > `[`operator+`](#classae_1_1_str_1a7e84cd844c513a965c1260dad30c9416)`(const char * str) const` 

#### `public void `[`operator+=`](#classae_1_1_str_1a8399904c2ce7fe9bf83681caba8ff48f)`(const char * str)` 

#### `public bool `[`operator==`](#classae_1_1_str_1a8e322f788d938f5ae9f0b02b0058eee6)`(const char * str) const` 

#### `public bool `[`operator!=`](#classae_1_1_str_1a7c5fd5cb080a7c2c367efae6dbe605cf)`(const char * str) const` 

#### `public bool `[`operator<`](#classae_1_1_str_1ab078bc7ae8e87a55bf95c689da7f61b2)`(const char * str) const` 

#### `public bool `[`operator>`](#classae_1_1_str_1a9ba17b4fc812fdb76765f16ad8fd88d2)`(const char * str) const` 

#### `public bool `[`operator<=`](#classae_1_1_str_1aeb91d558338d46291849a9c1d5b4e5e0)`(const char * str) const` 

#### `public bool `[`operator>=`](#classae_1_1_str_1ae785b6578a93c784dde47f34e74cbc51)`(const char * str) const` 

#### `public char & `[`operator[]`](#classae_1_1_str_1aaa4c8e2e9d95477a1c99ec1ef13e5b4a)`(uint32_t i)` 

#### `public const char `[`operator[]`](#classae_1_1_str_1a546af89f286c1fd8621b03ba9582c830)`(uint32_t i) const` 

#### `public const char * `[`c_str`](#classae_1_1_str_1af94563d6b8d6db689a773bf40abae306)`() const` 

#### `public template<>`  <br/>`void `[`Append`](#classae_1_1_str_1adefe38f106799f44f9c2b4468fc45d98)`(const `[`Str`](#classae_1_1_str)`< N2 > & str)` 

#### `public void `[`Append`](#classae_1_1_str_1a7a2b9b8a6840ad9dd64db35aa10f581d)`(const char * str)` 

#### `public void `[`Trim`](#classae_1_1_str_1a197c9299fa80d6081da87a71ed1a9e22)`(uint32_t len)` 

#### `public uint32_t `[`Length`](#classae_1_1_str_1a5a5eb5373182a86a1328224e0c6add3d)`() const` 

#### `public uint32_t `[`Size`](#classae_1_1_str_1a6faf765a4a59e47ae31fcc514fe18f4f)`() const` 

#### `public bool `[`Empty`](#classae_1_1_str_1ad08b6bb5aa69bf0ae412d3189f46ae20)`() const` 

<br></br>
# `ae::Array` class 

#### `public  `[`Array`](#classae_1_1_array_1a9ce0cc51d518006c1950601590ebd1ff)`()` 

Static array (N > 0) only.

#### `public  `[`Array`](#classae_1_1_array_1a77beb3433c3cf3accb89343d6c804a9a)`(const T & val,uint32_t length)` 

Static array (N > 0) only. Appends 'length' number of 'val's.

#### `public  `[`Array`](#classae_1_1_array_1a3db9d76d81b73550d00138caaf8ee3e9)`(std::initializer_list< T > initList)` 

Static array (N > 0) only. Construct from a standard initializer list.

#### `public  `[`Array`](#classae_1_1_array_1aadf5e36ee6d21e93d12ff44f5b86a150)`(ae::Tag tag)` 

Dynamic array (N == 0) only.

#### `public  `[`Array`](#classae_1_1_array_1ac3f851816642f9ccc1474982550ab81a)`(ae::Tag tag,uint32_t size)` 

Dynamic array (N == 0) only. Reserve size (with length of 0).

#### `public  `[`Array`](#classae_1_1_array_1ab0efbfe8871ef3d7c5444076aa608139)`(ae::Tag tag,const T & val,uint32_t length)` 

Dynamic array (N == 0) only.

Reserves 'length' and appends 'length' number of 'val's.

#### `public void `[`Reserve`](#classae_1_1_array_1a4ec3588b1ab7772066a0a8778404600c)`(uint32_t total)` 

Dynamic array (N == 0) only.

Expands array storage to avoid copying data unneccesarily on Append(). Retrieve the current storage limit with Size().

#### `public  `[`Array`](#classae_1_1_array_1a8bda8c9ff61a777d029902e24580f02c)`(const Array< T, N > & other)` 

Copy constructor.

The ae::Tag of `other` will be used for the newly constructed array if the array is dynamic (N == 0).

#### `public  `[`Array`](#classae_1_1_array_1a11cff98c616cdb71a5463a56c759510e)`(Array< T, N > && other) noexcept` 

Move constructor falls back to the regular copy constructor for static arrays (N > 0) or if the given ae::Tags don't match.

#### `public void `[`operator=`](#classae_1_1_array_1ac5e66f33b699d3a3d45d5e32b7b5bea7)`(const Array< T, N > & other)` 

Assignment operator.

#### `public void `[`operator=`](#classae_1_1_array_1a72eb23e610445d29c201e8f943a1ef32)`(Array< T, N > && other) noexcept` 

Move assignment operator falls back to the regular assignment operator for static arrays (N > 0) or if the given ae::Tags don't match.

#### `public  `[`~Array`](#classae_1_1_array_1a201792d63d037514979d432e4b478ccc)`()` 

#### `public T & `[`Append`](#classae_1_1_array_1a6f360f107d9b06eb8cdc3e37c49117ce)`(const T & value)` 

Adds one copy of `value` to the end of the array.

Can reallocate internal storage for dynamic arrays (N == 0), so take care when taking the address of any elements. Returns a reference to the added entry.

#### `public T * `[`Append`](#classae_1_1_array_1ae9796b7004dbc5f422c7bfdeb3a260f6)`(const T & value,uint32_t count)` 

Adds `count` copies of `value` to the end of the array.

Can reallocate internal storage for dynamic arrays (N == 0), so take care when taking the address of any elements. Returns a pointer to the first new element added, or one past the end of the array if `count` is zero.

#### `public T * `[`AppendArray`](#classae_1_1_array_1a2003ecf537bba12b40a81d5092b9cc51)`(const T * values,uint32_t count)` 

Adds `count` elements from `values`.

Can reallocate internal storage for dynamic arrays (N == 0), so take care when taking the address of any elements. Returns a pointer to the first new element added, or one past the end of the array if `count` is zero.

#### `public T & `[`Insert`](#classae_1_1_array_1a2b1a1801ab5ee50b2a836c5cdfca9f7b)`(uint32_t index,const T & value)` 

Adds one copy of `value` at `index`.

`index` must be less than or equal to Length(). Can reallocate internal storage for dynamic arrays (N == 0), so take care when taking the address of any elements. Returns a reference to the added entry.

#### `public T * `[`Insert`](#classae_1_1_array_1a8f67d37ef48b159933af4f10084caf0a)`(uint32_t index,const T & value,uint32_t count)` 

Adds `count` copies of `value` at `index`.

`index` must be less than or equal to Length(). Can reallocate internal storage for dynamic arrays (N == 0), so take care when taking the address of any elements. Returns a pointer to the first new element added, or the address of the element at `index` if `count` is zero.

#### `public T * `[`InsertArray`](#classae_1_1_array_1a8485a235c0076dba49b94104f0d8b849)`(uint32_t index,const T * values,uint32_t count)` 

Adds `count` elements from `values` at `index`.

`index` must be less than or equal to Length(). Can reallocate internal storage for dynamic arrays (N == 0), so take care when taking the address of any elements. Returns a pointer to the first new element added, or the address of the element at `index` if `count` is zero.

#### `public template<>`  <br/>`int32_t `[`Find`](#classae_1_1_array_1a7d37472803080b2c34f704af6861469d)`(const U & value) const` 

Returns the index of the first matching element or -1 when not found.

#### `public template<>`  <br/>`int32_t `[`FindLast`](#classae_1_1_array_1ae8f57ef4818a45b8a27a5c12a8c2b4a1)`(const U & value) const` 

Returns the index of the last matching element or -1 when not found.

#### `public template<>`  <br/>`int32_t `[`FindFn`](#classae_1_1_array_1a0fef1c1eef6471da44ec9b8b206c5655)`(Fn testFn) const` 

Returns the index of the first matching element or -1 when not found.

The function signature should match 'bool (*)( const T2& )' or '[...]( const T2& ) -> bool'. Return true from the predicate for a any matching element.

#### `public template<>`  <br/>`int32_t `[`FindLastFn`](#classae_1_1_array_1a15758ae09a05f7b94ac14ef13882c91f)`(Fn testFn) const` 

Returns the index of the last matching element or -1 when not found.

The function signature should match 'bool (*)( const U& )' or '[...]( const T2& ) -> bool'. Return true from the predicate for any matching element.

#### `public template<>`  <br/>`uint32_t `[`RemoveAll`](#classae_1_1_array_1aae28481ffd38ddc6e2c3a27025cf31d4)`(const U & value)` 

Remove all elements that match `value`.

Returns the number of elements that were removed.

#### `public template<>`  <br/>`uint32_t `[`RemoveAllFn`](#classae_1_1_array_1a4149849eb82f91be882189e556960a4f)`(Fn testFn)` 

Remove elements based on predicate `testFn`.

The function signature should match 'bool (*)( const U& )' and '[]( const U& ) -> bool'. Return true from the predicate for removal of the given element. Returns the number of elements that were removed.

#### `public void `[`Remove`](#classae_1_1_array_1a893057f1f7f9b33025ed9f15edaaa278)`(uint32_t index,uint32_t count)` 

Removes `count` elements at `index`.

`index` plus `count` must be less than or equal to Length().

#### `public void `[`Clear`](#classae_1_1_array_1a2a4ce35a44f8bccac254ed4c7f541b45)`()` 

Destructs all elements in the array and resets the array to length zero.

Does not affect the size of the array.

#### `public const T & `[`operator[]`](#classae_1_1_array_1aa8511565c8e30d5f8c164d4972c390b6)`(int32_t index) const` 

Performs bounds checking in debug mode. Use 'GetData()' to get raw array.

#### `public T & `[`operator[]`](#classae_1_1_array_1a2c9766bef405a18606ad67980e484de4)`(int32_t index)` 

Performs bounds checking in debug mode. Use 'GetData()' to get raw array.

#### `public inline T * `[`Data`](#classae_1_1_array_1a49d4ae1328a86186a9da6eaa57df095d)`()` 

Returns a pointer to the first element of the array, but can return null when the array length is zero.

#### `public inline const T * `[`Data`](#classae_1_1_array_1a0690efac9354df3776612cf4a2e837a5)`() const` 

Returns a pointer to the first element of the array, but can return null when the array length is zero.

#### `public inline uint32_t `[`Length`](#classae_1_1_array_1afe6a164371993c93121e561cacb9764f)`() const` 

Returns the number of elements currently in the array.

#### `public inline _AE_DYNAMIC_STORAGE uint32_t `[`Size`](#classae_1_1_array_1a9ae494d0ff3352f83bb87398fde010ed)`(...) const` 

Returns the total size of a dynamic array (N == 0)

#### `public inline ae::Tag `[`Tag`](#classae_1_1_array_1ac45c0f09445733788ab2a9b7d1e12867)`() const` 

Returns the tag provided to the constructor for dynamic arrays (N == 0).

Returns ae::Tag() for all static arrays (N > 0).

#### `public inline T * `[`begin`](#classae_1_1_array_1ac34bcd684ddbf101deebb3cb35ad300c)`()` 

For ranged-based looping.

Returns a pointer to the first element of the array, but can return null when array length is zero. Lowercase to match the c++ standard.

#### `public inline T * `[`end`](#classae_1_1_array_1aa17ff60ab57f7f163dd006e70c476496)`()` 

For ranged-based looping.

Returns a pointer one past the last element of the array, but can return null when array length is zero. Lowercase to match the c++ standard.

#### `public inline const T * `[`begin`](#classae_1_1_array_1a33071a3f43ad95a28392c1008e4888e5)`() const` 

For ranged-based looping.

Returns a pointer to the first element of the array, but can return null when array length is zero. Lowercase to match the c++ standard.

#### `public inline const T * `[`end`](#classae_1_1_array_1a77a4cf1a5394d015de3d7908c01ace6e)`() const` 

For ranged-based looping.

Returns a pointer one past the last element of the array, but can return null when array length is zero. Lowercase to match the c++ standard.

<br></br>
# `ae::HashMap` class 

#### `public  `[`HashMap`](#classae_1_1_hash_map_1a427cfe062966f6ecd8e524a745aee963)`()` 

Constructor for a hash map with static allocated storage (N > 0).

#### `public  `[`HashMap`](#classae_1_1_hash_map_1a78a15a8fc6bdf2e52af8361da23466f6)`(ae::Tag pool)` 

Constructor for a hash map with dynamically allocated storage (N == 0).

#### `public void `[`Reserve`](#classae_1_1_hash_map_1ae565d5ac8583ba89b42a78a13eadad56)`(uint32_t size)` 

Expands the storage if necessary so a `size` number of key/index pairs can be added without any internal allocations.

Asserts if using static storage and `size` is greater than N.

#### `public  `[`HashMap`](#classae_1_1_hash_map_1a5ed551c6d3d1a699b58cccd0849c22c7)`(const HashMap< N > & other)` 

#### `public void `[`operator=`](#classae_1_1_hash_map_1a045c94b77c2a3dde0f3286019467ff82)`(const HashMap< N > & other)` 

#### `public  `[`~HashMap`](#classae_1_1_hash_map_1a7aaa19c7086941a119e1318d9bf1eb50)`()` 

Releases allocated storage.

#### `public bool `[`Set`](#classae_1_1_hash_map_1aa5ed38d3d9a18304ffee77ddfe28b778)`(uint32_t key,uint32_t index)` 

Adds an entry for lookup with ae::HashMap::Get().

If the key already exists the index will be updated. In both cases the return value will be true, and false otherwise.

#### `public int32_t `[`Remove`](#classae_1_1_hash_map_1add7bfd79fa2c7528c86a34d4d21511c5)`(uint32_t key)` 

Removes the entry with `key` if it exists.

Returns the index associated with the removed key on success, -1 otherwise.

#### `public void `[`Decrement`](#classae_1_1_hash_map_1a2cb6336f4dabc051c4837e56f99fe0aa)`(uint32_t index)` 

Decrements the existing index values supplied to ae::HashMap::Insert() of all entries greater than `index`.

Useful when index values represent offsets into another array being compacted after the removal of an entry.

#### `public int32_t `[`Get`](#classae_1_1_hash_map_1a40282d5940ac30fa8a59784321b3a1e1)`(uint32_t key) const` 

Returns the index associated with the given key, or -1 if the key is not found.

#### `public void `[`Clear`](#classae_1_1_hash_map_1aebc78d239f7a5c15e176ee07518e9d42)`()` 

Removes all entries.

#### `public uint32_t `[`Length`](#classae_1_1_hash_map_1a16c8cfe352086f26f0c9228780224d2b)`() const` 

Returns the number of entries.

#### `public inline _AE_DYNAMIC_STORAGE uint32_t `[`Size`](#classae_1_1_hash_map_1a2d60a56309dbff2f78065911bd99f1c8)`(...) const` 

Returns the number of allocated entries.

<br></br>
# `ae::Map` class 

#### `public  `[`Map`](#classae_1_1_map_1a1b7e60b4f2a92c5ba8b623c2f7da35b6)`()` 

Constructor for a map with static allocated storage (N > 0)

#### `public  `[`Map`](#classae_1_1_map_1a98d0f0c8c9d578eefb44ad148cac2379)`(ae::Tag pool)` 

Constructor for a map with dynamically allocated storage (N == 0)

#### `public void `[`Reserve`](#classae_1_1_map_1a656db8eae5a8c3abdd8b86977f646c35)`(uint32_t count)` 

Expands the map storage if necessary so a `count` number of key/value pairs can be added without any internal allocations.

Asserts if using static storage and `count` is less than N.

#### `public Value & `[`Set`](#classae_1_1_map_1a1f07706c6b3fa06c63c88e3fbc0cbb83)`(const Key & key,const Value & value)` 

Add or replace a key/value pair in the map.

Can be retrieved with ae::Map::Get(). The value is updated in place if the element is found, otherwise the new pair is appended. It's not safe to keep a pointer to the value across non-const operations.

#### `public Value & `[`Get`](#classae_1_1_map_1a85f301e4b3c2f0a63516b5733e6c5d8c)`(const Key & key)` 

Returns a modifiable reference to the value set with `key`.

Asserts when key/value pair is missing.

#### `public const Value & `[`Get`](#classae_1_1_map_1a6af928551ef30a8d50eeaebb27a08160)`(const Key & key) const` 

Returns the value set with `key`. Asserts when key/value pair is missing.

#### `public const Value & `[`Get`](#classae_1_1_map_1ade06b678a6f545c93a3084395ee478b4)`(const Key & key,const Value & defaultValue) const` 

Returns the value set with `key`.

Returns `defaultValue` otherwise when the key/value pair is missing.

#### `public Value * `[`TryGet`](#classae_1_1_map_1a1815c09ebff97b4797cd55863fc79919)`(const Key & key)` 

Returns a pointer to the value set with `key`.

Returns null otherwise when the key/value pair is missing.

#### `public const Value * `[`TryGet`](#classae_1_1_map_1a2f1dc49bfa0cf51a11d34206485491fc)`(const Key & key) const` 

Returns a pointer to the value set with `key`.

Returns null otherwise when the key/value pair is missing.

#### `public bool `[`TryGet`](#classae_1_1_map_1af83b0e74a58b75cb3fb66851e72624c4)`(const Key & key,Value * valueOut)` 

Returns true when `key` matches an existing key/value pair.

A copy of the value is set to `valueOut`.

#### `public bool `[`TryGet`](#classae_1_1_map_1a1b343e3eead1a20925d0be63d27bd223)`(const Key & key,Value * valueOut) const` 

Returns true when `key` matches an existing key/value pair.

A copy of the value is set to `valueOut`.

#### `public bool `[`Remove`](#classae_1_1_map_1af2858612b941b162c5a4912f6ea32d28)`(const Key & key,Value * valueOut)` 

Performs a constant time removal of an element with `key` while potentially re-ordering elements with ae::MapMode::Fast.

Performs a linear time removal of an element with `key` with ae::MapMode::Stable. Returns true on success, and a copy of the value is set to `valueOut` if it is not null.

#### `public void `[`RemoveIndex`](#classae_1_1_map_1aa672f3e1c4442d7d388a524910e99097)`(uint32_t index,Value * valueOut)` 

Removes an element by index. See ae::Map::Remove() for more details.

#### `public void `[`Clear`](#classae_1_1_map_1a3434a2fa44eea0935e1e053d63744733)`()` 

Remove all key/value pairs from the map.

#### `public const Key & `[`GetKey`](#classae_1_1_map_1a32cfbd733f347691a2d738765a4e3d89)`(int32_t index) const` 

Access elements by index. Returns the nth key in the map.

#### `public const Value & `[`GetValue`](#classae_1_1_map_1ae3927fd0a1fb4b5b90130f9037244951)`(int32_t index) const` 

Access elements by index. Returns the nth value in the map.

#### `public Value & `[`GetValue`](#classae_1_1_map_1a45bc643a1e5bc61ef0346c62b1570c13)`(int32_t index)` 

Access elements by index.

Returns a modifiable reference to the nth value in the map.

#### `public int32_t `[`GetIndex`](#classae_1_1_map_1a389ff91f6b479572612c223d2beab504)`(const Key & key) const` 

Returns the index of a key/value pair in the map.

Returns -1 when key/value pair is missing.

#### `public uint32_t `[`Length`](#classae_1_1_map_1a7a806e7702bca463895597d846ca536f)`() const` 

Returns the number of key/value pairs in the map.

#### `public inline _AE_DYNAMIC_STORAGE uint32_t `[`Size`](#classae_1_1_map_1aa2fae9f99974132d5adf6c1aea624ea8)`(...) const` 

Returns the number of allocated entries.

#### `public inline ae::Pair< Key, Value > * `[`begin`](#classae_1_1_map_1a9e4ce7a90971a1491e5e4a43f9dc17bc)`()` 

#### `public inline ae::Pair< Key, Value > * `[`end`](#classae_1_1_map_1aababd500a25fc48f9030c96984ee00df)`()` 

#### `public inline const ae::Pair< Key, Value > * `[`begin`](#classae_1_1_map_1a516abb999689408a242443afd36ea247)`() const` 

#### `public inline const ae::Pair< Key, Value > * `[`end`](#classae_1_1_map_1a06aa0aa1685bf5cf33531b49878b45cf)`() const` 

<br></br>
# `ae::Dict` class 

#### `public  `[`Dict`](#classae_1_1_dict_1a6bfa292229ec8aeaac4e4f964f380c46)`(ae::Tag tag)` 

#### `public void `[`SetString`](#classae_1_1_dict_1a5b20f4fccabef8c5480ec7de447c94ba)`(const char * key,const char * value)` 

#### `public inline void `[`SetString`](#classae_1_1_dict_1ae5fa8a6caf85fd5a53a9d520c8d2f413)`(const char * key,char * value)` 

#### `public void `[`SetInt`](#classae_1_1_dict_1ad507cf96b9b60039813c5a21b2318e42)`(const char * key,int32_t value)` 

#### `public void `[`SetUint`](#classae_1_1_dict_1af998ffc57dabf6813dc368279460256d)`(const char * key,uint32_t value)` 

#### `public void `[`SetFloat`](#classae_1_1_dict_1a0f883be3201b7c265317248f373e6a42)`(const char * key,float value)` 

#### `public void `[`SetDouble`](#classae_1_1_dict_1a2cd6d69ff494cc2f2576affad70b5bd9)`(const char * key,double value)` 

#### `public void `[`SetBool`](#classae_1_1_dict_1a0d826d9fdffd733a1a1e53c700b9ef58)`(const char * key,bool value)` 

#### `public void `[`SetVec2`](#classae_1_1_dict_1a78c80529ea68827ab33be691222a483b)`(const char * key,ae::Vec2 value)` 

#### `public void `[`SetVec3`](#classae_1_1_dict_1aa6e0c898014d9ea2fad5f5a17de26417)`(const char * key,ae::Vec3 value)` 

#### `public void `[`SetVec4`](#classae_1_1_dict_1a0655e3798b149457e3c5e6491e670e8e)`(const char * key,ae::Vec4 value)` 

#### `public void `[`SetInt2`](#classae_1_1_dict_1aea33af5b9fd15cbd46824b3e81392712)`(const char * key,ae::Int2 value)` 

#### `public void `[`SetMatrix4`](#classae_1_1_dict_1a0f663f3134e797f46f60e662af8aeda0)`(const char * key,const ae::Matrix4 & value)` 

#### `public void `[`Clear`](#classae_1_1_dict_1aaf4b53dbb5e91dbbc9b2ec9cdd63c50e)`()` 

#### `public const char * `[`GetString`](#classae_1_1_dict_1afc8df3544a5305b71e7b26488412b063)`(const char * key,const char * defaultValue) const` 

#### `public int32_t `[`GetInt`](#classae_1_1_dict_1a71e5fc431feee56425f8fc2d7c331e8b)`(const char * key,int32_t defaultValue) const` 

#### `public uint32_t `[`GetUint`](#classae_1_1_dict_1ad5767ac952c1d6ae1e109f3dcf9a4d5b)`(const char * key,uint32_t defaultValue) const` 

#### `public float `[`GetFloat`](#classae_1_1_dict_1a99c77991d2bdcbd7f06703369fca8779)`(const char * key,float defaultValue) const` 

#### `public double `[`GetDouble`](#classae_1_1_dict_1aea766ea8b18b5543e1b77d187780d2f9)`(const char * key,double defaultValue) const` 

#### `public bool `[`GetBool`](#classae_1_1_dict_1a24cd050b30380e12645bdf3807549997)`(const char * key,bool defaultValue) const` 

#### `public ae::Vec2 `[`GetVec2`](#classae_1_1_dict_1ad340edda8e7a3508f88cf9db62e820df)`(const char * key,ae::Vec2 defaultValue) const` 

#### `public ae::Vec3 `[`GetVec3`](#classae_1_1_dict_1aef8d1955d20feb4269353ce0370787d3)`(const char * key,ae::Vec3 defaultValue) const` 

#### `public ae::Vec4 `[`GetVec4`](#classae_1_1_dict_1a58a3a54583f7fb08830d816b3b60b2f6)`(const char * key,ae::Vec4 defaultValue) const` 

#### `public ae::Int2 `[`GetInt2`](#classae_1_1_dict_1a20efc06a6df26b6fcccb20c8ce8b3735)`(const char * key,ae::Int2 defaultValue) const` 

#### `public ae::Matrix4 `[`GetMatrix4`](#classae_1_1_dict_1af594704228c5636fb24954eb159e4d94)`(const char * key,const ae::Matrix4 & defaultValue) const` 

#### `public bool `[`Has`](#classae_1_1_dict_1ac6f1f8f96ea03753146dbd181f767386)`(const char * key) const` 

#### `public const char * `[`GetKey`](#classae_1_1_dict_1a9d6ed8840d1d98b6299d5517ebf2d933)`(uint32_t idx) const` 

#### `public const char * `[`GetValue`](#classae_1_1_dict_1a9e89f5aa58cf93d2a0201e8630e65812)`(uint32_t idx) const` 

#### `public inline uint32_t `[`Length`](#classae_1_1_dict_1a7df7dfd4136756fb7ef3783d7aed77c1)`() const` 

#### `public inline ae::Pair< `[`ae::Str128](DataStructures.md#classae_1_1_str), [ae::Str128`](DataStructures.md#classae_1_1_str)` > * `[`begin`](#classae_1_1_dict_1ad9b7221c3b461d5e8991056ff2da44df)`()` 

#### `public inline ae::Pair< `[`ae::Str128](DataStructures.md#classae_1_1_str), [ae::Str128`](DataStructures.md#classae_1_1_str)` > * `[`end`](#classae_1_1_dict_1aae5e6f2dd796b15630a1581311fa24c1)`()` 

#### `public inline const ae::Pair< `[`ae::Str128](DataStructures.md#classae_1_1_str), [ae::Str128`](DataStructures.md#classae_1_1_str)` > * `[`begin`](#classae_1_1_dict_1a3241208bf62ea34e19a1af2929c147bd)`() const` 

#### `public inline const ae::Pair< `[`ae::Str128](DataStructures.md#classae_1_1_str), [ae::Str128`](DataStructures.md#classae_1_1_str)` > * `[`end`](#classae_1_1_dict_1a144da167f0b7545623098ceaa5eb15d8)`() const` 

<br></br>
# `ae::List` class 

#### `public  `[`List`](#classae_1_1_list_1adacd7cba952464c0fae66c027ea68491)`()` 

#### `public  `[`~List`](#classae_1_1_list_1aedff939dc68feb893a3e7541a9917f6a)`()` 

#### `public void `[`Append`](#classae_1_1_list_1aaea43ef8b2ffc17cca2787f463274cdd)`(ListNode< T > & node)` 

#### `public void `[`Clear`](#classae_1_1_list_1a9ae3525763ff0160536f434922383a56)`()` 

#### `public T * `[`GetFirst`](#classae_1_1_list_1ad6547adaa202c9c3074dc31e35e3c031)`()` 

#### `public T * `[`GetLast`](#classae_1_1_list_1a50ba19ea3181fc1df0e01f68d8ba5580)`()` 

#### `public const T * `[`GetFirst`](#classae_1_1_list_1ac5c13a1ad760dbbbe6ca1e1981f72434)`() const` 

#### `public const T * `[`GetLast`](#classae_1_1_list_1a67d5e8479d237a508ecf594f60310e08)`() const` 

#### `public template<>`  <br/>`T * `[`Find`](#classae_1_1_list_1a8db8af0c0203fe7ac8927a11905a4ba6)`(const U & value)` 

#### `public template<>`  <br/>`T * `[`FindFn`](#classae_1_1_list_1a02ddb9810e60589c5457dc2a25acae45)`(Fn predicateFn)` 

#### `public uint32_t `[`Length`](#classae_1_1_list_1a4422227b0fd97919c963e9591a167b82)`() const` 

<br></br>
# `ae::ListNode` class 

#### `public  `[`ListNode`](#classae_1_1_list_node_1a3e191d36e25abb56ead40a796782b72f)`(T * owner)` 

#### `public  `[`~ListNode`](#classae_1_1_list_node_1af98ea9e83c76af9c8bc9b3c664505a73)`()` 

#### `public void `[`Remove`](#classae_1_1_list_node_1ae26f8f586d1e61c6377423a707d271c8)`()` 

#### `public T * `[`GetFirst`](#classae_1_1_list_node_1a658ec7f2a57b2287be75f4bfcc251fcb)`()` 

#### `public T * `[`GetNext`](#classae_1_1_list_node_1add88264ef373ddc03ab9e958cfdd179f)`()` 

#### `public T * `[`GetPrev`](#classae_1_1_list_node_1ab0461d5b21f07e6b2cbf8a2bd1285d80)`()` 

#### `public T * `[`GetLast`](#classae_1_1_list_node_1aff4caa45e65281d25845654016a6fcff)`()` 

#### `public const T * `[`GetFirst`](#classae_1_1_list_node_1a70b66189f951887a538aea933c3e289c)`() const` 

#### `public const T * `[`GetNext`](#classae_1_1_list_node_1a5577628490ed865ae45ea523b7f2153f)`() const` 

#### `public const T * `[`GetPrev`](#classae_1_1_list_node_1a6aa0e652849ca3281120a13845c3c06c)`() const` 

#### `public const T * `[`GetLast`](#classae_1_1_list_node_1a72b97b55e0350be70929a2095285ca19)`() const` 

#### `public List< T > * `[`GetList`](#classae_1_1_list_node_1a1bd8a0218c5f7067a7151d4d0af46e4a)`()` 

#### `public const List< T > * `[`GetList`](#classae_1_1_list_node_1a13f6973633a7421db8622a307992d1fe)`() const` 

<br></br>
# `ae::RingBuffer` class 

#### `public  `[`RingBuffer`](#classae_1_1_ring_buffer_1ade8c192022c2687c5a6cfa9a9c9ddd87)`()` 

Constructor for a ring buffer with static allocated storage (N > 0).

#### `public  `[`RingBuffer`](#classae_1_1_ring_buffer_1a7904a3a403b7d2ded0fa0fd400122f6b)`(ae::Tag tag,uint32_t size)` 

Constructor for a ring buffer with dynamically allocated storage (N == 0).

#### `public T & `[`Append`](#classae_1_1_ring_buffer_1a2ffb3049611fd5482df9b86a6f44f0f3)`(const T & val)` 

Appends an element to the current end of the ring buffer.

It's safe to call this when the ring buffer is full, although in this case the element previously at index 0 to be destroyed.

#### `public void `[`Clear`](#classae_1_1_ring_buffer_1a1efe3e4aa066cbbfa40e8054b6b568a5)`()` 

Resets Length() to 0. Does not affect Size().

#### `public T & `[`Get`](#classae_1_1_ring_buffer_1a7b5c91910a1369eae16464931682ad8d)`(uint32_t index)` 

Returns the element at the given `index`, which must be less than Length().

#### `public const T & `[`Get`](#classae_1_1_ring_buffer_1a333765f086cc63050241291603d1b8be)`(uint32_t index) const` 

Returns the element at the given `index`, which must be less than Length().

#### `public inline uint32_t `[`Length`](#classae_1_1_ring_buffer_1a974d47ef2e04147b8a3490052cc60eba)`() const` 

Returns the number of appended entries up to Size().

#### `public inline _AE_DYNAMIC_STORAGE uint32_t `[`Size`](#classae_1_1_ring_buffer_1ab73ff260f6906a2812d59fa694660ed3)`(...) const` 

Returns the max number of entries.

<br></br>
# `ae::FreeList` class 

[ae::FreeList](#classae_1_1_free_list) can be used along side a separate data array to track allocated elements.

Given a size, [ae::FreeList](#classae_1_1_free_list) allows allocation and release of array indices from 0 to size - 1.

#### `public  `[`FreeList`](#classae_1_1_free_list_1a1315d059819eacfbd0831a33acfb03b8)`()` 

#### `public  `[`FreeList`](#classae_1_1_free_list_1a7d2d161eabbd2ae96fe6394783d74199)`(const ae::Tag & tag,uint32_t size)` 

#### `public int32_t `[`Allocate`](#classae_1_1_free_list_1a92098374833f1c8e2662a9cb6da9be36)`()` 

Returns (0 <= index < N) on success, and negative on failure.

#### `public void `[`Free`](#classae_1_1_free_list_1ab9ea9b9065bde71cfcd9cd626a5a68be)`(int32_t idx)` 

Releases `idx` for future calls to [ae::FreeList::Allocate()](DataStructures.md#classae_1_1_free_list_1a92098374833f1c8e2662a9cb6da9be36).

`idx` must be an allocated index or negative (a result of [ae::FreeList::Allocate()](DataStructures.md#classae_1_1_free_list_1a92098374833f1c8e2662a9cb6da9be36) failure).

#### `public void `[`FreeAll`](#classae_1_1_free_list_1a18be8cdcb87ea338354d44902f6fd9b7)`()` 

Frees all allocated indices.

#### `public int32_t `[`GetFirst`](#classae_1_1_free_list_1a6ca90ebfb49bae28157c320f2cf55b8a)`() const` 

Returns the index of the first allocated object.

Returns a negative value if there are no allocated objects.

#### `public int32_t `[`GetNext`](#classae_1_1_free_list_1a042e8a10f725641b68b64d1ad5fe2e34)`(int32_t idx) const` 

Returns the index of the next allocated object after `idx`.

Returns a negative value if there are no more allocated objects. `idx` must be an allocated index or negative. A negative value will be returned if `idx` is negative.

#### `public bool `[`IsAllocated`](#classae_1_1_free_list_1a608a3bec224d1a9714f4d9c1d69c7162)`(int32_t idx) const` 

Returns true if the given `idx` is currently allocated.

`idx` must be negative or less than N.

#### `public bool `[`HasFree`](#classae_1_1_free_list_1ab07af6cbe335c3e128aba7243dda63f0)`() const` 

Returns true if the next [Allocate()](DataStructures.md#classae_1_1_free_list_1a92098374833f1c8e2662a9cb6da9be36) will succeed.

#### `public uint32_t `[`Length`](#classae_1_1_free_list_1a7dea55e2c1fa845097cf672b857bfbc4)`() const` 

Returns the number of allocated elements.

#### `public inline _AE_DYNAMIC_STORAGE uint32_t `[`Size`](#classae_1_1_free_list_1a899323117b203863fe2a4fc98fc7ce8a)`(...) const` 

Returns the maximum length of the list.

<br></br>
# `ae::ObjectPool` class 

#### `public  `[`ObjectPool`](#classae_1_1_object_pool_1a1ab8c06df5eb09eb22f17ec2a81a8da3)`()` 

Constructor for static ae::ObjectPool's only.

#### `public  `[`ObjectPool`](#classae_1_1_object_pool_1a3964bfb77964b7bad5901831a00d3697)`(const ae::Tag & tag)` 

Constructor for paged ae::ObjectPool's only.

#### `public  `[`~ObjectPool`](#classae_1_1_object_pool_1a8b2568c4e37df409bf2522bf4d391118)`()` 

All objects allocated with [ae::ObjectPool::New()](undefined.md#group___allocation_1ga2e2d7eb9a9db2cb2bbc2e4f438cf0c17) must be destroyed before the ae::ObjectPool is destroyed.

#### `public T * `[`New`](#classae_1_1_object_pool_1a26ee101302949a0e78b605e9a25eb54e)`()` 

Returns a pointer to a freshly constructed object T or null if there are no free objects.

Call [ae::ObjectPool::Delete()](undefined.md#group___allocation_1ga0f2fa464d37ebeea843e6d3fb4efb650) to destroy the object. [ae::ObjectPool::Delete()](undefined.md#group___allocation_1ga0f2fa464d37ebeea843e6d3fb4efb650) must be called on every object returned by [ae::ObjectPool::New()](undefined.md#group___allocation_1ga2e2d7eb9a9db2cb2bbc2e4f438cf0c17).

#### `public void `[`Delete`](#classae_1_1_object_pool_1a0d36a0ae25976c3bff1f51f951ea2896)`(T * obj)` 

Destructs and releases the object `obj` for future use by [ae::ObjectPool::New()](undefined.md#group___allocation_1ga2e2d7eb9a9db2cb2bbc2e4f438cf0c17).

It is safe for the `obj` parameter to be null.

#### `public void `[`DeleteAll`](#classae_1_1_object_pool_1af25dd0787ad3a7e39f4e9f355ff614f7)`()` 

Destructs and releases all objects for future use by [ae::ObjectPool::New()](undefined.md#group___allocation_1ga2e2d7eb9a9db2cb2bbc2e4f438cf0c17).

#### `public const T * `[`GetFirst`](#classae_1_1_object_pool_1ab752651b0bcc0895327a8adc245a62ef)`() const` 

Returns the first allocated object in the pool or null if the pool is empty.

#### `public const T * `[`GetNext`](#classae_1_1_object_pool_1ab8024787046eefe46005a4167308378f)`(const T * obj) const` 

Returns the next allocated object after `obj` or null if there are no more objects.

Null will be returned if `obj` is null.

#### `public T * `[`GetFirst`](#classae_1_1_object_pool_1a6f0f14eceb91dc83da97266c9d2022c7)`()` 

Returns the first allocated object in the pool or null if the pool is empty.

#### `public T * `[`GetNext`](#classae_1_1_object_pool_1a6799306ef4bdc53d86a1b09c37628afc)`(T * obj)` 

Returns the next allocated object after `obj` or null if there are no more objects.

Null will be returned if `obj` is null.

#### `public bool `[`HasFree`](#classae_1_1_object_pool_1acdc7731ca40a16cf7c7e489d589d6208)`() const` 

Returns true if the pool has any unallocated objects available.

This always returns true for paged pools.

#### `public uint32_t `[`Length`](#classae_1_1_object_pool_1a27f80dc8c484d0b855a816953e312942)`() const` 

Returns the number of allocated objects.

#### `public inline _AE_PAGED_POOL uint32_t `[`Size`](#classae_1_1_object_pool_1a59f1814460687c894d2bbefd4d67619e)`(...) const` 

Returns the total number of objects in the pool.

<br></br>
# `ae::OpaquePool` class 

[ae::OpaquePool](#classae_1_1_opaque_pool) is useful for dynamically allocating memory for many object instances when the object type is not known at compile time.

It's particularly useful in conjunction with ae::Type (see constructor for more info on this). It's possible to iterate over an [ae::OpaquePool](#classae_1_1_opaque_pool) by calling [ae::OpaquePool::Iterate< T >](DataStructures.md#classae_1_1_opaque_pool_1a4641b57d91454699986880a0db9837cd) (where the template parameter is required because [ae::OpaquePool](#classae_1_1_opaque_pool) is not templated itself). Additionally, a static ae::Map of [ae::OpaquePool](#classae_1_1_opaque_pool)'s is a great way to allocate game objects or components loaded from a level file.

#### `public  `[`OpaquePool`](#classae_1_1_opaque_pool_1a55f80241e55217fb33fe43c4f4fe9f0d)`(const ae::Tag & tag,uint32_t objectSize,uint32_t objectAlignment,uint32_t poolSize,bool paged)` 

Constructs an [ae::OpaquePool](#classae_1_1_opaque_pool) with dynamic internal storage.

`tag` will be used for all internal allocations. All objects returned by the pool will have `objectSize` and `objectAlignment`. If the pool is `paged` it will allocate pages of size `poolSize` as necessary. If the pool is not `paged`, then `objects` can be allocated at a time. It may be useful to use this in conjunction with registered ae::Type's, passing the results of ae::Type::GetSize() to `objectSize` and ae::Type::GetAlignment() to `objectAlignment`.

#### `public  `[`~OpaquePool`](#classae_1_1_opaque_pool_1a87676908d40e4cc3b47d68e1eca51322)`()` 

All objects allocated with [ae::OpaquePool::Allocate](DataStructures.md#classae_1_1_opaque_pool_1a9ac38680caba14115bff18c428153e05)/New() must be destroyed before the [ae::OpaquePool](#classae_1_1_opaque_pool) is destroyed.

#### `public template<>`  <br/>`T * `[`New`](#classae_1_1_opaque_pool_1ab038794c5cd98bc9e08feaef5e4c28da)`()` 

Returns a pointer to a freshly constructed object T.

If the pool is not paged and there are no free objects null will be returned. Call [ae::OpaquePool::Delete()](DataStructures.md#classae_1_1_opaque_pool_1af83ca92d8ff8d2100465aa51b59db7b9) to destroy the object. [ae::OpaquePool::Delete()](DataStructures.md#classae_1_1_opaque_pool_1af83ca92d8ff8d2100465aa51b59db7b9) must be called on every object returned by [ae::OpaquePool::New()](DataStructures.md#classae_1_1_opaque_pool_1ab038794c5cd98bc9e08feaef5e4c28da), although it is safe to mix calls to [ae::OpaquePool::Allocate](DataStructures.md#classae_1_1_opaque_pool_1a9ac38680caba14115bff18c428153e05)/New() and [ae::OpaquePool::Free](DataStructures.md#classae_1_1_opaque_pool_1a877ce435a17d0544a320ed58ab60088c)/Delete() as long as constructors and destructors are called manually with [ae::OpaquePool::Allocate()](DataStructures.md#classae_1_1_opaque_pool_1a9ac38680caba14115bff18c428153e05) and [ae::OpaquePool::Free()](DataStructures.md#classae_1_1_opaque_pool_1a877ce435a17d0544a320ed58ab60088c).

#### `public template<>`  <br/>`void `[`Delete`](#classae_1_1_opaque_pool_1af83ca92d8ff8d2100465aa51b59db7b9)`(T * obj)` 

Destructs and releases the object `obj` for future use. It is safe for `obj` to be null.

#### `public template<>`  <br/>`void `[`DeleteAll`](#classae_1_1_opaque_pool_1a29c5f7b4564efd01c57da2b5ed8259f1)`()` 

Destructs and releases all objects for future use.

#### `public void * `[`Allocate`](#classae_1_1_opaque_pool_1a9ac38680caba14115bff18c428153e05)`()` 

Returns a pointer to an object.

If the pool is not paged and there are no free objects null will be returned. The user is responsible for any constructor calls. [ae::OpaquePool::Free()](DataStructures.md#classae_1_1_opaque_pool_1a877ce435a17d0544a320ed58ab60088c) must be called on every object returned by [ae::OpaquePool::Allocate()](DataStructures.md#classae_1_1_opaque_pool_1a9ac38680caba14115bff18c428153e05). It is safe to mix calls to [ae::OpaquePool::Allocate](DataStructures.md#classae_1_1_opaque_pool_1a9ac38680caba14115bff18c428153e05)/New() and [ae::OpaquePool::Free](DataStructures.md#classae_1_1_opaque_pool_1a877ce435a17d0544a320ed58ab60088c)/Delete() as long as constructors and destructors are called manually with [ae::OpaquePool::Allocate()](DataStructures.md#classae_1_1_opaque_pool_1a9ac38680caba14115bff18c428153e05) and [ae::OpaquePool::Free()](DataStructures.md#classae_1_1_opaque_pool_1a877ce435a17d0544a320ed58ab60088c).

#### `public void `[`Free`](#classae_1_1_opaque_pool_1a877ce435a17d0544a320ed58ab60088c)`(void * obj)` 

Releases the object `obj` for future use. It is safe for `obj` to be null.

#### `public void `[`FreeAll`](#classae_1_1_opaque_pool_1ab7f6d944a6f491f5b09f9350cfde6153)`()` 

Releases all objects for future use by [ae::OpaquePool::Allocate()](DataStructures.md#classae_1_1_opaque_pool_1a9ac38680caba14115bff18c428153e05).

THIS FUNCTION DOES NOT CALL THE OBJECTS DESTRUCTORS, so please use with caution!

#### `public bool `[`HasFree`](#classae_1_1_opaque_pool_1a3e7dad10631de5b3051a39e2bd52c3c9)`() const` 

Returns true if the pool has any unallocated objects available.

#### `public inline uint32_t `[`Length`](#classae_1_1_opaque_pool_1a4082e54171e79f34ae8bf58d7c7244ad)`() const` 

Returns the number of allocated objects.

#### `public inline uint32_t `[`Size`](#classae_1_1_opaque_pool_1a129b5c0564d9ab91119a5a7c766d7e2c)`() const` 

Returns the total number of objects in the pool.

Note that this number can grow and shrink for paged pools.

#### `public inline uint32_t `[`PageSize`](#classae_1_1_opaque_pool_1ac651296fae85e7e6b5c83a01899185ad)`() const` 

Returns the maximum number of objects per page.

#### `public template<>`  <br/>[`Iterator`](undefined.md#classae_1_1_opaque_pool_1_1_iterator)`< T > `[`Iterate`](#classae_1_1_opaque_pool_1a4641b57d91454699986880a0db9837cd)`()` 

Returns an [ae::OpaquePool::Iterator](undefined.md#classae_1_1_opaque_pool_1_1_iterator) which is stl conformant.

#### `public template<>`  <br/>[`Iterator`](undefined.md#classae_1_1_opaque_pool_1_1_iterator)`< const T > `[`Iterate`](#classae_1_1_opaque_pool_1acbaa567648a3b08e4d41c52676c21147)`() const` 

Returns an [ae::OpaquePool::Iterator](undefined.md#classae_1_1_opaque_pool_1_1_iterator) which is stl conformant.

<br></br>
# `ae::Pair` struct 

#### `public K `[`key`](#structae_1_1_pair_1ad5f4b7d8200474ca1ae6d316baa6bde3) 

#### `public V `[`value`](#structae_1_1_pair_1a2f3df713a0f760fc7e60af92bfdc0964) 

#### `public inline  `[`Pair`](#structae_1_1_pair_1adae043fcbf3e507f91c9ae9424aab518)`(const K & k,const V & v)` 

