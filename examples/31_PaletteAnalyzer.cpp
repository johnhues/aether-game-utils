//------------------------------------------------------------------------------
// 31_PaletteAnalyzer.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2026 John Hughes
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
// A live sixteen slot palette bench. Panels follow DawnBringer's GrafX2 palette
// analysis: hue-brightness scatter, brightness range coverage, register
// occupancy, and the pairwise wash matrix. Measurements are CIELAB, so hue
// spacing reflects what the eye reports rather than what the encoding does.
//
// Given arguments it runs headless instead, remapping images in bulk:
//
//   31_PaletteAnalyzer <scheme> <image.png>... <output dir or .png>
//   31_PaletteAnalyzer Aether4 ~/Pictures/KodakTrueColor/kodim*.png ./out
//
// Each image is written as <base>_<scheme>.png. Stitching is left to the
// caller, so a three way comparison against the original is:
//
//   magick a.png original.png b.png +append out.png
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
#include "ae/aeImGui.h"
#include "imgui_internal.h" // DockBuilder
#include <cmath>
#include <cctype>
#include <thread>
#include <atomic>
#include "../loaders/stb_image.h"
#include "imgui.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_ALL = "all";
const uint32_t kSlotCount = 16;
const uint32_t kSlotColumns = 8;
//! Panels are fixed in place. Only the dock splitters move, so the layout can
//! be resized but not rearranged, torn out, tabbed or closed.
const ImGuiWindowFlags kPanelFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
//! Opens a panel and labels it. A docked node with no tab bar has no title bar,
//! so the name would not appear anywhere otherwise.
static void BeginPanel( const char* name );
const ImGuiDockNodeFlags kLeafFlags = ImGuiDockNodeFlags_NoTabBar
	| ImGuiDockNodeFlags_NoDocking | ImGuiDockNodeFlags_NoUndocking
	| ImGuiDockNodeFlags_NoCloseButton | ImGuiDockNodeFlags_NoWindowMenuButton;
//! Chroma below this reads as grey, so a slot under it is counted as a neutral
//! and excluded from hue spacing.
const float kNeutralChroma = 12.0f;
//! DawnBringer builds register by register. Bounds are L* band edges.
const float kRegisterEdges[ 3 ] = { 25.0f, 50.0f, 75.0f };
//! Procedural test sheet. Bands stack vertically, each drawn as source over
//! mapped so the loss is read by looking down rather than side to side.
const uint32_t kTestWidth = 320;
const uint32_t kTestBand = 44;
const uint32_t kTestFieldHeight = 80;
const uint32_t kTestSphereHeight = 84;
const uint32_t kTestCheckerCell = 32;
const uint32_t kTestHeight = kTestBand * 4 + kTestFieldHeight * 2 + kTestSphereHeight
	+ kTestCheckerCell * 4;


//------------------------------------------------------------------------------
// Schemes
//------------------------------------------------------------------------------
//! One named sixteen slot palette, ordered by ascending lightness so the 8x2
//! grid reads as a dark register above a bright one. To add a scheme, paste a
//! block below and add
//! it to \p kSchemes. The analyzer's "Copy as C++" button emits exactly this
//! format, so a palette tweaked in the tool pastes straight back in as a new
//! scheme and can be compared against the others without touching ae::Color.
struct SchemeColor
{
	const char* name;
	uint8_t r, g, b;
};

struct Scheme
{
	const char* name;
	SchemeColor colors[ kSlotCount ];
};

//! The 24 ColorChecker patches in sRGB. Six of these are "memory colours" the
//! chart exists to test - two skin tones, foliage, blue sky, blue flower - and
//! they are the ones a limited palette usually fails first.
const SchemeColor kColorChecker[ 24 ] = {
	{ "dark skin", 115, 82, 68 },    { "light skin", 194, 150, 130 },
	{ "blue sky", 98, 122, 157 },    { "foliage", 87, 108, 67 },
	{ "blue flower", 133, 128, 177 },{ "bluish green", 103, 189, 170 },
	{ "orange", 214, 126, 44 },      { "purplish blue", 80, 91, 166 },
	{ "moderate red", 193, 90, 99 }, { "purple", 94, 60, 108 },
	{ "yellow green", 157, 188, 64 },{ "orange yellow", 224, 163, 46 },
	{ "blue", 56, 61, 150 },         { "green", 70, 148, 73 },
	{ "red", 175, 54, 60 },          { "yellow", 231, 199, 31 },
	{ "magenta", 187, 86, 149 },     { "cyan", 8, 133, 161 },
	{ "white", 243, 243, 242 },      { "neutral 8", 200, 200, 200 },
	{ "neutral 6.5", 160, 160, 160 },{ "neutral 5", 122, 122, 121 },
	{ "neutral 3.5", 85, 85, 85 },   { "black", 52, 52, 52 },
};

const Scheme kScheme_Aether = { "ae::Color", {
	{ "Black", 45, 45, 45 },        { "DarkRed", 175, 65, 90 },
	{ "DarkGray", 105, 105, 100 },  { "Purple", 120, 90, 195 },
	{ "Blue", 70, 120, 225 },       { "Red", 240, 75, 90 },
	{ "Gray", 145, 135, 130 },      { "Orange", 255, 150, 60 },
	{ "Teal", 90, 195, 185 },       { "Yellow", 250, 205, 100 },
	{ "Green", 180, 240, 80 },      { "White", 235, 230, 215 },
	{ "-", 45, 45, 45 },            { "-", 45, 45, 45 },
	{ "-", 45, 45, 45 },            { "-", 45, 45, 45 },
} };

const Scheme kScheme_Aether2 = { "Aether2", {
	{ "Floor", 31, 30, 27 },
	{ "Maroon", 95, 0, 0 },
	{ "Cold", 55, 61, 66 },
	{ "Pine", 0, 76, 69 },
	{ "Wine", 121, 18, 118 },
	{ "Indigo", 30, 76, 144 },
	{ "Crimson", 189, 49, 81 },
	{ "Olive", 121, 109, 0 },
	{ "Warm", 144, 136, 127 },
	{ "Coral", 249, 87, 90 },
	{ "Rust", 246, 153, 80 },
	{ "Orchid", 255, 178, 255 },
	{ "Cyan", 0, 224, 255 },
	{ "Amber", 255, 201, 0 },
	{ "White", 218, 213, 200 },
	{ "Fern", 114, 251, 121 },
} };

const Scheme kScheme_Aether3 = { "Aether3", {
	{ "Plum", 34, 23, 59 },
	{ "Maroon", 88, 23, 19 },
	{ "Ocean", 0, 52, 65 },
	{ "Indigo", 29, 48, 81 },
	{ "Wine", 97, 54, 93 },
	{ "Moss", 46, 98, 41 },
	{ "Blue", 71, 87, 119 },
	{ "Crimson", 184, 56, 82 },
	{ "Olive", 118, 106, 82 },
	{ "Fern", 93, 140, 45 },
	{ "Violet", 104, 143, 201 },
	{ "Coral", 189, 154, 146 },
	{ "Rust", 255, 127, 68 },
	{ "Cyan", 109, 195, 218 },
	{ "Lilac", 245, 181, 255 },
	{ "Amber", 243, 203, 113 },
} };

const Scheme kScheme_Aether4 = { "Aether4", {
	{ "Plum", 34, 23, 59 },
	{ "Maroon", 88, 23, 19 },
	{ "Floor", 62, 41, 40 },
	{ "Indigo", 29, 48, 81 },
	{ "Wine", 97, 54, 93 },
	{ "Moss", 73, 92, 68 },
	{ "Umber", 116, 81, 63 },
	{ "Rust", 133, 101, 79 },
	{ "Red", 204, 53, 63 },
	{ "Orange", 151, 127, 115 },
	{ "Fern", 138, 147, 105 },
	{ "Peach", 208, 132, 72 },
	{ "Peach2", 255, 127, 68 },
	{ "Cyan", 138, 173, 184 },
	{ "Warm", 189, 171, 157 },
	{ "Amber", 243, 203, 113 },
} };

const Scheme kScheme_Aether5 = { "Aether5", {
	{ "Wine", 56, 39, 55 },
	{ "Indigo", 29, 48, 81 },
	{ "Maroon", 78, 41, 39 },
	{ "Magenta", 97, 54, 93 },
	{ "Moss", 73, 92, 68 },
	{ "Umber", 116, 81, 63 },
	{ "Rust", 133, 101, 79 },
	{ "Red", 192, 67, 69 },
	{ "Sky", 98, 128, 153 },
	{ "Orange", 151, 127, 115 },
	{ "Fern", 138, 147, 105 },
	{ "Peach", 226, 133, 79 },
	{ "Cyan", 138, 173, 184 },
	{ "Warm", 189, 171, 157 },
	{ "Amber", 243, 203, 113 },
	{ "Green", 252, 251, 215 },
} };

const Scheme kScheme_Aether6 = { "Aether6", {
	{ "Wine", 40, 25, 39 },
	{ "Indigo", 29, 48, 81 },
	{ "Maroon", 78, 41, 39 },
	{ "Magenta", 96, 55, 92 },
	{ "Moss", 67, 94, 61 },
	{ "Navy", 53, 92, 110 },
	{ "Umber", 116, 81, 63 },
	{ "Rust", 133, 101, 79 },
	{ "Red", 192, 67, 69 },
	{ "Orange", 151, 127, 115 },
	{ "Fern", 135, 149, 88 },
	{ "Peach", 220, 136, 87 },
	{ "Cyan", 111, 177, 196 },
	{ "Warm", 189, 171, 157 },
	{ "Amber", 243, 203, 113 },
	{ "Green", 252, 251, 215 },
} };

const Scheme kScheme_Aether7 = { "Aether7", {
	{ "Wine", 40, 25, 39 },
	{ "Blood", 78, 41, 41 },
	{ "Indigo", 26, 57, 91 },
	{ "Magenta", 96, 55, 92 },
	{ "Umber", 115, 81, 62 },
	{ "Moss", 82, 93, 55 },
	{ "Navy", 59, 110, 136 },
	{ "Rust", 138, 102, 83 },
	{ "Crimson", 192, 66, 76 },
	{ "Orange", 156, 132, 120 },
	{ "Fern", 127, 152, 61 },
	{ "Peach", 231, 130, 70 },
	{ "Rose", 247, 116, 125 },
	{ "Cyan", 133, 174, 186 },
	{ "Amber", 252, 199, 115 },
	{ "Yellow", 255, 243, 212 },
} };

const Scheme kScheme_Aether8 = { "Aether8", {
	{ "Floor", 34, 26, 41 },
	{ "Blood", 78, 41, 41 },
	{ "Purple", 74, 56, 93 },
	{ "Umber", 114, 80, 61 },
	{ "Moss", 82, 93, 55 },
	{ "Pine", 50, 96, 99 },
	{ "Rust", 136, 101, 82 },
	{ "Crimson", 182, 73, 80 },
	{ "Fern", 127, 152, 61 },
	{ "Orange", 163, 138, 126 },
	{ "Rose", 247, 116, 125 },
	{ "Cyan", 79, 191, 204 },
	{ "Peach", 255, 151, 101 },
	{ "Amber", 238, 195, 107 },
	{ "Lilac", 233, 189, 255 },
	{ "White", 255, 232, 217 },
} };

const Scheme kScheme_Pico8 = { "PICO-8", {
	{ "black", 0, 0, 0 },           { "dark-blue", 29, 43, 83 },
	{ "dark-purple", 126, 37, 83 }, { "dark-green", 0, 135, 81 },
	{ "brown", 171, 82, 54 },       { "dark-grey", 95, 87, 79 },
	{ "light-grey", 194, 195, 199 },{ "white", 255, 241, 232 },
	{ "red", 255, 0, 77 },          { "orange", 255, 163, 0 },
	{ "yellow", 255, 236, 39 },     { "green", 0, 228, 54 },
	{ "blue", 41, 173, 255 },       { "lavender", 131, 118, 156 },
	{ "pink", 255, 119, 168 },      { "peach", 255, 204, 170 },
} };

const Scheme kScheme_DawnBringer16 = { "DawnBringer 16", {
	{ "db00", 20, 12, 28 },         { "db01", 68, 36, 52 },
	{ "db02", 48, 52, 109 },        { "db03", 78, 74, 78 },
	{ "db04", 133, 76, 48 },        { "db05", 52, 101, 36 },
	{ "db06", 208, 70, 72 },        { "db07", 117, 113, 97 },
	{ "db08", 89, 125, 206 },       { "db09", 210, 125, 44 },
	{ "db10", 133, 149, 161 },      { "db11", 109, 170, 44 },
	{ "db12", 210, 170, 153 },      { "db13", 109, 194, 202 },
	{ "db14", 218, 212, 94 },       { "db15", 222, 238, 214 },
} };

const Scheme kScheme_Aether11 = { "Aether11", {
	{ "Plum", 34, 23, 59 },
	{ "Maroon", 88, 23, 19 },
	{ "Floor", 62, 41, 40 },
	{ "Indigo", 29, 48, 81 },
	{ "Wine", 97, 54, 93 },
	{ "Moss", 73, 92, 68 },
	{ "Umber", 116, 81, 63 },
	{ "Rust", 133, 101, 79 },
	{ "Red", 204, 53, 63 },
	{ "Orange", 151, 127, 115 },
	{ "Fern", 138, 147, 105 },
	{ "Ember", 255, 127, 68 },
	{ "Cyan", 138, 173, 184 },
	{ "Warm", 189, 171, 157 },
	{ "Amber", 243, 203, 113 },
	{ "White", 255, 232, 217 },
} };

