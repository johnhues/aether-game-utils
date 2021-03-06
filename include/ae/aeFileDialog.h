//------------------------------------------------------------------------------
// aeFileDialog.h
// Utilities for allocating objects. Provides functionality to track current and
// past allocations.
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
// @NOTE: These functions currently do not handle misbehaving shell extensions
// well on Windows. In particular p4bridge had to be disabled on my system due to
// a crash upon hovering over folders. If you run into an issue that you are
// unable to resolve while using these functions, try setting 'useLegacy' to true.
//------------------------------------------------------------------------------
#ifndef AEFILEDIALOG_H
#define AEFILEDIALOG_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
class aeWindow;
namespace AE_NAMESPACE {
#if !_AE_APPLE_ // No std::path support

//------------------------------------------------------------------------------
// OpenFile
//------------------------------------------------------------------------------
struct OpenFileParams
{
  const char* windowTitle = "Open";
  Array< FileFilter > filters = AE_ALLOC_TAG_FILE; // Leave empty for { ae::FileFilter( "All Files", "*" ) }

  aeWindow* window = nullptr; // Recommended. Setting this will create a modal 'Open' dialog.
  const char* defaultPath = "";
  // @TODO: allowMultiselect
  bool useLegacy = false;
};
Array< std::string > OpenFile( const OpenFileParams& params ); // See note at top of header

//------------------------------------------------------------------------------
// SaveFile
//------------------------------------------------------------------------------
struct SaveFileParams
{
  const char* windowTitle = "Save As";
  Array< FileFilter > filters = AE_ALLOC_TAG_FILE; // Leave empty for { ae::FileFilter( "All Files", "*" ) }

  aeWindow* window = nullptr; // Recommended. Setting this will create a modal 'Save As' dialog.
  const char* defaultPath = "";
  bool confirmOverwrite = true;
  bool useLegacy = false;
};
std::string SaveFile( const SaveFileParams& params ); // See note at top of header

#endif

} // AE_NAMESPACE end

#endif
