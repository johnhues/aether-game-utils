// ae::Color
#define AE_MAIN
#include "aether.h"

int main()
{
	//! [Palette]
	const ae::Color slots[] = {
		ae::Color::AetherDarkBlue(),
		ae::Color::AetherRed(),
		ae::Color::AetherYellow(),
	};
	for( const ae::Color& slot : slots )
	{
		const ae::Vec3 srgb = slot.GetSRGB() * 255.0f;
		AE_INFO( "sRGB #, #, #", (int)srgb.x, (int)srgb.y, (int)srgb.z );
	}
	//! [Palette]
	return 0;
}