const Scheme kScheme_Aether10 = { "Aether10", {
	{ "Plum", 34, 23, 59 },
	{ "Maroon", 88, 23, 19 },
	{ "Floor", 62, 41, 40 },
	{ "Indigo", 29, 48, 81 },
	{ "Wine", 97, 54, 93 },
	{ "Moss", 73, 92, 68 },
	{ "Umber", 116, 81, 63 },
	{ "Rust", 133, 101, 79 },
	{ "Red", 204, 53, 63 },
	{ "Orange", 151, 127, 115 },
	{ "Fern", 138, 147, 105 },
	{ "Peach", 208, 132, 72 },
	{ "Ember", 255, 127, 68 },
	{ "Warm", 189, 171, 157 },
	{ "Amber", 243, 203, 113 },
	{ "White", 255, 232, 217 },
} };

const Scheme kScheme_Aether9 = { "Aether9", {
	{ "Plum", 34, 23, 59 },
	{ "Maroon", 88, 23, 19 },
	{ "Indigo", 29, 48, 81 },
	{ "Wine", 97, 54, 93 },
	{ "Moss", 73, 92, 68 },
	{ "Umber", 116, 81, 63 },
	{ "Rust", 133, 101, 79 },
	{ "Red", 204, 53, 63 },
	{ "Orange", 151, 127, 115 },
	{ "Fern", 138, 147, 105 },
	{ "Peach", 208, 132, 72 },
	{ "Ember", 255, 127, 68 },
	{ "Cyan", 138, 173, 184 },
	{ "Warm", 189, 171, 157 },
	{ "Amber", 243, 203, 113 },
	{ "White", 255, 232, 217 },
} };

const Scheme kScheme_Aether12 = { "Aether12", {
	{ "Black", 24, 7, 18 },
	{ "DarkRed", 88, 23, 19 },
	{ "SkinDark", 62, 41, 40 },
	{ "DarkBlue", 29, 48, 81 },
	{ "DarkPurple", 97, 54, 93 },
	{ "DarkGreen", 73, 92, 68 },
	{ "SkinMediumDark", 116, 81, 63 },
	{ "SkinMedium", 133, 101, 79 },
	{ "Red", 204, 53, 63 },
	{ "SkinMediumLight", 151, 127, 115 },
	{ "Green", 138, 147, 103 },
	{ "Orange", 255, 127, 68 },
	{ "Blue", 113, 177, 196 },
	{ "SkinLight", 202, 163, 164 },
	{ "Yellow", 243, 203, 113 },
	{ "White", 255, 243, 245 },
} };



//! Hardware palettes, for reference. Sorted ascending by lightness like the
//! rest. Several of these have more than one accepted sRGB conversion; these
//! are the commonly cited ones - C64 is Pepto, TMS9918 the standard MSX set.
const Scheme kScheme_C64 = { "Commodore 64", {
	{ "black", 0, 0, 0 },           { "blue", 53, 40, 121 },
	{ "brown", 67, 57, 0 },         { "dk-grey", 68, 68, 68 },
	{ "red", 104, 55, 43 },         { "purple", 111, 61, 134 },
	{ "orange", 111, 79, 37 },      { "lt-blue", 108, 94, 181 },
	{ "grey", 108, 108, 108 },      { "lt-red", 154, 103, 89 },
	{ "green", 88, 141, 67 },       { "lt-grey", 149, 149, 149 },
	{ "cyan", 112, 164, 178 },      { "yellow", 184, 199, 111 },
	{ "lt-green", 154, 210, 132 },  { "white", 255, 255, 255 },
} };

const Scheme kScheme_Vic20 = { "VIC-20", {
	{ "black", 0, 0, 0 },           { "blue", 64, 49, 141 },
	{ "red", 120, 41, 34 },         { "purple", 170, 95, 182 },
	{ "lt-blue", 128, 113, 204 },   { "lt-red", 184, 105, 98 },
	{ "orange", 170, 116, 73 },     { "green", 85, 160, 73 },
	{ "lt-purple", 234, 159, 246 }, { "lt-orange", 234, 180, 137 },
	{ "yellow", 191, 206, 114 },    { "cyan", 135, 214, 221 },
	{ "lt-green", 148, 224, 137 },  { "lt-yellow", 240, 243, 150 },
	{ "lt-cyan", 199, 255, 255 },   { "white", 255, 255, 255 },
} };

const Scheme kScheme_Spectrum = { "ZX Spectrum", {
	{ "black", 0, 0, 0 },           { "br-black", 0, 0, 0 },
	{ "blue", 0, 0, 215 },          { "br-blue", 0, 0, 255 },
	{ "red", 215, 0, 0 },           { "magenta", 215, 0, 215 },
	{ "br-red", 255, 0, 0 },        { "br-magenta", 255, 0, 255 },
	{ "green", 0, 215, 0 },         { "cyan", 0, 215, 215 },
	{ "yellow", 215, 215, 0 },      { "white", 215, 215, 215 },
	{ "br-green", 0, 255, 0 },      { "br-cyan", 0, 255, 255 },
	{ "br-yellow", 255, 255, 0 },   { "br-white", 255, 255, 255 },
} };

const Scheme kScheme_Ega = { "EGA", {
	{ "black", 0, 0, 0 },           { "blue", 0, 0, 170 },
	{ "red", 170, 0, 0 },           { "dk-grey", 85, 85, 85 },
	{ "magenta", 170, 0, 170 },     { "brown", 170, 85, 0 },
	{ "lt-blue", 85, 85, 255 },     { "lt-red", 255, 85, 85 },
	{ "green", 0, 170, 0 },         { "cyan", 0, 170, 170 },
	{ "lt-magenta", 255, 85, 255 }, { "lt-grey", 170, 170, 170 },
	{ "lt-green", 85, 255, 85 },    { "lt-cyan", 85, 255, 255 },
	{ "lt-yellow", 255, 255, 85 },  { "white", 255, 255, 255 },
} };

const Scheme kScheme_Intellivision = { "Intellivision", {
	{ "black", 12, 0, 5 },          { "brown", 60, 88, 0 },
	{ "blue", 0, 45, 255 },         { "dk-green", 0, 120, 15 },
	{ "purple", 200, 26, 125 },     { "red", 255, 61, 16 },
	{ "pink", 255, 50, 118 },       { "green", 0, 167, 32 },
	{ "grey", 167, 168, 168 },      { "lt-blue", 189, 172, 200 },
	{ "yel-green", 108, 205, 48 },  { "orange", 255, 166, 0 },
	{ "cyan", 90, 203, 255 },       { "tan", 201, 212, 100 },
	{ "yellow", 250, 234, 39 },     { "white", 255, 252, 255 },
} };

const Scheme kScheme_Tms9918 = { "TMS9918 MSX", {
	{ "black", 0, 0, 0 },           { "dk-blue", 89, 85, 224 },
	{ "dk-red", 185, 94, 81 },      { "magenta", 183, 102, 181 },
	{ "lt-blue", 128, 118, 241 },   { "md-red", 219, 101, 89 },
	{ "dk-green", 58, 162, 65 },    { "md-green", 62, 184, 73 },
	{ "lt-red", 255, 137, 125 },    { "lt-green", 116, 208, 125 },
	{ "dk-yellow", 204, 195, 94 },  { "cyan", 101, 219, 239 },
	{ "grey", 204, 204, 204 },      { "lt-yellow", 222, 208, 135 },
	{ "white", 255, 255, 255 },     { "-", 0, 0, 0 },
} };

//! Every scheme the analyzer offers. Add new blocks here.
const Scheme* kSchemes[] = {
	&kScheme_Aether12,
	&kScheme_Aether11,
	&kScheme_Aether10,
	&kScheme_Aether9,
	&kScheme_Aether8,
	&kScheme_Aether7,
	&kScheme_Aether6,
	&kScheme_Aether5,
	&kScheme_Aether4,
	&kScheme_Aether3,
	&kScheme_Aether2,
	&kScheme_Aether,
	&kScheme_DawnBringer16,
	&kScheme_Pico8,
	&kScheme_C64,
	&kScheme_Vic20,
	&kScheme_Spectrum,
	&kScheme_Ega,
	&kScheme_Intellivision,
	&kScheme_Tms9918,
};

//------------------------------------------------------------------------------
// CIELAB conversion
//------------------------------------------------------------------------------
//! CIE D65 white point, matching the sRGB primaries ae::Color stores.
const ae::Vec3 kWhitePoint = ae::Vec3( 0.95047f, 1.0f, 1.08883f );

static float LabForward( float t )
{
	return ( t > 216.0f / 24389.0f ) ? std::cbrt( t ) : ( 24389.0f / 27.0f * t + 16.0f ) / 116.0f;
}

static float LabInverse( float t )
{
	const float cube = t * t * t;
	return ( cube > 216.0f / 24389.0f ) ? cube : ( 116.0f * t - 16.0f ) * 27.0f / 24389.0f;
}

//! Returns ( L*, C*, h ) with L* 0-100, C* 0-130ish, h in degrees 0-360.
static ae::Vec3 ColorToLCh( ae::Color color )
{
	const ae::Vec3 rgb = color.GetLinearRGB();
	const ae::Vec3 xyz(
		0.4124564f * rgb.x + 0.3575761f * rgb.y + 0.1804375f * rgb.z,
		0.2126729f * rgb.x + 0.7151522f * rgb.y + 0.0721750f * rgb.z,
		0.0193339f * rgb.x + 0.1191920f * rgb.y + 0.9503041f * rgb.z );
	const float fx = LabForward( xyz.x / kWhitePoint.x );
	const float fy = LabForward( xyz.y / kWhitePoint.y );
	const float fz = LabForward( xyz.z / kWhitePoint.z );
	const float lightness = 116.0f * fy - 16.0f;
	const float a = 500.0f * ( fx - fy );
	const float b = 200.0f * ( fy - fz );
	float hue = ae::RadToDeg( std::atan2( b, a ) );
	if( hue < 0.0f )
	{
		hue += 360.0f;
	}
	return ae::Vec3( lightness, std::sqrt( a * a + b * b ), hue );
}

//! Converts back to sRGB. \p inGamutOut receives false when the requested
//! chroma cannot be held at this lightness and hue, in which case the returned
//! color is clipped.
static ae::Color LChToColor( ae::Vec3 lch, bool* inGamutOut )
{
	const float radians = ae::DegToRad( lch.z );
	const float a = lch.y * std::cos( radians );
	const float b = lch.y * std::sin( radians );
	const float fy = ( lch.x + 16.0f ) / 116.0f;
	const ae::Vec3 xyz(
		LabInverse( fy + a / 500.0f ) * kWhitePoint.x,
		LabInverse( fy ) * kWhitePoint.y,
		LabInverse( fy - b / 200.0f ) * kWhitePoint.z );
	const ae::Vec3 rgb(
		3.2404542f * xyz.x - 1.5371385f * xyz.y - 0.4985314f * xyz.z,
		-0.9692660f * xyz.x + 1.8760108f * xyz.y + 0.0415560f * xyz.z,
		0.0556434f * xyz.x - 0.2040259f * xyz.y + 1.0572252f * xyz.z );
	if( inGamutOut )
	{
		const float low = ae::Min( rgb.x, rgb.y, rgb.z );
		const float high = ae::Max( rgb.x, rgb.y, rgb.z );
		*inGamutOut = ( low >= -0.0001f && high <= 1.0001f );
	}
	return ae::Color::RGB(
		ae::Clip01( rgb.x ), ae::Clip01( rgb.y ), ae::Clip01( rgb.z ) );
}

//! LCh to Lab, for distance comparisons where hue angle would wrap.
static ae::Vec3 LChToLab( ae::Vec3 lch )
{
	const float radians = ae::DegToRad( lch.z );
	return ae::Vec3( lch.x, lch.y * std::cos( radians ), lch.y * std::sin( radians ) );
}

//! Largest chroma sRGB can hold at this lightness and hue.
static float MaxChroma( float lightness, float hue )
{
	float low = 0.0f;
	float high = 150.0f;
	for( uint32_t i = 0; i < 40; i++ )
	{
		const float mid = ( low + high ) * 0.5f;
		bool inGamut = false;
		LChToColor( ae::Vec3( lightness, mid, hue ), &inGamut );
		( inGamut ? low : high ) = mid;
	}
	return low;
}

//! WCAG relative luminance, which weights the linear channels differently than
//! CIE Y and so disagrees slightly with L*.
static float RelativeLuminance( ae::Color color )
{
	const ae::Vec3 rgb = color.GetLinearRGB();
	return 0.2126f * rgb.x + 0.7152f * rgb.y + 0.0722f * rgb.z;
}

static float ContrastRatio( ae::Color lhs, ae::Color rhs )
{
	const float a = RelativeLuminance( lhs );
	const float b = RelativeLuminance( rhs );
	return ( ae::Max( a, b ) + 0.05f ) / ( ae::Min( a, b ) + 0.05f );
}

