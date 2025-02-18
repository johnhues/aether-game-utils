//------------------------------------------------------------------------------
// aeUuid.h
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
#ifndef AEUUID_H
#define AEUUID_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

//------------------------------------------------------------------------------
// AetherUuid class
//------------------------------------------------------------------------------
struct AetherUuid
{
	AetherUuid() = default;
	AetherUuid( const char* str );
	
	bool operator==( const AetherUuid& other ) const { return memcmp( uuid, other.uuid, 16 ) == 0; }
	bool operator!=( const AetherUuid& other ) const { return memcmp( uuid, other.uuid, 16 ) != 0; }

	static AetherUuid Generate();
	static AetherUuid Zero();

	void ToString( char* str, uint32_t max ) const;

	uint8_t uuid[ 16 ];
};
namespace ae { template <> inline uint32_t GetHash( AetherUuid e )
{
	return ae::Hash().HashData( e.uuid, sizeof(e.uuid) ).Get();
} }

std::ostream& operator<<( std::ostream& os, const AetherUuid& uuid );

inline void Serialize( ae::BinaryStream* stream, AetherUuid* uuid )
{
	stream->SerializeRaw( uuid->uuid, sizeof( uuid->uuid ) );
}

inline void Serialize( ae::BinaryWriter* stream, const AetherUuid* uuid )
{
	stream->SerializeRaw( uuid->uuid, sizeof( uuid->uuid ) );
}

#endif
