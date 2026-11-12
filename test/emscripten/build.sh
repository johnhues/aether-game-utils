#!/usr/bin/env bash
# Builds Triangle.cpp with Emscripten, warnings as errors, and fails if aether
# imports anything Emscripten does not provide. Requires em++ on the PATH (see
# emsdk_env.sh) and node. Usage: test/emscripten/build.sh [output dir]
set -euo pipefail
here="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
root="$( cd "$here/../.." && pwd )"
out="${1:-$root/build_emscripten_test}"
mkdir -p "$out"

# Default warnings, plus the ones aether's CMake targets treat as errors
em++ -std=c++20 -O2 -fno-exceptions -Werror \
	-Wreturn-type -Wimplicit-fallthrough -Wnarrowing -Wuninitialized -Wreorder -Wmismatched-tags -Wimplicit-int-conversion \
	-I"$root" "$here/Triangle.cpp" "$root/aether.cpp" \
	-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sALLOW_MEMORY_GROWTH=1 -sENVIRONMENT=web \
	--shell-file "$here/shell.html" -o "$out/index.html"

node -e '
const m = new WebAssembly.Module( require( "fs" ).readFileSync( process.argv[ 1 ] ) );
const foreign = WebAssembly.Module.imports( m ).filter( ( i ) => i.module !== "env" && i.module !== "wasi_snapshot_preview1" );
if( foreign.length )
{
	console.error( "aether imports outside of Emscripten: " + foreign.map( ( i ) => i.module + "." + i.name ).join( " " ) );
	process.exit( 1 );
}
' "$out/index.wasm"
echo "Built $out/index.html"