static void BeginPanel( const char* name )
{
	ImGui::Begin( name, nullptr, kPanelFlags );
	ImGui::TextDisabled( "%s", name );
	ImGui::Separator();
}

//------------------------------------------------------------------------------
// ColorGroup
//------------------------------------------------------------------------------
const uint32_t kMaxGroups = 8;
//! Slots that belong together as a ramp or a set. Membership is a bit per slot,
//! so it is tied to slot position rather than to a color: loading another
//! scheme keeps the groups and repoints them at whatever now sits there.
struct ColorGroup
{
	char name[ 24 ] = "";
	uint16_t members = 0;
	bool visible = true;

	uint32_t GetCount() const
	{
		uint32_t count = 0;
		for( uint32_t i = 0; i < 16; i++ )
		{
			count += ( members & ( 1 << i ) ) ? 1 : 0;
		}
		return count;
	}
};

//! Path a blend between two colors actually takes.
enum class BlendSpace
{
	LinearRGB, //!< what ae::Color::Lerp and the wash matrix do
	Lab,       //!< straight line through the solid, dips toward the axis
	LCh        //!< arc around the axis, holds chroma and sweeps hue
};

//! Maps a color into the cylinder: L* up the axis, chroma as radius, hue as
//! angle. Scaled so L*100 is 2 units tall and C*50 is 1 unit out.
static ae::Vec3 LChToCylinder( ae::Vec3 lch )
{
	const float radians = ae::DegToRad( lch.z );
	return ae::Vec3(
		lch.y * std::cos( radians ) * 0.02f,
		lch.y * std::sin( radians ) * 0.02f,
		lch.x * 0.02f );
}

//! Blends two colors along \p space and returns the result.
static ae::Color BlendColors( ae::Color from, ae::Color to, float t, BlendSpace space )
{
	if( space == BlendSpace::LinearRGB )
	{
		return from.Lerp( to, t );
	}
	const ae::Vec3 a = ColorToLCh( from );
	const ae::Vec3 b = ColorToLCh( to );
	if( space == BlendSpace::Lab )
	{
		const ae::Vec3 labA = LChToLab( a );
		const ae::Vec3 labB = LChToLab( b );
		const ae::Vec3 lab = labA + ( labB - labA ) * t;
		const float chroma = std::sqrt( lab.y * lab.y + lab.z * lab.z );
		float hue = ae::RadToDeg( std::atan2( lab.z, lab.y ) );
		hue = ( hue < 0.0f ) ? ( hue + 360.0f ) : hue;
		return LChToColor( ae::Vec3( lab.x, chroma, hue ), nullptr );
	}
	// Shortest way round, so a pair either side of 0 does not sweep the wheel.
	float hueDelta = std::fmod( b.z - a.z + 540.0f, 360.0f ) - 180.0f;
	return LChToColor( ae::Vec3(
		a.x + ( b.x - a.x ) * t,
		a.y + ( b.y - a.y ) * t,
		a.z + hueDelta * t ), nullptr );
}

//------------------------------------------------------------------------------
// Slot
//------------------------------------------------------------------------------
struct Slot
{
	ae::Color color = ae::Color::Black();
	char name[ 32 ] = "";
	//! Cached so dragging a point does not round trip through sRGB every frame,
	//! which would ratchet the value on colors sitting outside the gamut.
	ae::Vec3 lch = ae::Vec3( 0.0f );
	bool inGamut = true;

	void SetLCh( ae::Vec3 value )
	{
		value.z = ae::Mod( value.z, 360.0f );
		lch = value;
		color = LChToColor( value, &inGamut );
	}

	void SetColor( ae::Color value )
	{
		color = value;
		lch = ColorToLCh( value );
		inGamut = true;
	}
};

//! A slot named "-" is empty. It is drawn, but excluded from every measurement
//! and sorted after the real colors rather than by its lightness.
static bool IsEmptySlot( const Slot& slot )
{
	return ( slot.name[ 0 ] == '-' && slot.name[ 1 ] == 0 );
}

//------------------------------------------------------------------------------
// Metrics, recomputed every frame because sixteen slots is nothing
//------------------------------------------------------------------------------
struct Metrics
{
	float lightnessRange = 0.0f;
	float worstValueGap = 0.0f;
	float worstValueGapAt = 0.0f;
	float worstHueVoid = 0.0f;
	float worstHueVoidAt = 0.0f;
	float chromaSpread = 0.0f;
	uint32_t registerCounts[ 4 ] = { 0 };
	uint32_t chromaticCount = 0;
};

static Metrics MeasurePalette( const Slot* slots, uint32_t count )
{
	Metrics metrics;
	ae::Array< float, kSlotCount > lightness;
	ae::Array< float, kSlotCount > hues;
	float chromaLow = 1000.0f;
	float chromaHigh = 0.0f;
	for( uint32_t i = 0; i < count; i++ )
	{
		if( IsEmptySlot( slots[ i ] ) )
		{
			continue;
		}
		const ae::Vec3 lch = slots[ i ].lch;
		lightness.Append( lch.x );
		uint32_t band = 0;
		while( band < 3 && lch.x >= kRegisterEdges[ band ] )
		{
			band++;
		}
		metrics.registerCounts[ band ]++;
		if( lch.y >= kNeutralChroma )
		{
			hues.Append( lch.z );
			chromaLow = ae::Min( chromaLow, lch.y );
			chromaHigh = ae::Max( chromaHigh, lch.y );
		}
	}
	metrics.chromaticCount = hues.Length();
	metrics.chromaSpread = ( chromaHigh > chromaLow ) ? ( chromaHigh - chromaLow ) : 0.0f;

	std::sort( lightness.begin(), lightness.end() );
	if( lightness.Length() >= 2 )
	{
		metrics.lightnessRange = lightness[ lightness.Length() - 1 ] - lightness[ 0 ];
		for( uint32_t i = 0; i + 1 < lightness.Length(); i++ )
		{
			const float gap = lightness[ i + 1 ] - lightness[ i ];
			if( gap > metrics.worstValueGap )
			{
				metrics.worstValueGap = gap;
				metrics.worstValueGapAt = lightness[ i ];
			}
		}
	}

	std::sort( hues.begin(), hues.end() );
	if( hues.Length() >= 2 )
	{
		for( uint32_t i = 0; i < hues.Length(); i++ )
		{
			const float next = ( i + 1 < hues.Length() ) ? hues[ i + 1 ] : ( hues[ 0 ] + 360.0f );
			const float gap = next - hues[ i ];
			if( gap > metrics.worstHueVoid )
			{
				metrics.worstHueVoid = gap;
				metrics.worstHueVoidAt = hues[ i ];
			}
		}
	}
	return metrics;
}

//------------------------------------------------------------------------------
// ImGui helpers
//------------------------------------------------------------------------------
//! ImGui works in linear here: aeImGui converts the style to linear on init
//! because ae::GraphicsDevice applies the sRGB transfer in its present shader.
//! Handing it sRGB values encodes the transfer twice and lifts everything.
static ImU32 ToImColor( ae::Color color )
{
	const ae::Vec3 rgb = color.GetLinearRGB();
	return IM_COL32( (int)( rgb.x * 255.0f ), (int)( rgb.y * 255.0f ), (int)( rgb.z * 255.0f ), 255 );
}

static ImVec4 ToImVec4( ae::Color color )
{
	const ae::Vec3 rgb = color.GetLinearRGB();
	return ImVec4( rgb.x, rgb.y, rgb.z, 1.0f );
}

static ae::Str32 ToHex( ae::Color color )
{
	const ae::Vec3 srgb = color.GetSRGB();
	char buffer[ 8 ];
	snprintf( buffer, sizeof( buffer ), "#%02X%02X%02X",
		(int)( srgb.x * 255.0f + 0.5f ),
		(int)( srgb.y * 255.0f + 0.5f ),
		(int)( srgb.z * 255.0f + 0.5f ) );
	return ae::Str32( buffer );
}

//! Returns true when \p text parsed as "#RRGGBB" or "RRGGBB".
static bool ParseHex( const char* text, ae::Color* colorOut )
{
	while( *text == '#' || *text == ' ' )
	{
		text++;
	}
	uint32_t r, g, b;
	if( sscanf( text, "%02x%02x%02x", &r, &g, &b ) != 3 )
	{
		return false;
	}
	*colorOut = ae::Color::SRGB8( (uint8_t)r, (uint8_t)g, (uint8_t)b );
	return true;
}

//------------------------------------------------------------------------------
// Schemes
//------------------------------------------------------------------------------
static void LoadScheme( const Scheme& scheme, Slot* slotsOut )
{
	for( uint32_t i = 0; i < kSlotCount; i++ )
	{
		const SchemeColor& entry = scheme.colors[ i ];
		slotsOut[ i ].SetColor( ae::Color::SRGB8( entry.r, entry.g, entry.b ) );
		snprintf( slotsOut[ i ].name, sizeof( slotsOut[ i ].name ), "%s", entry.name );
	}
}

//! Fills \p orderOut with slot indices, by ascending lightness when \p sorted
//! and declaration order otherwise. Empty slots always come last. Display only:
//! the slots themselves never move, so editing and export keep the order the
//! palette is written in.
static void GetDisplayOrder( const Slot* slots, uint32_t count, bool sorted, uint32_t* orderOut )
{
	for( uint32_t i = 0; i < count; i++ )
	{
		orderOut[ i ] = i;
	}
	if( sorted )
	{
		std::sort( orderOut, orderOut + count, [ slots ]( uint32_t lhs, uint32_t rhs )
		{
			const bool lhsEmpty = IsEmptySlot( slots[ lhs ] );
			const bool rhsEmpty = IsEmptySlot( slots[ rhs ] );
			if( lhsEmpty != rhsEmpty )
			{
				return rhsEmpty;
			}
			return slots[ lhs ].lch.x < slots[ rhs ].lch.x;
		} );
	}
}

//! Measures a scheme without disturbing the slots being edited.
static Metrics MeasureScheme( const Scheme& scheme )
{
	Slot scratch[ kSlotCount ];
	LoadScheme( scheme, scratch );
	return MeasurePalette( scratch, kSlotCount );
}


//------------------------------------------------------------------------------
// BackgroundJob
//------------------------------------------------------------------------------
//! One unit of work on a worker thread. The owner polls IsComplete() and calls
//! Finish() before reading the result. The worker must not allocate, so any
//! buffers it writes are sized by the owner before Start(). Web builds have no
//! threads, so there the work runs inline and completes before Start() returns.
class BackgroundJob
{
public:
	~BackgroundJob() { Finish(); }

	//! Runs \p fn on a worker. Must not be called while already running.
	template< typename Fn >
	void Start( Fn&& fn )
	{
		AE_ASSERT( !m_running );
		m_complete = false;
		m_running = true;
#if _AE_EMSCRIPTEN_
		fn();
		m_complete = true;
#else
		m_thread = std::thread( [ this, fn ]()
		{
			fn();
			m_complete = true;
		} );
#endif
	}

	bool IsRunning() const { return m_running; }
	//! True once the worker is done and Finish() has not been called yet.
	bool IsComplete() const { return m_running && m_complete; }

	//! Joins the worker. Does nothing when not running.
	void Finish()
	{
		if( m_running )
		{
#if !_AE_EMSCRIPTEN_
			m_thread.join();
#endif
			m_running = false;
			m_complete = false;
		}
	}

private:
	std::thread m_thread;
	std::atomic< bool > m_complete = { false };
	bool m_running = false;
};

//------------------------------------------------------------------------------
// Test images
//------------------------------------------------------------------------------
//! A PNG decoded into memory. Nothing is written back and nothing is vendored;
//! images are chosen at runtime.
struct TestImage
{
	TestImage( const ae::Tag& tag ) : source( tag ) {}
	ae::Str128 name;
	ae::Array< uint8_t > source; //!< RGB8, sRGB encoded
	uint32_t width = 0;
	uint32_t height = 0;
};

//------------------------------------------------------------------------------
// Test sheet
//------------------------------------------------------------------------------
//! Ordered dither matrix. Perturbing the source before matching spreads the
//! error over neighbouring pixels, which is what lets two palette entries stand
//! in for a colour between them.
const float kBayer4[ 16 ] = {
	 0.0f / 16.0f,  8.0f / 16.0f,  2.0f / 16.0f, 10.0f / 16.0f,
	12.0f / 16.0f,  4.0f / 16.0f, 14.0f / 16.0f,  6.0f / 16.0f,
	 3.0f / 16.0f, 11.0f / 16.0f,  1.0f / 16.0f,  9.0f / 16.0f,
	15.0f / 16.0f,  7.0f / 16.0f, 13.0f / 16.0f,  5.0f / 16.0f,
};

//! Nearest palette entry by CIELAB distance. Empty slots are not candidates.
static ae::Color MapToPalette( ae::Color source, const Slot* slots, uint32_t count,
	uint32_t x, uint32_t y, float ditherStrength )
{
	ae::Vec3 lab = ColorToLCh( source );
	if( ditherStrength > 0.0f )
	{
		const float threshold = kBayer4[ ( y % 4 ) * 4 + ( x % 4 ) ] - 0.5f;
		lab.x = ae::Clip( lab.x + threshold * ditherStrength, 0.0f, 100.0f );
	}
	const ae::Vec3 sourceLab = LChToLab( lab );
	float bestDistance = 1e9f;
	ae::Color best = source;
	for( uint32_t i = 0; i < count; i++ )
	{
		if( IsEmptySlot( slots[ i ] ) )
		{
			continue;
		}
		const ae::Vec3 slotLab = LChToLab( slots[ i ].lch );
		const float distance = ( slotLab - sourceLab ).LengthSquared();
		if( distance < bestDistance )
		{
			bestDistance = distance;
			best = slots[ i ].color;
		}
	}
	return best;
}

