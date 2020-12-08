//------------------------------------------------------------------------------
// aeAudio.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2020 John Hughes
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
#include "aeAudio.h"
#include "aeLog.h"

#if _AE_APPLE_
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
void CheckALError()
{
  const char* errStr = "UNKNOWN_ERROR";
  switch ( alGetError() )
  {
  case AL_NO_ERROR: return;
  case AL_INVALID_NAME: errStr = "AL_INVALID_NAME"; break;
  case AL_INVALID_ENUM: errStr = "AL_INVALID_ENUM"; break;
  case AL_INVALID_VALUE: errStr = "AL_INVALID_VALUE"; break;
  case AL_INVALID_OPERATION: errStr = "AL_INVALID_OPERATION"; break;
  case AL_OUT_OF_MEMORY: errStr = "AL_OUT_OF_MEMORY"; break;
  default: break;
  }
  AE_LOG( "OpenAL Error: #", errStr );
  AE_FAIL();
}

void LoadWavFile( const char* filename, ALuint* buffer, float* length )
{
  struct ChunkHeader
  {
    char chunkId[ 4 ];
    uint32_t chunkSize;
  };

  struct FormatChunk
  {
    uint16_t formatCode;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    uint16_t dwChannelMask;
    uint8_t subformat[ 16 ];
  };

  struct RiffChunk
  {
    char waveId[ 4 ];
  };

  FormatChunk wave_format;
  bool hasReadFormat = false;
  uint32_t dataSize = 0;

  FILE* soundFile = fopen( filename, "rb" );
  AE_ASSERT_MSG( soundFile, "Could not open wav file: #", filename );

  ChunkHeader header;
  fread( &header, sizeof( header ), 1, soundFile );
  while ( !feof( soundFile ) )
  {
    if ( memcmp( header.chunkId, "RIFF", 4 ) == 0 )
    {
      RiffChunk riff;
      fread( &riff, sizeof( riff ), 1, soundFile );
      AE_ASSERT( memcmp( riff.waveId, "WAVE", 4 ) == 0 );
    }
    else if ( memcmp( header.chunkId, "fmt ", 4 ) == 0 )
    {
      fread( &wave_format, header.chunkSize, 1, soundFile );
      hasReadFormat = true;
    }
    else if ( memcmp( header.chunkId, "data", 4 ) == 0 )
    {
      AE_ASSERT( hasReadFormat );
      AE_ASSERT_MSG( dataSize == 0, "Combining WAV data chunks is currently not supported" );

      uint8_t* data = new uint8_t[ header.chunkSize ];
      fread( data, header.chunkSize, 1, soundFile );

      ALsizei size = header.chunkSize;
      ALsizei frequency = wave_format.sampleRate;
      dataSize = size;

      ALenum format;
      if ( wave_format.numChannels == 1 )
      {
        if ( wave_format.bitsPerSample == 8 ) { format = AL_FORMAT_MONO8; }
        else if ( wave_format.bitsPerSample == 16 ) { format = AL_FORMAT_MONO16; }
      }
      else if ( wave_format.numChannels == 2 )
      {
        if ( wave_format.bitsPerSample == 8 ) { format = AL_FORMAT_STEREO8; }
        else if ( wave_format.bitsPerSample == 16 ) { format = AL_FORMAT_STEREO16; }
      }
      alGenBuffers( 1, buffer );
      alBufferData( *buffer, format, (void*)data, size, frequency );
      delete[] data;
    }
    else
    {
      fseek( soundFile, header.chunkSize, SEEK_CUR );
    }

    fread( &header, sizeof( header ), 1, soundFile );
  }

  fclose( soundFile );

  CheckALError();

  AE_ASSERT( hasReadFormat );
  AE_ASSERT( dataSize );

  *length = dataSize / ( wave_format.sampleRate * wave_format.numChannels * wave_format.bitsPerSample / 8.0f );
}

//------------------------------------------------------------------------------
// aeAudioData member functions
//------------------------------------------------------------------------------
aeAudioData::aeAudioData()
{
  name = "";
  buffer = 0;
  length = 0.0f;
}

