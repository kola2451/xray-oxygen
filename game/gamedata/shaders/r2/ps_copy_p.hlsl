#include "common.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Pixel
half4 	main	( float4 tc : TEXCOORD0 )	: COLOR
{
	return	tex2Dproj	(s_base,tc);
}