//! Fills \p pixelsOut with sRGB bytes. Bands: value ramp, hue sweep, a hue by
//! lightness field, then a lit sphere per slot.
static void GenerateTestSheet( const Slot* slots, uint32_t count, float ditherStrength,
	uint8_t* pixelsOut )
{
	auto Put = [ pixelsOut ]( uint32_t x, uint32_t y, ae::Color color )
	{
		const ae::Vec3 srgb = color.GetSRGB();
		uint8_t* p = pixelsOut + ( y * kTestWidth + x ) * 3;
		p[ 0 ] = (uint8_t)( ae::Clip01( srgb.x ) * 255.0f + 0.5f );
		p[ 1 ] = (uint8_t)( ae::Clip01( srgb.y ) * 255.0f + 0.5f );
		p[ 2 ] = (uint8_t)( ae::Clip01( srgb.z ) * 255.0f + 0.5f );
	};

	uint32_t row = 0;
	// Value ramp, then the same ramp mapped. Banding here is the palette's
	// lightness gaps made visible.
	for( uint32_t y = 0; y < kTestBand; y++ )
	{
		for( uint32_t x = 0; x < kTestWidth; x++ )
		{
			const float lightness = 100.0f * x / ( kTestWidth - 1.0f );
			const ae::Color source = LChToColor( ae::Vec3( lightness, 0.0f, 0.0f ), nullptr );
			Put( x, row + y, source );
			Put( x, row + kTestBand + y, MapToPalette( source, slots, count, x, y, ditherStrength ) );
		}
	}
	row += kTestBand * 2;

	// Hue sweep at a fixed mid lightness and chroma.
	for( uint32_t y = 0; y < kTestBand; y++ )
	{
		for( uint32_t x = 0; x < kTestWidth; x++ )
		{
			const float hue = 360.0f * x / ( kTestWidth - 1.0f );
			const ae::Color source = LChToColor( ae::Vec3( 60.0f, 45.0f, hue ), nullptr );
			Put( x, row + y, source );
			Put( x, row + kTestBand + y, MapToPalette( source, slots, count, x, y, ditherStrength ) );
		}
	}
	row += kTestBand * 2;

	// Hue across, lightness down. Flat regions are hues the palette collapses.
	for( uint32_t y = 0; y < kTestFieldHeight; y++ )
	{
		for( uint32_t x = 0; x < kTestWidth; x++ )
		{
			const float hue = 360.0f * x / ( kTestWidth - 1.0f );
			const float lightness = 95.0f - 85.0f * y / ( kTestFieldHeight - 1.0f );
			const ae::Color source = LChToColor( ae::Vec3( lightness, 38.0f, hue ), nullptr );
			Put( x, row + y, source );
			Put( x, row + kTestFieldHeight + y, MapToPalette( source, slots, count, x, y, ditherStrength ) );
		}
	}
	row += kTestFieldHeight * 2;

	// ColorChecker. Each patch is split down the middle, source left and mapped
	// right, so a shift shows as a seam rather than needing a second grid.
	for( uint32_t i = 0; i < 24; i++ )
	{
		const SchemeColor& patch = kColorChecker[ i ];
		const ae::Color source = ae::Color::SRGB8( patch.r, patch.g, patch.b );
		const uint32_t cellX = ( i % 6 ) * ( kTestWidth / 6 );
		const uint32_t cellY = ( i / 6 ) * kTestCheckerCell;
		for( uint32_t y = 0; y < kTestCheckerCell; y++ )
		{
			for( uint32_t x = 0; x < kTestWidth / 6; x++ )
			{
				const bool mapped = ( x > ( kTestWidth / 6 ) / 2 );
				Put( cellX + x, row + cellY + y, mapped
					? MapToPalette( source, slots, count, x, y, ditherStrength ) : source );
			}
		}
	}
	row += kTestCheckerCell * 4;

	// A lit sphere per slot, mapped. A hue with nowhere to shade goes flat.
	const uint32_t cell = kTestWidth / count;
	const float radius = cell * 0.42f;
	const ae::Vec3 lightDir = ae::Vec3( -0.45f, -0.5f, 0.74f ).NormalizeCopy();
	for( uint32_t y = 0; y < kTestSphereHeight; y++ )
	{
		for( uint32_t x = 0; x < kTestWidth; x++ )
		{
			Put( x, row + y, slots[ count - 1 ].color );
		}
	}
	for( uint32_t i = 0; i < count; i++ )
	{
		const ae::Vec3 base = slots[ i ].lch;
		const float centerX = i * cell + cell * 0.5f;
		const float centerY = kTestSphereHeight * 0.5f;
		for( uint32_t y = 0; y < kTestSphereHeight; y++ )
		{
			for( uint32_t x = (uint32_t)( i * cell ); x < ( i + 1 ) * cell && x < kTestWidth; x++ )
			{
				const float dx = ( x - centerX ) / radius;
				const float dy = ( y - centerY ) / radius;
				const float sq = dx * dx + dy * dy;
				if( sq > 1.0f )
				{
					continue;
				}
				const ae::Vec3 normal = ae::Vec3( dx, dy, sqrtf( 1.0f - sq ) );
				const float lambert = ae::Max( 0.0f, normal.Dot( lightDir ) );
				// Shade in lightness so the sphere asks the palette for tones of
				// its own hue rather than for a darker grey.
				const float shaded = ae::Clip( base.x * ( 0.30f + 0.85f * lambert ), 0.0f, 100.0f );
				const ae::Color source = LChToColor( ae::Vec3( shaded, base.y, base.z ), nullptr );
				Put( x, row + y, MapToPalette( source, slots, count, x, y, ditherStrength ) );
			}
		}
	}
}

//! Remaps \p image into \p pixelsOut, which must hold width * height * 3 bytes.
static void MapImage( const TestImage& image, const Slot* slots, uint32_t count,
	float ditherStrength, uint8_t* pixelsOut )
{
	for( uint32_t y = 0; y < image.height; y++ )
	{
		for( uint32_t x = 0; x < image.width; x++ )
		{
			const uint32_t i = ( y * image.width + x ) * 3;
			const ae::Color source = ae::Color::SRGB8(
				image.source[ i ], image.source[ i + 1 ], image.source[ i + 2 ] );
			const ae::Color mapped = MapToPalette( source, slots, count, x, y, ditherStrength );
			const ae::Vec3 srgb = mapped.GetSRGB();
			pixelsOut[ i ] = (uint8_t)( ae::Clip01( srgb.x ) * 255.0f + 0.5f );
			pixelsOut[ i + 1 ] = (uint8_t)( ae::Clip01( srgb.y ) * 255.0f + 0.5f );
			pixelsOut[ i + 2 ] = (uint8_t)( ae::Clip01( srgb.z ) * 255.0f + 0.5f );
		}
	}
}

//! Hover readout for a single slot, shared by the swatch grid and the
//! hue-brightness plot.
static void DrawColorTooltip( const Slot& slot, const Slot* slots, uint32_t count )
{
	uint32_t darkest = 0;
	uint32_t lightest = 0;
	for( uint32_t i = 0; i < count; i++ )
	{
		darkest = ( slots[ i ].lch.x < slots[ darkest ].lch.x ) ? i : darkest;
		lightest = ( slots[ i ].lch.x > slots[ lightest ].lch.x ) ? i : lightest;
	}
	ImGui::BeginTooltip();
	ImGui::Text( "%s  %s", slot.name, ToHex( slot.color ).c_str() );
	ImGui::Text( "L*%.1f C*%.1f h%.0f", slot.lch.x, slot.lch.y, slot.lch.z );
	ImGui::Text( "on %s %.2f   on %s %.2f",
		slots[ darkest ].name, ContrastRatio( slot.color, slots[ darkest ].color ),
		slots[ lightest ].name, ContrastRatio( slot.color, slots[ lightest ].color ) );
	ImGui::EndTooltip();
}

//------------------------------------------------------------------------------
// Panels
//------------------------------------------------------------------------------
//! Hue-brightness scatter, DawnBringer's primary diagram. Dots are draggable:
//! horizontal sets hue, vertical sets L*. Neutrals sit in the left gutter.
static void DrawHueBrightness( Slot* slots, uint32_t count, int32_t* selectedInOut,
	const Scheme* overlay, const Metrics& metrics )
{
	// Left margin carries the L* labels only. Every slot is plotted at its
	// measured hue, including near neutrals, where the angle is unstable but
	// still what the color has.
	const float labelMargin = 34.0f;
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	const ImVec2 avail = ImGui::GetContentRegionAvail();
	const ImVec2 size = ImVec2( ae::Max( avail.x, 240.0f ), ae::Max( avail.y - 4.0f, 200.0f ) );
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	const ImVec2 plotMin = ImVec2( origin.x + labelMargin, origin.y );
	const ImVec2 plotMax = ImVec2( origin.x + size.x, origin.y + size.y );
	const float plotWidth = plotMax.x - plotMin.x;

	drawList->AddRectFilled( origin, plotMax, IM_COL32( 11, 14, 18, 255 ) );
	if( metrics.worstHueVoid > 0.0f )
	{
		const float voidStart = metrics.worstHueVoidAt / 360.0f;
		const float voidEnd = ae::Min( 1.0f, ( metrics.worstHueVoidAt + metrics.worstHueVoid ) / 360.0f );
		drawList->AddRectFilled(
			ImVec2( plotMin.x + plotWidth * voidStart, plotMin.y ),
			ImVec2( plotMin.x + plotWidth * voidEnd, plotMax.y ),
			IM_COL32( 255, 75, 90, 28 ) );
	}
	for( uint32_t i = 0; i <= 4; i++ )
	{
		const float y = plotMin.y + ( plotMax.y - plotMin.y ) * ( 1.0f - i * 0.25f );
		drawList->AddLine( ImVec2( plotMin.x, y ), ImVec2( plotMax.x, y ), IM_COL32( 42, 48, 57, 255 ) );
		drawList->AddText( ImVec2( origin.x, y - 7.0f ), IM_COL32( 143, 163, 182, 255 ),
			ae::Str32::Format( "L*#", i * 25 ).c_str() );
		const float x = plotMin.x + plotWidth * i * 0.25f;
		drawList->AddLine( ImVec2( x, plotMin.y ), ImVec2( x, plotMax.y ), IM_COL32( 42, 48, 57, 255 ) );
	}
	drawList->AddLine( ImVec2( plotMin.x, plotMin.y ), ImVec2( plotMin.x, plotMax.y ),
		IM_COL32( 80, 90, 104, 255 ) );

	for( uint32_t i = 0; overlay && i < kSlotCount; i++ )
	{
		const ae::Color overlayColor = ae::Color::SRGB8(
			overlay->colors[ i ].r, overlay->colors[ i ].g, overlay->colors[ i ].b );
		const ae::Vec3 lch = ColorToLCh( overlayColor );
		const float y = plotMax.y - ( plotMax.y - plotMin.y ) * ( lch.x / 100.0f );
		const float x = plotMin.x + plotWidth * lch.z / 360.0f;
		drawList->AddCircle( ImVec2( x, y ), 9.0f, ToImColor( overlayColor ), 0, 2.0f );
	}

	ImGui::InvisibleButton( "hueBrightness", size );
	const bool active = ImGui::IsItemActive();
	const ImVec2 mouse = ImGui::GetIO().MousePos;
	int32_t hovered = -1;
	for( uint32_t i = 0; i < count; i++ )
	{
		const ae::Vec3 lch = slots[ i ].lch;
		const float y = plotMax.y - ( plotMax.y - plotMin.y ) * ( lch.x / 100.0f );
		const float x = plotMin.x + plotWidth * lch.z / 360.0f;
		const float radius = 7.0f + lch.y * 0.10f;
		const bool isSelected = ( (int32_t)i == *selectedInOut );
		drawList->AddCircleFilled( ImVec2( x, y ), radius, ToImColor( slots[ i ].color ) );
		drawList->AddCircle( ImVec2( x, y ), radius,
			isSelected ? IM_COL32( 255, 255, 255, 255 ) : IM_COL32( 0, 0, 0, 200 ), 0, isSelected ? 2.5f : 1.0f );
		const float dx = mouse.x - x;
		const float dy = mouse.y - y;
		if( dx * dx + dy * dy < ( radius + 4.0f ) * ( radius + 4.0f ) )
		{
			hovered = (int32_t)i;
		}
	}
	if( hovered >= 0 )
	{
		DrawColorTooltip( slots[ hovered ], slots, count );
	}
	if( ImGui::IsItemClicked() && hovered >= 0 )
	{
		*selectedInOut = hovered;
	}
	if( active && *selectedInOut >= 0 && ImGui::IsMouseDragging( ImGuiMouseButton_Left ) )
	{
		Slot& slot = slots[ *selectedInOut ];
		ae::Vec3 lch = slot.lch;
		lch.x = ae::Clip01( ( plotMax.y - mouse.y ) / ( plotMax.y - plotMin.y ) ) * 100.0f;
		lch.z = ae::Clip01( ( mouse.x - plotMin.x ) / plotWidth ) * 360.0f;
		slot.SetLCh( lch );
	}
}

