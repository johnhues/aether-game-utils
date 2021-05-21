//------------------------------------------------------------------------------
// aeAudio.h
// Platform specific defines
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
#ifndef AEAUDIO_H
#define AEAUDIO_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeArray.h"
#include "aeString.h"

//------------------------------------------------------------------------------
// aeAudioData class
//------------------------------------------------------------------------------
class aeAudioData
{
public:
  aeAudioData();
  void Initialize( const char* filePath );
  void Destroy();

  aeStr64 name;
  uint32_t buffer;
  float length;
};

//------------------------------------------------------------------------------
// aeAudio class
//------------------------------------------------------------------------------
class aeAudio
{
public:
  void Initialize( uint32_t musicChannels, uint32_t sfxChannels );
  void Terminate();

  void SetVolume( float volume );

  void PlayMusic( const aeAudioData* audioFile, float volume, uint32_t channel );
  void PlaySfx( const aeAudioData* audioFile, float volume, int32_t priority ); // @NOTE: Lower priority values interrupt sfx with higher values

  void StopMusic( uint32_t channel );
  void StopAllSfx();

  uint32_t GetMusicChannelCount() const;
  uint32_t GetSfxChannelCount() const;

  void Log();

private:
  struct aeAudioChannel
  {
    aeAudioChannel();
    uint32_t source;
    int32_t priority;
    const aeAudioData* resource;
  };

  ae::Array< aeAudioChannel > m_musicChannels;
  ae::Array< aeAudioChannel > m_sfxChannels;
};

#endif
