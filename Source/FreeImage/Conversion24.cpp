// ==========================================================
// Bitmap conversion routines
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
// - Dale Larson (dlarson@norsesoft.com)
// - Herv� Drolon (drolon@infonie.fr)
// - Jani Kajala (janik@remedy.fi)
//
// This file is part of FreeImage 3
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================

#include "FreeImage.h"
#include "Utilities.h"

// ----------------------------------------------------------
//  internal conversions X to 24 bits
// ----------------------------------------------------------

void DLL_CALLCONV
FreeImage_ConvertLine1To24(BYTE *target, BYTE *source, int width_in_pixels, RGBQUAD *palette) {
	for (int cols = 0; cols < width_in_pixels; cols++) {
		BYTE index = (source[cols >> 3] & (0x80 >> (cols & 0x07))) != 0 ? 1 : 0;

		target[FI_RGBA_BLUE] = palette[index].rgbBlue;
		target[FI_RGBA_GREEN] = palette[index].rgbGreen;
		target[FI_RGBA_RED] = palette[index].rgbRed;

		target += 3;
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine4To24(BYTE *target, BYTE *source, int width_in_pixels, RGBQUAD *palette) {
	BOOL low_nibble = FALSE;
	int x = 0;

	for (int cols = 0; cols < width_in_pixels; ++cols ) {
		if (low_nibble) {
			target[FI_RGBA_BLUE] = palette[LOWNIBBLE(source[x])].rgbBlue;
			target[FI_RGBA_GREEN] = palette[LOWNIBBLE(source[x])].rgbGreen;
			target[FI_RGBA_RED] = palette[LOWNIBBLE(source[x])].rgbRed;

			x++;
		} else {
			target[FI_RGBA_BLUE] = palette[HINIBBLE(source[x]) >> 4].rgbBlue;
			target[FI_RGBA_GREEN] = palette[HINIBBLE(source[x]) >> 4].rgbGreen;
			target[FI_RGBA_RED] = palette[HINIBBLE(source[x]) >> 4].rgbRed;
		}

		low_nibble = !low_nibble;

		target += 3;
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine8To24(BYTE *target, BYTE *source, int width_in_pixels, RGBQUAD *palette) {
	for (int cols = 0; cols < width_in_pixels; cols++) {
		target[FI_RGBA_BLUE] = palette[source[cols]].rgbBlue;
		target[FI_RGBA_GREEN] = palette[source[cols]].rgbGreen;
		target[FI_RGBA_RED] = palette[source[cols]].rgbRed;

		target += 3;
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine16To24_555(BYTE *target, BYTE *source, int width_in_pixels) {
	WORD *bits = (WORD *)source;

	for (int cols = 0; cols < width_in_pixels; cols++) {
		target[FI_RGBA_RED] = (((bits[cols] & FI16_555_RED_MASK) >> FI16_555_RED_SHIFT) * 0xFF) / 0x1F;
		target[FI_RGBA_GREEN] = (((bits[cols] & FI16_555_GREEN_MASK) >> FI16_555_GREEN_SHIFT) * 0xFF) / 0x1F;
		target[FI_RGBA_BLUE] = (((bits[cols] & FI16_555_BLUE_MASK) >> FI16_555_BLUE_SHIFT) * 0xFF) / 0x1F;

		target += 3;
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine16To24_565(BYTE *target, BYTE *source, int width_in_pixels) {
	WORD *bits = (WORD *)source;

	for (int cols = 0; cols < width_in_pixels; cols++) {
		target[FI_RGBA_RED] = (((bits[cols] & FI16_565_RED_MASK) >> FI16_565_RED_SHIFT) * 0xFF) / 0x1F;
		target[FI_RGBA_GREEN] = (((bits[cols] & FI16_565_GREEN_MASK) >> FI16_565_GREEN_SHIFT) * 0xFF) / 0x3F;
		target[FI_RGBA_BLUE] = (((bits[cols] & FI16_565_BLUE_MASK) >> FI16_565_BLUE_SHIFT) * 0xFF) / 0x1F;

		target += 3;
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine32To24(BYTE *target, BYTE *source, int width_in_pixels) {
	for (int cols = 0; cols < width_in_pixels; cols++) {
		target[FI_RGBA_BLUE] = source[FI_RGBA_BLUE];
		target[FI_RGBA_GREEN] = source[FI_RGBA_GREEN];
		target[FI_RGBA_RED] = source[FI_RGBA_RED];

		target += 3;
		source += 4;
	}
}

// ----------------------------------------------------------
//   smart convert X to 24 bits
// ----------------------------------------------------------

FIBITMAP * DLL_CALLCONV
FreeImage_ConvertTo24Bits(FIBITMAP *dib) {
	if(!dib) return NULL;

	int bpp = FreeImage_GetBPP(dib);

	if (bpp != 24) {
		int width = FreeImage_GetWidth(dib);
		int height = FreeImage_GetHeight(dib);

		switch(bpp) {
			case 1 :
			{
				FIBITMAP *new_dib = FreeImage_Allocate(width, height, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);

				if (new_dib != NULL)
					for (int rows = 0; rows < height; rows++)
						FreeImage_ConvertLine1To24(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib));					
				
				return new_dib;
			}

			case 4 :
			{
				FIBITMAP *new_dib = FreeImage_Allocate(width, height, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);

				if (new_dib != NULL)
					for (int rows = 0; rows < height; rows++)
						FreeImage_ConvertLine4To24(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib));
			
				return new_dib;
			}
				
			case 8 :
			{
				FIBITMAP *new_dib = FreeImage_Allocate(width, height, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);

				if (new_dib != NULL)
					for (int rows = 0; rows < height; rows++)
						FreeImage_ConvertLine8To24(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib));
									
				return new_dib;
			}

			case 16 :
			{
				FIBITMAP *new_dib = FreeImage_Allocate(width, height, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);

				if (new_dib != NULL) {
					for (int rows = 0; rows < height; rows++) {
						if ((FreeImage_GetRedMask(dib) == FI16_565_RED_MASK) && (FreeImage_GetGreenMask(dib) == FI16_565_GREEN_MASK) && (FreeImage_GetBlueMask(dib) == FI16_565_BLUE_MASK)) {
							FreeImage_ConvertLine16To24_565(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width);
						} else {
							// includes case where all the masks are 0
							FreeImage_ConvertLine16To24_555(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width);
						}
					}
				}

				return new_dib;
			}

			case 32 :
			{
				FIBITMAP *new_dib = FreeImage_Allocate(width, height, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);

				if (new_dib != NULL)
					for (int rows = 0; rows < height; rows++)
						FreeImage_ConvertLine32To24(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width);
				
				return new_dib;
			}
		}
	}

	return FreeImage_Clone(dib);
}
