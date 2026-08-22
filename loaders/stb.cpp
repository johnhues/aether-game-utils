//------------------------------------------------------------------------------
// stb.cpp
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
// Headers
//------------------------------------------------------------------------------
#include "ae/loaders.h"
AE_PUSH_DISABLE_ALL_WARNINGS()
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
AE_POP_DISABLE_ALL_WARNINGS()

namespace ae
{

//------------------------------------------------------------------------------
// stbLoadPng
//------------------------------------------------------------------------------
bool stbLoadPng( ae::Texture2D* texture, const uint8_t* data, uint32_t dataLen, ae::Texture::Filter filter, ae::Texture::Wrap wrap, bool autoGenerateMipmaps, bool isSRGB )
{
	if( !texture || !data || !dataLen )
	{
		return false;
	}
	stbi_set_flip_vertically_on_load( 1 );
#if _AE_IOS_
	stbi_convert_iphone_png_to_rgb( 1 );
#endif

	uint8_t* image = nullptr;
	int32_t width = 0;
	int32_t height = 0;
	int32_t channels = 0;
	if( !stbi_info_from_memory( data, dataLen, &width, &height, &channels ) )
	{
		return false;
	}
	const bool is16BitImage = stbi_is_16_bit_from_memory( data, dataLen );
	// Grey+alpha has no matching texture format, so expand it to RGBA on load
	const int32_t reqChannels = ( channels == STBI_grey_alpha ) ? STBI_rgb_alpha : channels;
	// @TODO: For now 16 bit images only support R16Unorm
	if( is16BitImage && reqChannels != STBI_grey )
	{
		return false;
	}
	if( is16BitImage )
	{
		image = (uint8_t*)stbi_load_16_from_memory( data, dataLen, &width, &height, &channels, reqChannels );
	}
	else
	{
		image = stbi_load_from_memory( data, dataLen, &width, &height, &channels, reqChannels );
	}
	if( !image )
	{
		return false;
	}
	const ae::RunOnDestroy cleanupSTBImage( [ image ]() { stbi_image_free( image ); } );

	ae::Texture::Format format;
	ae::Texture::Type type;
	switch( reqChannels )
	{
		case STBI_grey:
		{
			if( is16BitImage )
			{
				format = ae::Texture::Format::R16_UNORM;
				type = ae::Texture::Type::UInt16;
			}
			else
			{
				format = ae::Texture::Format::R8;
				type = ae::Texture::Type::UInt8;
			}
			break;
		}
		case STBI_rgb:
		{
			format = isSRGB ? ae::Texture::Format::RGB8_SRGB : ae::Texture::Format::RGB8;
			type = ae::Texture::Type::UInt8;
			break;
		}
		case STBI_rgb_alpha:
		{
			format = isSRGB ? ae::Texture::Format::RGBA8_SRGB : ae::Texture::Format::RGBA8;
			type = ae::Texture::Type::UInt8;
			break;
		}
		default:
		{
			texture->Terminate();
			return false;
		}
	}
	// @TODO: Return value from texture->Initialize() to indicate failure
	texture->Initialize( image, width, height, format, type, filter, wrap, autoGenerateMipmaps );
	return true;
}

} // End ae namespace