//! Brightness range coverage. DawnBringer calls this a must for any useful
//! palette, so the worst gap is called out directly on the bar.
static void DrawValueLadder( const Slot* slots, uint32_t count, const Metrics& metrics )
{
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	const float width = ae::Max( ImGui::GetContentRegionAvail().x, 320.0f );
	const float height = 44.0f;
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled( origin, ImVec2( origin.x + width, origin.y + height ),
		IM_COL32( 11, 14, 18, 255 ) );
	if( metrics.worstValueGap > 0.0f )
	{
		drawList->AddRectFilled(
			ImVec2( origin.x + width * metrics.worstValueGapAt / 100.0f, origin.y ),
			ImVec2( origin.x + width * ( metrics.worstValueGapAt + metrics.worstValueGap ) / 100.0f,
				origin.y + height ),
			IM_COL32( 255, 75, 90, 56 ) );
	}
	for( uint32_t i = 0; i < count; i++ )
	{
		const float x = origin.x + width * slots[ i ].lch.x / 100.0f;
		drawList->AddRectFilled( ImVec2( x - 3.0f, origin.y ), ImVec2( x + 3.0f, origin.y + height ),
			ToImColor( slots[ i ].color ) );
	}
	ImGui::Dummy( ImVec2( width, height ) );
	ImGui::Text( "range %.0f    worst gap %.0f at L*%.0f    chromatics %u    chroma spread %.0f",
		metrics.lightnessRange, metrics.worstValueGap, metrics.worstValueGapAt,
		metrics.chromaticCount, metrics.chromaSpread );
}

//! Pairwise wash matrix. Cell ( row, column ) shows the two slots in contact,
//! which is where simultaneous contrast and dither reach actually show up.
enum class WashMode
{
	Blend,  //!< 50% linear mix, what alpha or a fine dither resolves to
	Dither, //!< checkerboard of the two, what the eye gets at real pixel sizes
	Mark    //!< a mark of the row color on a field of the column color
};

static void DrawWashMatrix( const Slot* slots, uint32_t count, WashMode mode )
{
	const ImVec2 avail = ImGui::GetContentRegionAvail();
	const float cellSize = ae::Max( 8.0f, ae::Min( avail.x, avail.y ) / count );
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	const float total = cellSize * count;
	for( uint32_t row = 0; row < count; row++ )
	{
		for( uint32_t column = 0; column < count; column++ )
		{
			const ImVec2 cellMin( origin.x + column * cellSize, origin.y + row * cellSize );
			const ImVec2 cellMax( cellMin.x + cellSize - 1.0f, cellMin.y + cellSize - 1.0f );
			const ae::Color rowColor = slots[ row ].color;
			const ae::Color columnColor = slots[ column ].color;
			if( mode == WashMode::Blend )
			{
				drawList->AddRectFilled( cellMin, cellMax, ToImColor( rowColor.Lerp( columnColor, 0.5f ) ) );
			}
			else if( mode == WashMode::Dither )
			{
				const uint32_t checkers = 4;
				const float step = ( cellSize - 1.0f ) / checkers;
				for( uint32_t y = 0; y < checkers; y++ )
				{
					for( uint32_t x = 0; x < checkers; x++ )
					{
						drawList->AddRectFilled(
							ImVec2( cellMin.x + x * step, cellMin.y + y * step ),
							ImVec2( cellMin.x + ( x + 1 ) * step, cellMin.y + ( y + 1 ) * step ),
							ToImColor( ( ( x + y ) & 1 ) ? rowColor : columnColor ) );
					}
				}
			}
			else
			{
				drawList->AddRectFilled( cellMin, cellMax, ToImColor( columnColor ) );
				const float inset = cellSize * 0.28f;
				drawList->AddRectFilled(
					ImVec2( cellMin.x + inset, cellMin.y + inset ),
					ImVec2( cellMax.x - inset, cellMax.y - inset ), ToImColor( rowColor ) );
			}
		}
	}
	ImGui::InvisibleButton( "wash", ImVec2( total, total ) );
	if( ImGui::IsItemHovered() )
	{
		const ImVec2 mouse = ImGui::GetIO().MousePos;
		const uint32_t column = (uint32_t)ae::Clip( ( mouse.x - origin.x ) / cellSize, 0.0f, count - 1.0f );
		const uint32_t row = (uint32_t)ae::Clip( ( mouse.y - origin.y ) / cellSize, 0.0f, count - 1.0f );
		const ae::Color blend = slots[ row ].color.Lerp( slots[ column ].color, 0.5f );
		const ae::Vec3 rowLCh = slots[ row ].lch;
		const ae::Vec3 columnLCh = slots[ column ].lch;
		const float deltaL = rowLCh.x - columnLCh.x;
		ImGui::BeginTooltip();
		ImGui::Text( "%s  x  %s", slots[ row ].name, slots[ column ].name );
		ImGui::Text( "blend %s    contrast %.2f    dL* %.1f", ToHex( blend ).c_str(),
			ContrastRatio( slots[ row ].color, slots[ column ].color ), deltaL );
		ImGui::EndTooltip();
	}
}


//------------------------------------------------------------------------------
// Batch mode
//------------------------------------------------------------------------------
static uint32_t Crc32( const uint8_t* data, uint32_t length, uint32_t crc = 0xFFFFFFFFu )
{
	for( uint32_t i = 0; i < length; i++ )
	{
		crc ^= data[ i ];
		for( uint32_t bit = 0; bit < 8; bit++ )
		{
			crc = ( crc >> 1 ) ^ ( 0xEDB88320u & ( 0u - ( crc & 1u ) ) );
		}
	}
	return crc;
}

static void AppendChunk( ae::Array< uint8_t >& out, const char* type, const uint8_t* data, uint32_t length )
{
	const uint8_t header[ 4 ] = { (uint8_t)( length >> 24 ), (uint8_t)( length >> 16 ),
		(uint8_t)( length >> 8 ), (uint8_t)length };
	out.AppendArray( header, 4 );
	const uint32_t start = out.Length();
	out.AppendArray( (const uint8_t*)type, 4 );
	if( length )
	{
		out.AppendArray( data, length );
	}
	const uint32_t crc = Crc32( &out[ start ], length + 4 ) ^ 0xFFFFFFFFu;
	const uint8_t tail[ 4 ] = { (uint8_t)( crc >> 24 ), (uint8_t)( crc >> 16 ),
		(uint8_t)( crc >> 8 ), (uint8_t)crc };
	out.AppendArray( tail, 4 );
}

//! Writes 8 bit RGB as a PNG using stored deflate blocks, so nothing has to be
//! linked to produce a file the rest of the world can open.
static bool WritePng( const char* path, const uint8_t* pixels, uint32_t width, uint32_t height )
{
	ae::Array< uint8_t > raw = TAG_ALL;
	raw.Reserve( height * ( 1 + width * 3 ) );
	for( uint32_t y = 0; y < height; y++ )
	{
		raw.Append( (uint8_t)0 );
		raw.AppendArray( pixels + y * width * 3, width * 3 );
	}
	ae::Array< uint8_t > z = TAG_ALL;
	z.Append( (uint8_t)0x78 );
	z.Append( (uint8_t)0x01 );
	for( uint32_t offset = 0; offset < raw.Length(); offset += 65535 )
	{
		const uint32_t size = ae::Min( 65535u, raw.Length() - offset );
		const bool last = ( offset + size >= raw.Length() );
		z.Append( (uint8_t)( last ? 1 : 0 ) );
		z.Append( (uint8_t)( size & 0xFF ) );
		z.Append( (uint8_t)( size >> 8 ) );
		z.Append( (uint8_t)( ~size & 0xFF ) );
		z.Append( (uint8_t)( ( ~size >> 8 ) & 0xFF ) );
		z.AppendArray( &raw[ offset ], size );
	}
	uint32_t a = 1, b = 0;
	for( uint32_t i = 0; i < raw.Length(); i++ )
	{
		a = ( a + raw[ i ] ) % 65521;
		b = ( b + a ) % 65521;
	}
	const uint32_t adler = ( b << 16 ) | a;
	z.Append( (uint8_t)( adler >> 24 ) );
	z.Append( (uint8_t)( adler >> 16 ) );
	z.Append( (uint8_t)( adler >> 8 ) );
	z.Append( (uint8_t)adler );

	ae::Array< uint8_t > file = TAG_ALL;
	const uint8_t signature[ 8 ] = { 137, 80, 78, 71, 13, 10, 26, 10 };
	file.AppendArray( signature, 8 );
	const uint8_t ihdr[ 13 ] = {
		(uint8_t)( width >> 24 ), (uint8_t)( width >> 16 ), (uint8_t)( width >> 8 ), (uint8_t)width,
		(uint8_t)( height >> 24 ), (uint8_t)( height >> 16 ), (uint8_t)( height >> 8 ), (uint8_t)height,
		8, 2, 0, 0, 0 };
	AppendChunk( file, "IHDR", ihdr, 13 );
	AppendChunk( file, "IDAT", z.Data(), z.Length() );
	AppendChunk( file, "IEND", nullptr, 0 );
	return ae::FileSystem::Write( path, file.Data(), file.Length(), true ) == file.Length();
}