void aeAudioData::Initialize( const char* filePath )
{
  AE_ASSERT( !buffer );
  this->name = filePath; // @TODO: Should just be file name or file hash
  LoadWavFile( filePath, &this->buffer, &this->length );
  AE_ASSERT( buffer );
}

void aeAudioData::Destroy()
{
  AE_ASSERT( buffer );

  alDeleteBuffers( 1, &this->buffer );
  CheckALError();

  *this = aeAudioData();
}

//------------------------------------------------------------------------------
// aeAudio member functions
//------------------------------------------------------------------------------
void aeAudio::Initialize( uint32_t musicChannels, uint32_t sfxChannels )
{
  ALCdevice* device = alcOpenDevice( nullptr );
  AE_ASSERT( device );
  ALCcontext* ctx = alcCreateContext( device, nullptr );
  alcMakeContextCurrent( ctx );
  AE_ASSERT( ctx );
  CheckALError();
	
  aeArray< ALuint > sources( musicChannels + sfxChannels, 0 );
  alGenSources( (ALuint)sources.Length(), &sources[ 0 ] );

  m_musicChannels.Reserve( musicChannels );
  for ( uint32_t i = 0; i < musicChannels; i++ )
  {
    aeAudioChannel* channel = &m_musicChannels.Append( aeAudioChannel() );
    channel->source = sources[ i ];
    alGenSources( (ALuint)1, &channel->source );
    alSourcef( channel->source, AL_PITCH, 1 );
    alSourcef( channel->source, AL_GAIN, 1.0f );
    alSource3f( channel->source, AL_POSITION, 0, 0, 0 );
    alSourcei( channel->source, AL_LOOPING, AL_TRUE );
  }

  m_sfxChannels.Reserve( sfxChannels );
  for ( uint32_t i = 0; i < sfxChannels; i++ )
  {
    aeAudioChannel* channel = &m_sfxChannels.Append( aeAudioChannel() );
    channel->source = sources[ musicChannels + i ];
    alSourcef( channel->source, AL_PITCH, 1 );
    alSourcef( channel->source, AL_GAIN, 1.0f );
    alSource3f( channel->source, AL_POSITION, 0, 0, 0 );
    alSourcei( channel->source, AL_LOOPING, AL_FALSE );
  }

  ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
  alListener3f( AL_POSITION, 0, 0, 1.0f );
  alListenerfv( AL_ORIENTATION, listenerOri );
  alListenerf( AL_GAIN, 1.0f );

  CheckALError();
}

void aeAudio::Terminate()
{
  for ( uint32_t i = 0; i < m_musicChannels.Length(); i++ )
  {
    aeAudioChannel* channel = &m_musicChannels[ i ];
    alDeleteSources( 1, &channel->source );
    channel->source = -1;
  }

  for ( uint32_t i = 0; i < m_sfxChannels.Length(); i++ )
  {
    aeAudioChannel* channel = &m_sfxChannels[ i ];
    alDeleteSources( 1, &channel->source );
    channel->source = -1;
  }

  ALCcontext* ctx = alcGetCurrentContext();
  ALCdevice* device = alcGetContextsDevice( ctx );
  alcMakeContextCurrent( nullptr );
  alcDestroyContext( ctx );
  alcCloseDevice( device );
}

void aeAudio::SetVolume( float volume )
{
  volume = aeMath::Clip01( volume );
  alListenerf( AL_GAIN, volume );
}

void aeAudio::PlayMusic( const aeAudioData* audioFile, float volume, uint32_t channel )
{
  AE_ASSERT( audioFile );
  if ( channel >= m_musicChannels.Length() )
  {
    return;
  }

  aeAudioChannel* musicChannel = &m_musicChannels[ channel ];

  ALint state;
  alGetSourcei( musicChannel->source, AL_SOURCE_STATE, &state );
  if ( ( audioFile == musicChannel->resource ) && state == AL_PLAYING )
  {
    return;
  }

  if ( state == AL_PLAYING )
  {
    alSourceStop( musicChannel->source );
  }

  alSourcei( musicChannel->source, AL_BUFFER, audioFile->buffer );
  alSourcef( musicChannel->source, AL_GAIN, volume );
  alSourcePlay( musicChannel->source );
  CheckALError();
}

