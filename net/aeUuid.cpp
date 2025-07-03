// Above GUID, something includes a 'RGB()' macro which conflicts with ae
#include "ae/aeUuid.h"
#include "aether.h"
#include <inttypes.h>

/*
The MIT License (MIT)

Copyright (c) 2014 Graeme Hill (http://graemehill.ca)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <iomanip>

#ifdef __linux__
	#define GUID_LIBUUID
#endif

#ifdef __APPLE__
	#define GUID_CFUUID
#endif

#ifdef _MSC_VER
	#define GUID_WINDOWS
#endif

#ifdef __EMSCRIPTEN__
	#define GUID_LIBUUID
#endif

#ifdef GUID_LIBUUID
	#include <uuid/uuid.h>
#endif

#ifdef GUID_CFUUID
	#include <CoreFoundation/CFUUID.h>
#endif

#ifdef GUID_WINDOWS
	#include <objbase.h>
#endif

#ifdef GUID_ANDROID
	#include <jni.h>
#endif

// Class to represent a GUID/UUID. Each instance acts as a wrapper around a
// 16 byte value that can be passed around by value. It also supports
// conversion to string (via the stream operator <<) and conversion from a
// string via constructor.
class Guid
{
	public:

		// create a guid from vector of bytes
		Guid(const std::vector<unsigned char> &bytes);

		// create a guid from array of bytes
		Guid(const unsigned char *bytes);

		// create a guid from string
		Guid(const std::string &fromString);

		// create empty guid
		Guid();

		// copy constructor
		Guid(const Guid &other);

		// overload assignment operator
		Guid &operator=(const Guid &other);

		// overload equality and inequality operator
		bool operator==(const Guid &other) const;
		bool operator!=(const Guid &other) const;
		
		// actual data
		std::vector<unsigned char> _bytes;

		// make the << operator a friend so it can access _bytes
		friend std::ostream &operator<<(std::ostream &s, const Guid &guid);
};

// Class that can create new guids. The only reason this exists instead of
// just a global "newGuid" function is because some platforms will require
// that there is some attached context. In the case of android, we need to
// know what JNIEnv is being used to call back to Java, but the newGuid()
// function would no longer be cross-platform if we parameterized the android
// version. Instead, construction of the GuidGenerator may be different on
// each platform, but the use of newGuid is uniform.
class GuidGenerator
{
	public:
#ifdef GUID_ANDROID
		GuidGenerator(JNIEnv *env);
#else
		GuidGenerator() { }
#endif

		Guid newGuid();

#ifdef GUID_ANDROID
	private:
		JNIEnv *_env;
		jclass _uuidClass;
		jmethodID _newGuidMethod;
		jmethodID _mostSignificantBitsMethod;
		jmethodID _leastSignificantBitsMethod;
#endif
};

// overload << so that it's easy to convert to a string
std::ostream &operator<<(std::ostream &s, const Guid &guid)
{
	return s << std::hex << std::setfill('0')
		<< std::setw(2) << (int)guid._bytes[0]
		<< std::setw(2) << (int)guid._bytes[1]
		<< std::setw(2) << (int)guid._bytes[2]
		<< std::setw(2) << (int)guid._bytes[3]
		<< "-"
		<< std::setw(2) << (int)guid._bytes[4]
		<< std::setw(2) << (int)guid._bytes[5]
		<< "-"
		<< std::setw(2) << (int)guid._bytes[6]
		<< std::setw(2) << (int)guid._bytes[7]
		<< "-"
		<< std::setw(2) << (int)guid._bytes[8]
		<< std::setw(2) << (int)guid._bytes[9]
		<< "-"
		<< std::setw(2) << (int)guid._bytes[10]
		<< std::setw(2) << (int)guid._bytes[11]
		<< std::setw(2) << (int)guid._bytes[12]
		<< std::setw(2) << (int)guid._bytes[13]
		<< std::setw(2) << (int)guid._bytes[14]
		<< std::setw(2) << (int)guid._bytes[15];
}

// create a guid from vector of bytes
Guid::Guid(const std::vector<unsigned char> &bytes)
{
	_bytes = bytes;
}

// create a guid from array of bytes
Guid::Guid(const unsigned char *bytes)
{
	_bytes.assign(bytes, bytes + 16);
}

// converts a single hex char to a number (0 - 15)
unsigned char hexDigitToChar(char ch)
{
	if(ch > 47 && ch < 58)
		return ch - 48;

	if(ch > 96 && ch < 103)
		return ch - 87;

	if(ch > 64 && ch < 71)
		return ch - 55;

	return 0;
}

// converts the two hexadecimal characters to an unsigned char (a byte)
unsigned char hexPairToChar(char a, char b)
{
	return hexDigitToChar(a) * 16 + hexDigitToChar(b);
}

// create a guid from string
Guid::Guid(const std::string &fromString)
{
	_bytes.clear();

	char charOne, charTwo;
	bool lookingForFirstChar = true;

	for(const char &ch : fromString)
	{
		if(ch == '-')
			continue;

		if(lookingForFirstChar)
		{
			charOne = ch;
			lookingForFirstChar = false;
		}
		else
		{
			charTwo = ch;
			auto byte = hexPairToChar(charOne, charTwo);
			_bytes.push_back(byte);
			lookingForFirstChar = true;
		}
	}

}

// create empty guid
Guid::Guid()
{
	_bytes = std::vector<unsigned char>(16, 0);
}

// copy constructor
Guid::Guid(const Guid &other)
{
	_bytes = other._bytes;
}

// overload assignment operator
Guid &Guid::operator=(const Guid &other)
{
	_bytes = other._bytes;
	return *this;
}

// overload equality operator
bool Guid::operator==(const Guid &other) const
{
	return _bytes == other._bytes;
}

// overload inequality operator
bool Guid::operator!=(const Guid &other) const
{
	return !((*this) == other);
}

// This is the linux friendly implementation, but it could work on other
// systems that have libuuid available
#ifdef GUID_LIBUUID
Guid GuidGenerator::newGuid()
{
#ifdef __chip__
	const int byteArray[4] =
	{
		rand(),
		rand(),
		rand(),
		rand()
	};
	return (unsigned char*)byteArray;
#else
	uuid_t id;
	uuid_generate(id);
	return id;
#endif
}
#endif

// this is the mac and ios version 
#ifdef GUID_CFUUID
Guid GuidGenerator::newGuid()
{
	auto newId = CFUUIDCreate(NULL);
	auto bytes = CFUUIDGetUUIDBytes(newId);
	CFRelease(newId);

	const unsigned char byteArray[16] =
	{
		bytes.byte0,
		bytes.byte1,
		bytes.byte2,
		bytes.byte3,
		bytes.byte4,
		bytes.byte5,
		bytes.byte6,
		bytes.byte7,
		bytes.byte8,
		bytes.byte9,
		bytes.byte10,
		bytes.byte11,
		bytes.byte12,
		bytes.byte13,
		bytes.byte14,
		bytes.byte15
	};
	return byteArray;
}
#endif

// obviously this is the windows version
#ifdef GUID_WINDOWS
Guid GuidGenerator::newGuid()
{
	GUID newId;
	CoCreateGuid(&newId);

	const unsigned char bytes[16] = 
	{
		(uint8_t)( (newId.Data1 >> 24) & 0xFF ),
		(uint8_t)( (newId.Data1 >> 16) & 0xFF ),
		(uint8_t)( (newId.Data1 >> 8) & 0xFF ),
		(uint8_t)( (newId.Data1) & 0xff ),

		(uint8_t)( (newId.Data2 >> 8) & 0xFF ),
		(uint8_t)( (newId.Data2) & 0xff ),

		(uint8_t)( (newId.Data3 >> 8) & 0xFF ),
		(uint8_t)( (newId.Data3) & 0xFF ),

		newId.Data4[0],
		newId.Data4[1],
		newId.Data4[2],
		newId.Data4[3],
		newId.Data4[4],
		newId.Data4[5],
		newId.Data4[6],
		newId.Data4[7]
	};

	return bytes;
}
#endif

// android version that uses a call to a java api
#ifdef GUID_ANDROID
GuidGenerator::GuidGenerator(JNIEnv *env)
{
	_env = env;
	_uuidClass = env->FindClass("java/util/UUID");
	_newGuidMethod = env->GetStaticMethodID(_uuidClass, "randomUUID", "()Ljava/util/UUID;");
	_mostSignificantBitsMethod = env->GetMethodID(_uuidClass, "getMostSignificantBits", "()J");
	_leastSignificantBitsMethod = env->GetMethodID(_uuidClass, "getLeastSignificantBits", "()J");
}

Guid GuidGenerator::newGuid()
{
	jobject javaUuid = _env->CallStaticObjectMethod(_uuidClass, _newGuidMethod);
	jlong mostSignificant = _env->CallLongMethod(javaUuid, _mostSignificantBitsMethod);
	jlong leastSignificant = _env->CallLongMethod(javaUuid, _leastSignificantBitsMethod);

	unsigned char bytes[16] = 
	{
		(mostSignificant >> 56) & 0xFF,
		(mostSignificant >> 48) & 0xFF,
		(mostSignificant >> 40) & 0xFF,
		(mostSignificant >> 32) & 0xFF,
		(mostSignificant >> 24) & 0xFF,
		(mostSignificant >> 16) & 0xFF,
		(mostSignificant >> 8) & 0xFF,
		(mostSignificant) & 0xFF,
		(leastSignificant >> 56) & 0xFF,
		(leastSignificant >> 48) & 0xFF,
		(leastSignificant >> 40) & 0xFF,
		(leastSignificant >> 32) & 0xFF,
		(leastSignificant >> 24) & 0xFF,
		(leastSignificant >> 16) & 0xFF,
		(leastSignificant >> 8) & 0xFF,
		(leastSignificant) & 0xFF,
	};
	return bytes;
}
#endif

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
// AetherUuid member functions
//------------------------------------------------------------------------------
AetherUuid::AetherUuid( const char* str )
{
#define _aescn8 "%2" SCNx8
	sscanf( str, _aescn8 _aescn8 _aescn8 _aescn8 "-"
		_aescn8 _aescn8 "-" _aescn8 _aescn8 "-" _aescn8 _aescn8 "-"
		_aescn8 _aescn8 _aescn8 _aescn8 _aescn8 _aescn8,
		&uuid[0], &uuid[1], &uuid[2], &uuid[3],
		&uuid[4], &uuid[5], &uuid[6], &uuid[7], &uuid[8], &uuid[9],
		&uuid[10], &uuid[11], &uuid[12], &uuid[13], &uuid[14], &uuid[15] );
#undef _aescn8
}

AetherUuid AetherUuid::Generate()
{
	GuidGenerator gen;
	Guid localUuid = gen.newGuid();
	AE_ASSERT( localUuid._bytes.size() == 16 );

	AetherUuid result;
	memcpy( result.uuid, localUuid._bytes.data(), 16 );
	return result;
}

AetherUuid AetherUuid::Zero()
{
	AetherUuid result;
	memset( &result, 0, sizeof(result) );
	return result;
}

void AetherUuid::ToString( char* str, uint32_t max ) const
{
	AE_ASSERT( max >= 37 );
	snprintf( str, max, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", 
		uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
		uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15] );
}

std::ostream& operator<<( std::ostream& os, const AetherUuid& uuid )
{
	char str[ 64 ];
	uuid.ToString( str, countof(str) );
	return os << str;
}