//! Batch mode: <scheme> <image.png>... <output dir or .png>
//! Writes each image remapped to the scheme. Stitching is left to the caller.
static int32_t RunBatch( int32_t argc, char* argv[] )
{
	const Scheme* scheme = nullptr;
	for( uint32_t i = 0; i < countof( kSchemes ); i++ )
	{
		if( strcmp( kSchemes[ i ]->name, argv[ 1 ] ) == 0 )
		{
			scheme = kSchemes[ i ];
		}
	}
	if( !scheme )
	{
		AE_ERR( "No scheme named '#'", argv[ 1 ] );
		return 1;
	}
	Slot slots[ kSlotCount ];
	LoadScheme( *scheme, slots );

	const char* output = argv[ argc - 1 ];
	const bool outputIsFile = ( strstr( output, ".png" ) != nullptr );
	for( int32_t i = 2; i < argc - 1; i++ )
	{
		const char* inputPath = argv[ i ];
		const uint32_t fileSize = ae::FileSystem::GetSize( inputPath );
		if( !fileSize )
		{
			AE_WARN( "Could not read '#'", inputPath );
			continue;
		}
		ae::Array< uint8_t > fileData( TAG_ALL, (uint8_t)0, fileSize );
		ae::FileSystem::Read( inputPath, fileData.Data(), fileSize );
		int32_t width = 0, height = 0, channels = 0;
		uint8_t* decoded = stbi_load_from_memory( fileData.Data(), (int32_t)fileSize,
			&width, &height, &channels, 3 );
		if( !decoded )
		{
			AE_WARN( "Could not decode '#'", inputPath );
			continue;
		}
		const uint32_t w = (uint32_t)width;
		const uint32_t h = (uint32_t)height;
		ae::Array< uint8_t > mapped( TAG_ALL, (uint8_t)0, w * h * 3 );
		for( uint32_t y = 0; y < h; y++ )
		{
			for( uint32_t x = 0; x < w; x++ )
			{
				const uint32_t offset = ( y * w + x ) * 3;
				const ae::Color source = ae::Color::SRGB8(
					decoded[ offset ], decoded[ offset + 1 ], decoded[ offset + 2 ] );
				const ae::Vec3 srgb = MapToPalette( source, slots, kSlotCount, x, y, 0.0f ).GetSRGB();
				mapped[ offset ] = (uint8_t)( ae::Clip01( srgb.x ) * 255.0f + 0.5f );
				mapped[ offset + 1 ] = (uint8_t)( ae::Clip01( srgb.y ) * 255.0f + 0.5f );
				mapped[ offset + 2 ] = (uint8_t)( ae::Clip01( srgb.z ) * 255.0f + 0.5f );
			}
		}
		stbi_image_free( decoded );

		char outPath[ 512 ];
		if( outputIsFile )
		{
			snprintf( outPath, sizeof( outPath ), "%s", output );
		}
		else
		{
			const char* slash = strrchr( inputPath, '/' );
			char base[ 128 ];
			snprintf( base, sizeof( base ), "%s", slash ? ( slash + 1 ) : inputPath );
			char* dot = strstr( base, ".png" );
			if( dot )
			{
				*dot = 0;
			}
			snprintf( outPath, sizeof( outPath ), "%s/%s_%s.png", output, base, scheme->name );
		}
		if( WritePng( outPath, mapped.Data(), w, h ) )
		{
			AE_INFO( "Wrote '#'", outPath );
		}
		else
		{
			AE_ERR( "Could not write '#'", outPath );
		}
	}
	return 0;
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	// Batch mode when given arguments, so no window is created.
	if( argc >= 4 )
	{
		return RunBatch( argc, argv );
	}

	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	aeImGui ui;
	Slot slots[ kSlotCount ];
	int32_t selected = 0;
	int32_t overlayIndex = -1;
	bool compareAll = true;
	bool sortByLightness = true;
	char hexBuffer[ 16 ] = "";
	char blendHexA[ 16 ] = "";
	char blendHexB[ 16 ] = "";
	float blendT = 0.5f;
	ae::Color lastCopiedColor = ae::Color::Black();
	// Remapping a full size image is far too slow for the frame, so it runs on
	// a worker. The palette is snapshotted at launch, and a change during the
	// run marks it dirty so the job relaunches when it finishes.
	//! The color solid view and its groups. Bundled because ae::Application's
	//! callbacks inline 256 bytes of captures, and Update takes everything by
	//! reference.
	struct SolidView
	{
		SolidView( const ae::Tag& tag ) : debugLines( tag ) {}
		ae::DebugCamera camera = ae::Axis::Z;
		ae::DebugLines debugLines;
		ae::RenderTarget viewportTarget;
		bool viewportHovered = false;
		ColorGroup groups[ kMaxGroups ];
		int32_t selectedGroup = 0;
		int32_t blendSpace = (int32_t)BlendSpace::LCh;
		bool showAllGroups = true;
	} solid( TAG_ALL );
	BackgroundJob remapJob;
	bool remapDirty = true;
	uint64_t remapSignature = 0;
	Slot remapSlots[ kSlotCount ];
	ae::Array< uint8_t > remapPixels = TAG_ALL;
	uint32_t remapWidth = 0;
	uint32_t remapHeight = 0;
	bool hexEditing = false;
	ae::Array< TestImage* > testImages = TAG_ALL;
	int32_t testImageIndex = -1; //!< -1 is the procedural sheet
	float ditherStrength = 0.0f;
	ae::Texture2D testTexture;
	ae::Array< uint8_t > testPixels = TAG_ALL;
	uint64_t testSignature = 0;
	int32_t washMode = (int32_t)WashMode::Blend;
	ae::Color ground = ae::Color::SRGB8( 31, 30, 27 );
	ae::FileSystem fileSystem;
	//! ImGui keeps the pointer rather than copying, so this outlives the loop.
	ae::Str256 iniPath;

	auto Initialize = [&]() -> bool
	{
		AE_INFO( "Initialize" );

		window.Initialize( 1600, 1000, false, true, "31_PaletteAnalyzer" );
		window.SetTitle( "31_PaletteAnalyzer" );
		render.Initialize( &window );
		input.Initialize( &window );
		timeStep.SetTimeStep( 1.0f / 60.0f );
		ui.Initialize();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		// A bundle launched from Finder starts in a directory it cannot write to,
		// so the layout goes to the platform's user directory instead.
		fileSystem.Initialize( "", "aether", "PaletteAnalyzer" );
		if( fileSystem.GetRootDir( ae::FileSystem::Root::User, &iniPath ) )
		{
			fileSystem.CreateFolder( ae::FileSystem::Root::User, "" );
			iniPath += "/imgui.ini";
			io.IniFilename = iniPath.c_str();
		}

		solid.debugLines.Initialize( 8 * 1024 );
		solid.debugLines.SetXRayEnabled( false );
		solid.camera.Reset( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::Vec3( 3.5f, 3.5f, 2.5f ) );
		solid.camera.SetDistanceLimits( 0.5f, 20.0f );

		// Seeded from the groups already in this palette. Membership is by slot
		// index, so these follow position rather than color.
		const struct { const char* name; uint16_t members; } seedGroups[] = {
			{ "skin", ( 1 << 2 ) | ( 1 << 9 ) | ( 1 << 12 ) | ( 1 << 14 ) },
			{ "red", ( 1 << 1 ) | ( 1 << 6 ) | ( 1 << 11 ) },
			{ "purple", ( 1 << 0 ) | ( 1 << 4 ) | ( 1 << 7 ) },
			{ "blue", ( 1 << 5 ) | ( 1 << 8 ) },
			{ "yellow-green", ( 1 << 3 ) | ( 1 << 13 ) | ( 1 << 15 ) },
			{ "L35", ( 1 << 2 ) | ( 1 << 3 ) | ( 1 << 5 ) },
			{ "rainbow", ( 1 << 4 ) | ( 1 << 8 ) | ( 1 << 10 ) | ( 1 << 11 ) | ( 1 << 13 ) | ( 1 << 15 ) },
		};
		for( uint32_t i = 0; i < countof( seedGroups ) && i < kMaxGroups; i++ )
		{
			snprintf( solid.groups[ i ].name, sizeof( solid.groups[ i ].name ), "%s", seedGroups[ i ].name );
			solid.groups[ i ].members = seedGroups[ i ].members;
		}

		LoadScheme( *kSchemes[ 0 ], slots );
		lastCopiedColor = slots[ 0 ].color;

		AE_INFO( "Run" );
		return true;
	};

	auto Update = [&]() -> bool
	{
		const float dt = ae::Max( timeStep.GetTimeStep(), timeStep.GetDt() );
		input.Pump();
		ui.NewFrame( &render, &input, dt );

		const Metrics metrics = MeasurePalette( slots, kSlotCount );

		// Host window for the dockspace. Panels dock into it and are otherwise
		// free to be moved, resized and rearranged; ImGui stores the layout.
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos( viewport->WorkPos );
		ImGui::SetNextWindowSize( viewport->WorkSize );
		ImGui::SetNextWindowViewport( viewport->ID );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
		ImGui::Begin( "##host", nullptr,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking );
		ImGui::PopStyleVar( 3 );
		// The layout is built before DockSpace() is submitted, because DockSpace()
		// creates the node itself and there would be nothing left to test for.
		const ImGuiID dockspaceId = ImGui::GetID( "AnalyzerDockspace" );
		if( !ImGui::DockBuilderGetNode( dockspaceId ) )
		{
			ImGui::DockBuilderRemoveNode( dockspaceId );
			ImGui::DockBuilderAddNode( dockspaceId, ImGuiDockNodeFlags_DockSpace );
			ImGui::DockBuilderSetNodeSize( dockspaceId, viewport->WorkSize );
			// Tests down the right edge, palette editing stacked on the left, the
			// four analysis views gridded in the middle.
			ImGuiID main = 0;
			const ImGuiID tests = ImGui::DockBuilderSplitNode( dockspaceId, ImGuiDir_Right, 0.28f, nullptr, &main );
			ImGuiID center = 0;
			const ImGuiID left = ImGui::DockBuilderSplitNode( main, ImGuiDir_Left, 0.24f, nullptr, &center );

			ImGuiID leftRest = 0;
			const ImGuiID leftPalette = ImGui::DockBuilderSplitNode( left, ImGuiDir_Up, 0.34f, nullptr, &leftRest );
			ImGuiID leftRest2 = 0;
			const ImGuiID leftBlend = ImGui::DockBuilderSplitNode( leftRest, ImGuiDir_Up, 0.30f, nullptr, &leftRest2 );
			ImGuiID leftGroups = 0;
			const ImGuiID leftSchemes = ImGui::DockBuilderSplitNode( leftRest2, ImGuiDir_Up, 0.50f, nullptr, &leftGroups );

			ImGuiID centerBottom = 0;
			const ImGuiID centerTop = ImGui::DockBuilderSplitNode( center, ImGuiDir_Up, 0.56f, nullptr, &centerBottom );
			ImGuiID topSolid = 0;
			const ImGuiID topHue = ImGui::DockBuilderSplitNode( centerTop, ImGuiDir_Left, 0.42f, nullptr, &topSolid );
			ImGuiID bottomWashes = 0;
			const ImGuiID bottomValue = ImGui::DockBuilderSplitNode( centerBottom, ImGuiDir_Left, 0.46f, nullptr, &bottomWashes );
			ImGui::DockBuilderDockWindow( "Palette", leftPalette );
			ImGui::DockBuilderDockWindow( "Blend Tool", leftBlend );
			ImGui::DockBuilderDockWindow( "Schemes", leftSchemes );
			ImGui::DockBuilderDockWindow( "Groups", leftGroups );
			ImGui::DockBuilderDockWindow( "Hue - Brightness", topHue );
			ImGui::DockBuilderDockWindow( "Solid", topSolid );
			ImGui::DockBuilderDockWindow( "Value", bottomValue );
			ImGui::DockBuilderDockWindow( "Washes", bottomWashes );
			ImGui::DockBuilderDockWindow( "Tests", tests );
			const ImGuiID leaves[] = { leftPalette, leftBlend, leftSchemes, leftGroups,
				topHue, topSolid, bottomValue, bottomWashes, tests };
			for( const ImGuiID leaf : leaves )
			{
				if( ImGuiDockNode* node = ImGui::DockBuilderGetNode( leaf ) )
				{
					node->LocalFlags |= kLeafFlags;
				}
			}
			ImGui::DockBuilderFinish( dockspaceId );
		}
		ImGui::DockSpace( dockspaceId, ImVec2( 0.0f, 0.0f ),
			ImGuiDockNodeFlags_NoDockingSplit | ImGuiDockNodeFlags_NoUndocking );
		ImGui::End();

		BeginPanel( "Palette" );
		{
			ImGui::Checkbox( "sort by lightness", &sortByLightness );
			uint32_t order[ kSlotCount ];
			GetDisplayOrder( slots, kSlotCount, sortByLightness, order );
			const float swatch = ae::Max( 18.0f,
				( ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * ( kSlotColumns - 1 ) )
					/ kSlotColumns );
			for( uint32_t i = 0; i < kSlotCount; i++ )
			{
				const uint32_t slotIndex = order[ i ];
				ImGui::PushID( (int32_t)slotIndex );
				if( ImGui::ColorButton( "##swatch", ToImVec4( slots[ slotIndex ].color ),
					ImGuiColorEditFlags_NoTooltip, ImVec2( swatch, swatch ) ) )
				{
					selected = (int32_t)slotIndex;
				}
				if( ImGui::IsItemHovered() )
				{
					DrawColorTooltip( slots[ slotIndex ], slots, kSlotCount );
				}
				if( (int32_t)slotIndex == selected )
				{
					ImGui::GetWindowDrawList()->AddRect( ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
						IM_COL32( 255, 255, 255, 255 ), 0.0f, 0, 2.5f );
				}
				ImGui::PopID();
				if( ( i % kSlotColumns ) != ( kSlotColumns - 1 ) )
				{
					ImGui::SameLine();
				}
			}

			ImGui::Separator();
			Slot& slot = slots[ selected ];
			ImGui::InputText( "name", slot.name, sizeof( slot.name ) );
			ae::Vec3 lch = slot.lch;
			bool edited = false;
			edited |= ImGui::SliderFloat( "L*", &lch.x, 0.0f, 100.0f, "%.1f" );
			edited |= ImGui::SliderFloat( "C*", &lch.y, 0.0f, 130.0f, "%.1f" );
			edited |= ImGui::SliderFloat( "h", &lch.z, 0.0f, 360.0f, "%.1f" );
			if( edited )
			{
				slot.SetLCh( lch );
			}
			const float ceiling = MaxChroma( slot.lch.x, slot.lch.z );
			ImGui::Text( "%s   gamut ceiling C*%.1f%s", ToHex( slot.color ).c_str(), ceiling,
				slot.inGamut ? "" : "   CLIPPED" );
			if( !slot.inGamut && ImGui::Button( "Pull to gamut" ) )
			{
				slot.SetLCh( ae::Vec3( slot.lch.x, ceiling, slot.lch.z ) );
			}
			// Only refreshed from the slot while the field is idle, so typing and
			// pasting are not overwritten on the next frame.
			if( !hexEditing )
			{
				snprintf( hexBuffer, sizeof( hexBuffer ), "%s", ToHex( slot.color ).c_str() );
			}
			if( ImGui::InputText( "hex", hexBuffer, sizeof( hexBuffer ),
				ImGuiInputTextFlags_EnterReturnsTrue ) )
			{
				ae::Color parsed;
				if( ParseHex( hexBuffer, &parsed ) )
				{
					slot.SetColor( parsed );
				}
			}
			ImGui::SameLine();
			hexEditing = ImGui::IsItemActive();
			if( ImGui::Button( "Paste" ) )
			{
				ae::Color parsed;
				const std::string pasted = ae::GetClipboardText();
				if( ( selected < kSlotCount ) && ParseHex( pasted.c_str(), &parsed ) )
				{
					slots[ selected ].SetColor( parsed );
				}
			}

			uint32_t darkest = 0;
			uint32_t lightest = 0;
			for( uint32_t i = 0; i < kSlotCount; i++ )
			{
				darkest = ( slots[ i ].lch.x < slots[ darkest ].lch.x ) ? i : darkest;
				lightest = ( slots[ i ].lch.x > slots[ lightest ].lch.x ) ? i : lightest;
			}
			ImGui::Text( "on %s %.2f    on %s %.2f",
				slots[ darkest ].name, ContrastRatio( slot.color, slots[ darkest ].color ),
				slots[ lightest ].name, ContrastRatio( slot.color, slots[ lightest ].color ) );

			ImGui::Separator();
			if( ImGui::Button( "Copy as scheme" ) )
			{
				ae::Str< 2048 > out = "const Scheme kScheme_New = { \"New\", {\n";
				for( uint32_t i = 0; i < kSlotCount; i++ )
				{
					const ae::Vec3 srgb = slots[ i ].color.GetSRGB();
					out += ae::Str256::Format( "\t{ \"#\", #, #, # },\n",
						slots[ i ].name,
						(int)( srgb.x * 255.0f + 0.5f ),
						(int)( srgb.y * 255.0f + 0.5f ),
						(int)( srgb.z * 255.0f + 0.5f ) );
				}
				out += "} };\n";
				ae::SetClipboardText( out.c_str() );
			}
			if( ImGui::Button( "Copy as CSS" ) )
			{
				ae::Str< 2048 > out = ":root {\n";
				for( uint32_t i = 0; i < kSlotCount; i++ )
				{
					char lower[ 32 ];
					snprintf( lower, sizeof( lower ), "%s", slots[ i ].name );
					for( char* c = lower; *c; c++ )
					{
						*c = (char)tolower( (unsigned char)*c );
					}
					out += ae::Str256::Format( "\t--ae-#: #;\n", lower, ToHex( slots[ i ].color ) );
				}
				out += "}\n";
				ae::SetClipboardText( out.c_str() );
			}
		}
		ImGui::End();

		BeginPanel( "Blend Tool" );
		{
			// Blender. The two hex fields take a raw paste, so a color clicked
			// out of the Tests image goes straight in.
			auto PasteInto = []( char* buffer, uint32_t size )
			{
				const std::string pasted = ae::GetClipboardText();
				snprintf( buffer, size, "%s", pasted.c_str() );
			};
			ae::Color colorA, colorB;
			const bool hasA = ParseHex( blendHexA, &colorA );
			const bool hasB = ParseHex( blendHexB, &colorB );
			const ImVec2 chip( 18.0f, 18.0f );
			const float fieldWidth = ae::Max( 50.0f, ImGui::GetContentRegionAvail().x - 106.0f );

			auto HexRow = [&]( const char* chipId, const char* fieldId, const char* buttonId,
				char* buffer, uint32_t size, bool valid, ae::Color color )
			{
				ImGui::ColorButton( chipId, valid ? ToImVec4( color ) : ImVec4( 0, 0, 0, 0 ),
					ImGuiColorEditFlags_NoTooltip, chip );
				ImGui::SameLine();
				ImGui::SetNextItemWidth( fieldWidth );
				ImGui::InputText( fieldId, buffer, size );
				ImGui::SameLine();
				if( ImGui::Button( buttonId ) )
				{
					PasteInto( buffer, size );
				}
			};
			HexRow( "##chipA", "##hex1", "paste##1", blendHexA, sizeof( blendHexA ), hasA, colorA );
			HexRow( "##chipB", "##hex2", "paste##2", blendHexB, sizeof( blendHexB ), hasB, colorB );

			ImGui::SetNextItemWidth( fieldWidth );
			ImGui::SliderFloat( "lerp", &blendT, 0.0f, 1.0f, "%.2f" );

			// The whole ramp with a mark at the current position, so the slider
			// reads against what it is picking from.
			const ImVec2 rampMin = ImGui::GetCursorScreenPos();
			const float rampWidth = ae::Max( 32.0f, ImGui::GetContentRegionAvail().x - 8.0f );
			const float rampHeight = 20.0f;
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			if( hasA && hasB )
			{
				const uint32_t steps = 64;
				for( uint32_t i = 0; i < steps; i++ )
				{
					const float t0 = (float)i / steps;
					drawList->AddRectFilled(
						ImVec2( rampMin.x + rampWidth * t0, rampMin.y ),
						ImVec2( rampMin.x + rampWidth * ( i + 1.0f ) / steps, rampMin.y + rampHeight ),
						ToImColor( colorA.Lerp( colorB, t0 ) ) );
				}
				const float markX = rampMin.x + rampWidth * blendT;
				drawList->AddLine( ImVec2( markX, rampMin.y - 2.0f ),
					ImVec2( markX, rampMin.y + rampHeight + 2.0f ), IM_COL32( 255, 255, 255, 255 ), 2.0f );
			}
			else
			{
				drawList->AddRect( rampMin, ImVec2( rampMin.x + rampWidth, rampMin.y + rampHeight ),
					IM_COL32( 80, 90, 104, 255 ) );
			}
			ImGui::Dummy( ImVec2( rampWidth, rampHeight ) );

			char blendText[ 16 ] = "";
			ae::Color blended;
			if( hasA && hasB )
			{
				blended = colorA.Lerp( colorB, blendT );
				snprintf( blendText, sizeof( blendText ), "%s", ToHex( blended ).c_str() );
			}
			ImGui::ColorButton( "##blendchip",
				blendText[ 0 ] ? ToImVec4( blended ) : ImVec4( 0, 0, 0, 0 ),
				ImGuiColorEditFlags_NoTooltip, chip );
			ImGui::SameLine();
			ImGui::SetNextItemWidth( fieldWidth );
			ImGui::InputText( "##blend", blendText, sizeof( blendText ),
				ImGuiInputTextFlags_ReadOnly );
			ImGui::SameLine();
			if( ImGui::Button( "copy" ) && blendText[ 0 ] )
			{
				ae::SetClipboardText( blendText );
			}
		}
		ImGui::End();

		BeginPanel( "Hue - Brightness" );
		{
			ImGui::SetNextItemWidth( 240.0f );
			if( ImGui::BeginCombo( "overlay",
				( overlayIndex >= 0 ) ? kSchemes[ overlayIndex ]->name : "none" ) )
			{
				if( ImGui::Selectable( "none", overlayIndex < 0 ) )
				{
					overlayIndex = -1;
				}
				for( int32_t i = 0; i < (int32_t)countof( kSchemes ); i++ )
				{
					if( ImGui::Selectable( kSchemes[ i ]->name, overlayIndex == i ) )
					{
						overlayIndex = i;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::Text( "worst hue void %.0f deg at %.0f deg", metrics.worstHueVoid, metrics.worstHueVoidAt );
			DrawHueBrightness( slots, kSlotCount, &selected,
				( overlayIndex >= 0 ) ? kSchemes[ overlayIndex ] : nullptr, metrics );
		}
		ImGui::End();

		BeginPanel( "Value" );
		{
			DrawValueLadder( slots, kSlotCount, metrics );
			ImGui::Separator();
			const char* registerNames[ 4 ] = { "dark 0-25", "lower-mid 25-50", "upper-mid 50-75", "bright 75-100" };
			for( int32_t band = 3; band >= 0; band-- )
			{
				ImGui::Text( "%-16s %u", registerNames[ band ], metrics.registerCounts[ band ] );
				for( uint32_t i = 0; i < kSlotCount; i++ )
				{
					const float lightness = slots[ i ].lch.x;
					const float low = ( band > 0 ) ? kRegisterEdges[ band - 1 ] : -1.0f;
					const float high = ( band < 3 ) ? kRegisterEdges[ band ] : 101.0f;
					if( lightness >= low && lightness < high )
					{
						ImGui::SameLine();
						ImGui::ColorButton( slots[ i ].name, ToImVec4( slots[ i ].color ),
							ImGuiColorEditFlags_NoTooltip, ImVec2( 26.0f, 26.0f ) );
					}
				}
			}
		}
		ImGui::End();

		BeginPanel( "Washes" );
		{
			ImGui::SetNextItemWidth( 220.0f );
			ImGui::Combo( "mode", &washMode, "50% blend\0checkerboard dither\0mark on field\0" );
			DrawWashMatrix( slots, kSlotCount, (WashMode)washMode );
		}
		ImGui::End();

		BeginPanel( "Tests" );
		{
			if( ImGui::Button( "Load PNGs..." ) )
			{
				ae::FileDialogParams params;
				params.windowTitle = "Load test images";
				params.window = &window;
				params.allowMultipleSelection = true;
				params.filters.Append( ae::FileFilter( "PNG", "png" ) );
				const ae::Array< std::string > paths = ae::FileSystem::OpenDialog( params );
				for( const std::string& path : paths )
				{
					const uint32_t fileSize = ae::FileSystem::GetSize( path.c_str() );
					if( !fileSize )
					{
						continue;
					}
					ae::Array< uint8_t > fileData( TAG_ALL, 0, fileSize );
					ae::FileSystem::Read( path.c_str(), fileData.Data(), fileSize );
					int32_t width = 0, height = 0, channels = 0;
					uint8_t* decoded = stbi_load_from_memory(
						fileData.Data(), (int32_t)fileSize, &width, &height, &channels, 3 );
					if( !decoded )
					{
						AE_WARN( "Could not decode '#'", path.c_str() );
						continue;
					}
					TestImage* image = ae::New< TestImage >( TAG_ALL, TAG_ALL );
					image->width = (uint32_t)width;
					image->height = (uint32_t)height;
					image->source.AppendArray( decoded, (uint32_t)( width * height * 3 ) );
					const char* fileName = strrchr( path.c_str(), '/' );
					image->name = fileName ? ( fileName + 1 ) : path.c_str();
					stbi_image_free( decoded );
					testImages.Append( image );
					testImageIndex = (int32_t)testImages.Length() - 1;
				}
			}
			if( testImages.Length() )
			{
				ImGui::SameLine();
				if( ImGui::Button( "Clear" ) )
				{
					remapJob.Finish();
					for( TestImage* image : testImages )
					{
						ae::Delete( image );
					}
					testImages.Clear();
					testImageIndex = -1;
				}
			}

			const int32_t imageCount = (int32_t)testImages.Length();
			if( ImGui::Button( "<" ) )
			{
				testImageIndex = ( testImageIndex <= -1 ) ? ( imageCount - 1 ) : ( testImageIndex - 1 );
			}
			ImGui::SameLine();
			if( ImGui::Button( ">" ) )
			{
				testImageIndex = ( testImageIndex >= imageCount - 1 ) ? -1 : ( testImageIndex + 1 );
			}
			ImGui::SameLine();
			ImGui::Text( "%s", ( testImageIndex < 0 ) ? "procedural sheet"
				: testImages[ testImageIndex ]->name.c_str() );
			ImGui::SetNextItemWidth( 160.0f );
			ImGui::SliderFloat( "dither", &ditherStrength, 0.0f, 24.0f, "%.0f" );

			uint64_t signature = ae::Hash64().HashType( testImageIndex )
				.HashType( ditherStrength ).Get();
			for( uint32_t i = 0; i < kSlotCount; i++ )
			{
				const ae::Vec3 srgb = slots[ i ].color.GetSRGB();
				signature = ae::Hash64().HashType( signature ).HashType( srgb.x )
					.HashType( srgb.y ).HashType( srgb.z ).Get();
			}
			if( signature != remapSignature )
			{
				remapSignature = signature;
				remapDirty = true;
			}

			// The upload has to happen here, the GL context belongs to this thread.
			if( remapJob.IsComplete() )
			{
				remapJob.Finish();
				testPixels.Clear();
				testPixels.AppendArray( remapPixels.Data(), remapPixels.Length() );
				if( testTexture.GetTexture() )
				{
					testTexture.Terminate();
				}
				// bottomToTopData skips the vertical flip ae applies for GL's
				// bottom-left UV origin, which ImGui's top-left default undoes.
				ae::TextureParams params;
				params.data = testPixels.Data();
				params.width = remapWidth;
				params.height = remapHeight;
				params.format = ae::Texture::Format::RGB8_SRGB;
				params.type = ae::Texture::Type::UInt8;
				params.filter = ae::Texture::Filter::Nearest;
				params.wrap = ae::Texture::Wrap::Clamp;
				params.bottomToTopData = true;
				params.autoGenerateMipmaps = false;
				testTexture.Initialize( params );
			}

			if( remapDirty && !remapJob.IsRunning() )
			{
				remapDirty = false;
				remapWidth = kTestWidth;
				remapHeight = kTestHeight;
				if( testImageIndex >= 0 )
				{
					// Mapped over source, so the loss reads by looking down.
					remapWidth = testImages[ testImageIndex ]->width;
					remapHeight = testImages[ testImageIndex ]->height * 2;
				}
				// Sized here because ae's allocator expects its own thread.
				remapPixels.Clear();
				remapPixels.Append( (uint8_t)0, remapWidth * remapHeight * 3 );
				for( uint32_t i = 0; i < kSlotCount; i++ )
				{
					remapSlots[ i ] = slots[ i ];
				}
				const TestImage* image = ( testImageIndex >= 0 ) ? testImages[ testImageIndex ] : nullptr;
				const float dither = ditherStrength;
				uint8_t* target = remapPixels.Data();
				Slot* jobSlots = remapSlots;
				remapJob.Start( [ image, dither, target, jobSlots ]()
				{
					if( image )
					{
						const uint32_t half = image->width * image->height * 3;
						MapImage( *image, jobSlots, kSlotCount, dither, target );
						memcpy( target + half, image->source.Data(), half );
					}
					else
					{
						GenerateTestSheet( jobSlots, kSlotCount, dither, target );
					}
				} );
			}

			// The images scroll inside a child so the controls above stay put.
			ImGui::BeginChild( "##images", ImVec2( 0.0f, 0.0f ), ImGuiChildFlags_None,
				ImGuiWindowFlags_HorizontalScrollbar );
				if( testTexture.GetTexture() )
				{
					const float available = ImGui::GetContentRegionAvail().x;
					const float scale = ae::Min( 1.0f, available / testTexture.GetWidth() );
					const ImVec2 imageMin = ImGui::GetCursorScreenPos();
					ImGui::Image( (ImTextureID)(intptr_t)testTexture.GetTexture(),
						ImVec2( testTexture.GetWidth() * scale, testTexture.GetHeight() * scale ) );
					if( ImGui::IsItemHovered() && scale > 0.0f )
					{
						const ImVec2 mouse = ImGui::GetIO().MousePos;
						const uint32_t x = (uint32_t)ae::Clip( ( mouse.x - imageMin.x ) / scale,
							0.0f, testTexture.GetWidth() - 1.0f );
						const uint32_t y = (uint32_t)ae::Clip( ( mouse.y - imageMin.y ) / scale,
							0.0f, testTexture.GetHeight() - 1.0f );
						const uint32_t offset = ( y * testTexture.GetWidth() + x ) * 3;
						const ae::Color shown = ae::Color::SRGB8( testPixels[ offset ],
							testPixels[ offset + 1 ], testPixels[ offset + 2 ] );
						const ae::Vec3 shownLCh = ColorToLCh( shown );
						// A mapped pixel is always one of the slots, so name it.
						const char* slotName = "";
						for( uint32_t i = 0; i < kSlotCount; i++ )
						{
							if( slots[ i ].color == shown )
							{
								slotName = slots[ i ].name;
								break;
							}
						}
						ImGui::BeginTooltip();
						ImGui::Text( "%u, %u", x, y );
						ImGui::Text( "%s %s  L*%.1f C*%.1f h%.0f", ToHex( shown ).c_str(), slotName,
							shownLCh.x, shownLCh.y, shownLCh.z );
						if( testImageIndex >= 0 )
						{
							const TestImage& image = *testImages[ testImageIndex ];
							const bool onMapped = ( y < image.height );
							const uint32_t sourceRow = onMapped ? y : ( y - image.height );
							const uint32_t sourceOffset = ( sourceRow * image.width + x ) * 3;
							const ae::Color source = ae::Color::SRGB8( image.source[ sourceOffset ],
								image.source[ sourceOffset + 1 ], image.source[ sourceOffset + 2 ] );
							const ae::Vec3 sourceLCh = ColorToLCh( source );
							if( onMapped )
							{
								ImGui::Text( "source %s  L*%.1f C*%.1f h%.0f", ToHex( source ).c_str(),
									sourceLCh.x, sourceLCh.y, sourceLCh.z );
								ImGui::Text( "dE %.1f",
									( LChToLab( sourceLCh ) - LChToLab( shownLCh ) ).Length() );
							}
							else
							{
								ImGui::TextDisabled( "source" );
							}
						}
						ImGui::EndTooltip();
						if( ImGui::IsItemClicked() )
						{
							ae::SetClipboardText( ToHex( shown ).c_str() );
						}
					}
				}
			ImGui::EndChild();
		}
		ImGui::End();

		BeginPanel( "Schemes" );
		{
			ImGui::Checkbox( "show metrics", &compareAll );
			ImGui::Separator();
			for( uint32_t i = 0; i < countof( kSchemes ); i++ )
			{
				const Scheme& scheme = *kSchemes[ i ];
				ImGui::PushID( (int32_t)i );
				if( ImGui::Button( "Load" ) )
				{
					LoadScheme( scheme, slots );
					selected = 0;
				}
				ImGui::SameLine();
				ImGui::Text( "%-16s", scheme.name );
				ImGui::SameLine();
				const float strip = ae::Max( 6.0f, ( ImGui::GetContentRegionAvail().x - 8.0f ) / kSlotCount );
				const ImVec2 origin = ImGui::GetCursorScreenPos();
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				Slot scratch[ kSlotCount ];
				LoadScheme( scheme, scratch );
				uint32_t schemeOrder[ kSlotCount ];
				GetDisplayOrder( scratch, kSlotCount, sortByLightness, schemeOrder );
				for( uint32_t c = 0; c < kSlotCount; c++ )
				{
					drawList->AddRectFilled(
						ImVec2( origin.x + c * strip, origin.y ),
						ImVec2( origin.x + ( c + 1 ) * strip - 1.0f, origin.y + 20.0f ),
						ToImColor( scratch[ schemeOrder[ c ] ].color ) );
				}
				ImGui::Dummy( ImVec2( strip * kSlotCount, 20.0f ) );
				if( compareAll )
				{
					const Metrics schemeMetrics = MeasureScheme( scheme );
					ImGui::TextDisabled( "    range %.0f   worst gap %.0f   void %.0f   registers %u/%u/%u/%u",
						schemeMetrics.lightnessRange, schemeMetrics.worstValueGap, schemeMetrics.worstHueVoid,
						schemeMetrics.registerCounts[ 0 ], schemeMetrics.registerCounts[ 1 ],
						schemeMetrics.registerCounts[ 2 ], schemeMetrics.registerCounts[ 3 ] );
				}
				ImGui::PopID();
			}
			ImGui::Separator();
			ImGui::TextDisabled( "edited:  range %.0f   worst gap %.0f   void %.0f   registers %u/%u/%u/%u",
				metrics.lightnessRange, metrics.worstValueGap, metrics.worstHueVoid,
				metrics.registerCounts[ 0 ], metrics.registerCounts[ 1 ],
				metrics.registerCounts[ 2 ], metrics.registerCounts[ 3 ] );
		}
		ImGui::End();

		BeginPanel( "Solid" );
		{
			ImGui::SetNextItemWidth( 150.0f );
			ImGui::Combo( "blend", &solid.blendSpace, "linear RGB\0Lab\0LCh\0" );
			ImGui::SameLine();
			ImGui::Checkbox( "all groups", &solid.showAllGroups );
			const ImVec2 size = ImGui::GetContentRegionAvail();
			const uint32_t targetWidth = (uint32_t)ae::Max( 16.0f, size.x );
			const uint32_t targetHeight = (uint32_t)ae::Max( 16.0f, size.y );
			if( solid.viewportTarget.GetWidth() != targetWidth || solid.viewportTarget.GetHeight() != targetHeight )
			{
				solid.viewportTarget.Initialize( targetWidth, targetHeight );
				solid.viewportTarget.AddTexture( ae::Texture::Filter::Linear, ae::Texture::Wrap::Clamp );
				solid.viewportTarget.AddDepth( ae::Texture::Filter::Nearest, ae::Texture::Wrap::Clamp );
			}
			ImGui::Image( (ImTextureID)(intptr_t)solid.viewportTarget.GetTexture( 0 )->GetTexture(),
				ImVec2( (float)targetWidth, (float)targetHeight ), ImVec2( 0.0f, 1.0f ), ImVec2( 1.0f, 0.0f ) );
			solid.viewportHovered = ImGui::IsItemHovered();
		}
		ImGui::End();

		BeginPanel( "Groups" );
		{
			for( uint32_t i = 0; i < kMaxGroups; i++ )
			{
				ColorGroup& group = solid.groups[ i ];
				ImGui::PushID( (int32_t)i );
				ImGui::Checkbox( "##visible", &group.visible );
				ImGui::SameLine();
				if( ImGui::RadioButton( "##sel", solid.selectedGroup == (int32_t)i ) )
				{
					solid.selectedGroup = (int32_t)i;
				}
				ImGui::SameLine();
				ImGui::SetNextItemWidth( 110.0f );
				ImGui::InputText( "##name", group.name, sizeof( group.name ) );
				ImGui::SameLine();
				ImGui::Text( "%d", group.GetCount() );
				ImGui::PopID();
			}
			ImGui::Separator();
			ImGui::Text( "members of '%s'", solid.groups[ solid.selectedGroup ].name );
			// Click a swatch to add or remove the slot from the selected group.
			const float cell = ae::Max( 16.0f,
				( ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 7.0f ) / 8.0f );
			for( uint32_t i = 0; i < kSlotCount; i++ )
			{
				const uint16_t bit = (uint16_t)( 1 << i );
				const bool member = ( solid.groups[ solid.selectedGroup ].members & bit ) != 0;
				ImGui::PushID( 100 + (int32_t)i );
				if( ImGui::ColorButton( "##m", ToImVec4( slots[ i ].color ),
					ImGuiColorEditFlags_NoTooltip, ImVec2( cell, cell ) ) )
				{
					solid.groups[ solid.selectedGroup ].members ^= bit;
				}
				if( member )
				{
					ImGui::GetWindowDrawList()->AddRect( ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
						IM_COL32( 255, 255, 255, 255 ), 0.0f, 0, 2.5f );
				}
				if( ImGui::IsItemHovered() )
				{
					DrawColorTooltip( slots[ i ], slots, kSlotCount );
				}
				ImGui::PopID();
				if( ( i % kSlotColumns ) != ( kSlotColumns - 1 ) )
				{
					ImGui::SameLine();
				}
			}
		}
		ImGui::End();

		// The selected color is mirrored to the clipboard, so a click, a drag or
		// a slider all leave it ready to paste.
		if( selected >= 0 && slots[ selected ].color != lastCopiedColor )
		{
			lastCopiedColor = slots[ selected ].color;
			ae::SetClipboardText( ToHex( lastCopiedColor ).c_str() );
		}

		// Cylinder: L* up the axis, chroma as radius, hue as angle.
		solid.camera.SetInputEnabled( solid.viewportHovered );
		solid.camera.Update( &input, dt );
		solid.debugLines.Clear();
		{
			const ae::Color guide = ae::Color::SRGB8( 70, 78, 90 );
			for( uint32_t i = 0; i <= 4; i++ )
			{
				const float z = i * 25.0f * 0.02f;
				solid.debugLines.AddCircle( ae::Vec3( 0.0f, 0.0f, z ), ae::Vec3( 0.0f, 0.0f, 1.0f ),
					60.0f * 0.02f, guide, 48 );
			}
			solid.debugLines.AddLine( ae::Vec3( 0.0f ), ae::Vec3( 0.0f, 0.0f, 2.0f ), guide );
			for( uint32_t i = 0; i < 12; i++ )
			{
				const float radians = ae::DegToRad( i * 30.0f );
				solid.debugLines.AddLine( ae::Vec3( 0.0f ),
					ae::Vec3( std::cos( radians ), std::sin( radians ), 0.0f ) * ( 60.0f * 0.02f ), guide );
			}

			for( uint32_t g = 0; g < kMaxGroups; g++ )
			{
				const ColorGroup& group = solid.groups[ g ];
				if( !group.members || !group.visible )
				{
					continue;
				}
				if( !solid.showAllGroups && (int32_t)g != solid.selectedGroup )
				{
					continue;
				}
				// Ordered by lightness, then the real blend is sampled between
				// each pair so the line is the path, not a curve fitted to it.
				ae::Array< uint32_t, kSlotCount > ordered;
				for( uint32_t i = 0; i < kSlotCount; i++ )
				{
					if( group.members & ( 1 << i ) )
					{
						ordered.Append( i );
					}
				}
				std::sort( ordered.begin(), ordered.end(), [ &slots ]( uint32_t a, uint32_t b )
				{
					return slots[ a ].lch.x < slots[ b ].lch.x;
				} );
				for( uint32_t i = 0; i + 1 < ordered.Length(); i++ )
				{
					const ae::Color from = slots[ ordered[ i ] ].color;
					const ae::Color to = slots[ ordered[ i + 1 ] ].color;
					const uint32_t steps = 24;
					ae::Vec3 previous = LChToCylinder( slots[ ordered[ i ] ].lch );
					for( uint32_t s = 1; s <= steps; s++ )
					{
						const float blend = (float)s / steps;
						const ae::Color sample = BlendColors( from, to, blend, (BlendSpace)solid.blendSpace );
						const ae::Vec3 point = LChToCylinder( ColorToLCh( sample ) );
						solid.debugLines.AddLine( previous, point, sample );
						previous = point;
					}
				}
			}

			for( uint32_t i = 0; i < kSlotCount; i++ )
			{
				const ae::Vec3 point = LChToCylinder( slots[ i ].lch );
				solid.debugLines.AddSphere( point, ( (int32_t)i == selected ) ? 0.075f : 0.05f,
					slots[ i ].color, 8 );
			}
		}
		if( solid.viewportTarget.GetWidth() )
		{
			const ae::Matrix4 worldToView = ae::Matrix4::WorldToView(
				solid.camera.GetPosition(), solid.camera.GetForward(), solid.camera.GetUp() );
			const ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection(
				0.9f, solid.viewportTarget.GetAspectRatio(), solid.camera.GetMinDistance() * 0.5f, 100.0f );
			solid.viewportTarget.Activate();
			solid.viewportTarget.Clear( ground );
			solid.debugLines.Render( viewToProj * worldToView );
		}

		render.Activate();
		render.Clear( ground );
		ui.Render();
		render.Present();
		timeStep.Tick();
		return !input.quit;
	};

	auto Terminate = [&]() -> int32_t
	{
		AE_INFO( "Terminate" );
		remapJob.Finish();
		solid.debugLines.Terminate();
		solid.viewportTarget.Terminate();
		for( TestImage* image : testImages )
		{
			ae::Delete( image );
		}
		testImages.Clear();
		testTexture.Terminate();
		input.Terminate();
		render.Terminate();
		window.Terminate();
		return 0;
	};

	return ae::Application( argc, argv, Initialize, Update, Terminate );
}
