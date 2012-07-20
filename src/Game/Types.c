#include "Types.h"


Uint16 SwapShort( Uint16 val )
{
	return( Uint16 )( ( val << 8 ) | ( val >> 8 ) );
}


Uint32 SwapLong( Uint32 val )
{
	return( ( val << 24 ) | ( ( val << 8 ) & 0x00ff0000 ) | ( ( val >> 8 ) & 0x0000ff00 ) | ( val >> 24 ) );
}


float SwapFloat( float val )
{
	union
	{
		float f;
		Uint32 i;
	} swap;

	swap.f = val;
	swap.i = SwapLong( swap.i );
	return swap.f;
}
