# `Allocation` 

Allocation utilities. By default aether-game-utils uses system allocations (malloc / free), which may be fine for your use case. If not, it's advised that you implement your own ae::Allocator with dlmalloc or similar and then call [ae::SetGlobalAllocator()](undefined.md#group___allocation_1gab3cfe88cd853bbae077508261275e696) with your allocator at program start. All allocations are tagged, (@TODO) they can be inspected through the current ae::Allocator with [ae::GetGlobalAllocator()](undefined.md#group___allocation_1gac7aca866927c0cf9ae42b0e7bd10b3dd).

## Members

#### `public void `[`SetGlobalAllocator`](#group___allocation_1gab3cfe88cd853bbae077508261275e696)`(Allocator * alloc)` 

The given ae::Allocator is used for all memory allocations.

You must call [ae::SetGlobalAllocator()](undefined.md#group___allocation_1gab3cfe88cd853bbae077508261275e696) before any allocations are made or else a default allocator which uses malloc / free will be used. The set value can be retrieved with [ae::GetGlobalAllocator()](undefined.md#group___allocation_1gac7aca866927c0cf9ae42b0e7bd10b3dd).

#### `public Allocator * `[`GetGlobalAllocator`](#group___allocation_1gac7aca866927c0cf9ae42b0e7bd10b3dd)`()` 

Get the custom allocator set with [ae::SetGlobalAllocator()](undefined.md#group___allocation_1gab3cfe88cd853bbae077508261275e696).

If no custom allocator is set before the first allocation is made, this will return a default ae::Allocator which uses malloc / free. If [ae::SetGlobalAllocator()](undefined.md#group___allocation_1gab3cfe88cd853bbae077508261275e696) has never been called and no allocations have been made, this will return nullptr.

#### `public template<>`  <br/>`T * `[`NewArray`](#group___allocation_1gad6000f13b14be2eeac7a9eb910877440)`(ae::Tag tag,uint32_t count)` 

Allocates and constructs an array of 'count' elements of type T.

an ae::Tag must be specifed and should represent the allocation type. Type T must have a default constructor. All arrays allocated with this function should be freed with [ae::Delete()](undefined.md#group___allocation_1ga0f2fa464d37ebeea843e6d3fb4efb650). Uses [ae::GetGlobalAllocator()](undefined.md#group___allocation_1gac7aca866927c0cf9ae42b0e7bd10b3dd) and ae::Allocator::Allocate() internally.

#### `public template<>`  <br/>`T * `[`New`](#group___allocation_1ga2e2d7eb9a9db2cb2bbc2e4f438cf0c17)`(ae::Tag tag,Args ... args)` 

Allocates and constructs a single element of type T.

an ae::Tag must be specified and should represent the allocation type. All 'args' are passed to the constructor of T. All allocations should be freed with [ae::Delete()](undefined.md#group___allocation_1ga0f2fa464d37ebeea843e6d3fb4efb650). Uses [ae::GetGlobalAllocator()](undefined.md#group___allocation_1gac7aca866927c0cf9ae42b0e7bd10b3dd) and ae::Allocator::Allocate() internally.

#### `public template<>`  <br/>`void `[`Delete`](#group___allocation_1ga0f2fa464d37ebeea843e6d3fb4efb650)`(T * obj)` 

Should be called to destruct and free all allocations made with [ae::New()](undefined.md#group___allocation_1ga2e2d7eb9a9db2cb2bbc2e4f438cf0c17) and [ae::NewArray()](undefined.md#group___allocation_1gad6000f13b14be2eeac7a9eb910877440).

Uses [ae::GetGlobalAllocator()](undefined.md#group___allocation_1gac7aca866927c0cf9ae42b0e7bd10b3dd) and ae::Allocator::Free() internally.

#### `public inline void * `[`Allocate`](#group___allocation_1gaf86f02f7cadcac721a444986a4c62d9e)`(ae::Tag tag,uint32_t bytes,uint32_t alignment)` 

#### `public inline void * `[`Reallocate`](#group___allocation_1ga0f35d5fdf33c03397d5875c842f609f1)`(void * data,uint32_t bytes,uint32_t alignment)` 

#### `public inline void `[`Free`](#group___allocation_1gae749688512d651ecbbb8ace65264b275)`(void * data)` 

<br></br>
# `ae::Allocator` class 

#### `public virtual  `[`~Allocator`](#classae_1_1_allocator_1a6757fe133450a17b800a15632cc81e89)`()` 

#### `public void * `[`Allocate`](#classae_1_1_allocator_1a4d89f4311161fcdb94a7cc41f581f1b8)`(ae::Tag tag,uint32_t bytes,uint32_t alignment)` 

Should return 'bytes' with minimum alignment of 'alignment'.

Optionally, a tag should be used to select a pool of memory, or for diagnostics/debugging.

#### `public void * `[`Reallocate`](#classae_1_1_allocator_1a4b356c4ce27794b577bb94cf8a3e1faa)`(void * data,uint32_t bytes,uint32_t alignment)` 

Should attempt to expand or contract allocations made with Allocate() to match size 'bytes'.

On failure this function should return nullptr.

#### `public void `[`Free`](#classae_1_1_allocator_1a3c167e398d7ca93973419c40a5ee3bed)`(void * data)` 

Free memory allocated with ae::Allocator::Allocate() or ae::Allocator::Reallocate().

#### `public bool `[`IsThreadSafe`](#classae_1_1_allocator_1aeece6dfbe99a22d95fb6b4b9d248f59b)`() const` 

Used for safety checks.