void aeAudio::PlaySfx( const aeAudioData* audioFile, float volume, int32_t priority )
{
  ALint state;
  AE_ASSERT( audioFile );

  aeAudioChannel* leastPriorityChannel = nullptr;
  aeAudioChannel* unusedChannel = nullptr;
  for ( uint32_t i = 0; i < m_sfxChannels.Length(); i++ )
  {
    aeAudioChannel* sfxChannel = &m_sfxChannels[ i ];
    if ( !leastPriorityChannel || sfxChannel->priority >= leastPriorityChannel->priority )
    {
      leastPriorityChannel = sfxChannel;
    }

    if ( !unusedChannel )
    {
      alGetSourcei( sfxChannel->source, AL_SOURCE_STATE, &state );
      if ( state != AL_PLAYING )
      {
        unusedChannel = sfxChannel;
      }
    }
  }
  AE_ASSERT( leastPriorityChannel );

  aeAudioChannel* currentChannel = nullptr;
  if ( unusedChannel )
  {
    currentChannel = unusedChannel;
  }
  else if ( !leastPriorityChannel || leastPriorityChannel->priority < priority )
  {
    return;
  }
  else
  {
    currentChannel = leastPriorityChannel;
  }
  AE_ASSERT( currentChannel );

  alSourceStop( currentChannel->source );
  alGetSourcei( currentChannel->source, AL_SOURCE_STATE, &state );
  AE_ASSERT( state != AL_PLAYING );

  currentChannel->resource = audioFile;

  alSourcei( currentChannel->source, AL_BUFFER, audioFile->buffer );
  alSourcef( currentChannel->source, AL_GAIN, volume );
  alSourcePlay( currentChannel->source );
  CheckALError();
  currentChannel->priority = priority;

  alGetSourcei( currentChannel->source, AL_SOURCE_STATE, &state );
}

void aeAudio::StopMusic( uint32_t channel )
{
  if ( channel < m_musicChannels.Length() )
  {
    alSourceStop( m_musicChannels[ channel ].source );
  }
}

void aeAudio::StopAllSfx()
{
  for ( uint32_t i = 0; i < m_sfxChannels.Length(); i++ )
  {
    alSourceStop( m_sfxChannels[ i ].source );
  }
}

uint32_t aeAudio::GetMusicChannelCount() const
{
  return m_musicChannels.Length();
}

uint32_t aeAudio::GetSfxChannelCount() const
{
  return m_sfxChannels.Length();
}

// @TODO: Should return a string with current state of audio channels
void aeAudio::Log()
{
  for ( uint32_t i = 0; i < m_sfxChannels.Length(); i++ )
  {
    ALint state = 0;
    const aeAudioChannel* channel = &m_sfxChannels[ i ];
    alGetSourcei( channel->source, AL_SOURCE_STATE, &state );

    if ( state == AL_PLAYING )
    {
      AE_ASSERT( channel->resource );

      float playOffset = 0.0f;
      alGetSourcef( channel->source, AL_SEC_OFFSET, &playOffset );

      float playLength = channel->resource->length;

      const char* soundName = strrchr( channel->resource->name.c_str(), '/' );
      soundName = soundName ? soundName + 1 : channel->resource->name.c_str();
      const char* soundNameEnd = strrchr( channel->resource->name.c_str(), '.' );
      soundNameEnd = soundNameEnd ? soundNameEnd : soundName + strlen( soundName );
      uint32_t soundNameLen = soundNameEnd - soundName;

      char buffer[ 512 ];
      sprintf( buffer, "channel:%u name:%.*s offset:%.2fs length:%.2fs", i, soundNameLen, soundName, playOffset, playLength );
      AE_LOG( buffer );
    }
  }

  CheckALError();
}
