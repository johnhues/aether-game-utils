// ae::Array
#define AE_MAIN
#include "aether.h"

int main()
{
	//! [Append]
	ae::Array< const char*, 8 > tools;
	tools.Append( "hammer" );
	tools.Append( "chisel" );
	tools.Append( "rasp" );
	for( const char* tool : tools )
	{
		AE_INFO( "tool '#'", tool );
	}
	//! [Append]
	return 0;
}
