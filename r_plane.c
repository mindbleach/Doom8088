/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2023 Frenkel Smeijers
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Render floor and ceilings
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "compiler.h"
#include "r_defs.h"
#include "r_main.h"

#include "globdata.h"


static int16_t firstflat;
static int16_t  animated_flat_basepic;
int16_t __far* flattranslation;             // for global animation (extern'd in r_main.h for FLAT_SPAN in r_draw - mindbleach)


#if defined FLAT_SPAN
static int16_t animated_flat_basepic_color[3];
#else
static fixed_t planeheight;
static fixed_t basexscale, baseyscale;
#endif


//
// R_DrawSpan
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//

#if !defined FLAT_SPAN
inline static void R_DrawSpanPixel(uint32_t __far* dest, const byte __far* source, const byte* colormap, uint32_t position)
{
    uint16_t color = colormap[source[((position >> 4) & 0x0fc0) | (position >> 26)]];
    color = color | (color << 8);

    uint16_t __far* d = (uint16_t __far*) dest;
    *d++ = color;
    *d   = color;
}


typedef struct {
  uint32_t            position;
  uint32_t            step;
  const byte          __far* source; // start of a 64*64 tile image
  const lighttable_t  *colormap;
} draw_span_vars_t;


static void R_DrawSpan(uint16_t y, uint16_t x1, uint16_t x2, const draw_span_vars_t *dsvars)
{
    uint16_t count = (x2 - x1);

    const byte __far* source = dsvars->source;
    const byte *colormap = dsvars->colormap;

    uint32_t __far* dest = (uint32_t __far*)(_g_screen + (y * SCREENWIDTH) + (x1 << 2));

    const uint32_t step = dsvars->step;
    uint32_t position = dsvars->position;

    uint16_t l = (count >> 4);

    while (l--)
    {
        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;

        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;

        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;

        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
    }

    switch (count & 15)
    {
        case 15:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 14:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 13:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 12:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 11:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 10:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case  9:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case  8:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case  7:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case  6:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case  5:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case  4:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case  3:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case  2:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case  1:    R_DrawSpanPixel(dest, source, colormap, position);
    }
}


static const fixed_t yslopeTable[VIEWWINDOWHEIGHT / 2] =
{
    132104,134218,136400,138655,140985,143395,145889,148471,151146,153919,156796,159783,162886,166111,
    169467,172961,176602,180400,184365,188508,192842,197379,202135,207126,212370,217886,223696,229825,
    236299,243148,250406,258111,266305,275036,284360,294337,305040,316551,328965,342392,356962,372827,
    390168,409200,430185,453438,479349,508400,541201,578525,621378,671089,729444,798915,883011,986895,
    1118481,1290555,1525201,1864135,2396745,3355443,5592405,16777216
};

static const uint16_t distscaleTable[VIEWWINDOWWIDTH] =
{
    0x6A75,
    0x6438,0x5E8C,0x58AB,0x5319,0x4D63,0x4800,0x42B6,0x3D8E,
    0x38B2,0x33CF,0x2F3C,0x2AAB,0x266B,0x2237,0x1E54,0x1AA1,
    0x1721,0x13C1,0x10C4,0x0DEB,0x0B4E,0x08FD,0x06E8,0x051D,
    0x0392,0x0249,0x014A,0x0094,0x0025,0x0001,0x0023,0x0091,
    0x0145,0x0242,0x038A,0x0514,0x06DC,0x08EF,0x0B3E,0x0DDA,
    0x10B0,0x13AC,0x1709,0x1A87,0x1E37,0x2218,0x264A,0x2A88,
    0x2F16,0x33A7,0x3887,0x3D60,0x4286,0x47CB,0x4D2D,0x52DF,
    0x586E,0x5E4A,0x63F4,
};

static fixed_t yslope(uint8_t y)
{
	if (y >= 64)
		y = 127 - y;
	return yslopeTable[y];
}

static fixed_t distscale(uint8_t x)
{
	return 0x010000 | distscaleTable[x];
}


static void R_MapPlane(uint16_t y, uint16_t x1, uint16_t x2, draw_span_vars_t *dsvars)
{    
    const fixed_t distance = FixedMul(planeheight, yslope(y));
    dsvars->step = ((FixedMul(distance,basexscale) << 10) & 0xffff0000) | ((FixedMul(distance,baseyscale) >> 6) & 0x0000ffff);

    fixed_t length = FixedMul (distance, distscale(x1));
    angle_t angle = (viewangle + xtoviewangle(x1))>>ANGLETOFINESHIFT;

    // killough 2/28/98: Add offsets
    uint32_t xfrac =  viewx + FixedMul(finecosine(angle), length);
    uint32_t yfrac = -viewy - FixedMul(finesine(  angle), length);

    dsvars->position = ((xfrac << 10) & 0xffff0000) | ((yfrac >> 6)  & 0x0000ffff);

    R_DrawSpan(y, x1, x2, dsvars);
}


//
// R_MakeSpans
//

static void R_MakeSpans(int16_t x, uint16_t t1, uint16_t b1, uint16_t t2, uint16_t b2, draw_span_vars_t *dsvars)
{
    static byte spanstart[VIEWWINDOWHEIGHT];

    for (; t1 < t2 && t1 <= b1; t1++)
        R_MapPlane(t1, spanstart[t1], x, dsvars);

    for (; b1 > b2 && b1 >= t1; b1--)
        R_MapPlane(b1, spanstart[b1], x, dsvars);

    while (t2 < t1 && t2 <= b2)
        spanstart[t2++] = x;

    while (b2 > b1 && b2 >= t2)
        spanstart[b2--] = x;
}
#endif


static void R_DoDrawPlane(visplane_t __far* pl)
{
    if (pl->minx <= pl->maxx)
    {
        if (pl->picnum == skyflatnum)
        {
            // sky flat
            R_DrawSky(pl);
        }
        else
        {
            // regular flat
#if defined FLAT_SPAN
            draw_column_vars_t dcvars;

            byte color = R_GetColorMapColor(pl->lightlevel, flattranslation[pl->picnum]);

            for (int16_t x = pl->minx; x <= pl->maxx; x++)
            {
                if (pl->top[x] <= pl->bottom[x])
                {
                    dcvars.x = x;
                    dcvars.yl = pl->top[x];
                    dcvars.yh = pl->bottom[x];
                    R_DrawColumnFlat(color, &dcvars);
                }
            }
#else
            const int16_t stop = pl->maxx + 1;

            pl->top[pl->minx - 1] = pl->top[stop] = 0xff; // dropoff overflow

            draw_span_vars_t dsvars;

            dsvars.source   = W_GetLumpByNum(firstflat + flattranslation[pl->picnum]);
            dsvars.colormap = R_LoadColorMap(pl->lightlevel);

            planeheight = D_abs(pl->height - viewz);

            for (register int16_t x = pl->minx; x <= stop; x++)
            {
                R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1], pl->top[x], pl->bottom[x], &dsvars);
            }

            Z_ChangeTagToCache(dsvars.source);
#endif
        }
    }
}


//
// RDrawPlanes
// At the end of each frame.
//

void R_DrawPlanes (void)
{
    for (int8_t i = 0; i < MAXVISPLANES; i++)
    {
        visplane_t __far* pl = _g_visplanes[i];

        while(pl)
        {
            if(pl->modified)
                R_DoDrawPlane(pl);

            pl = pl->next;
        }
    }
}


//
// R_ClearPlanes
// At begining of frame.
//

void R_ClearPlanes(void)
{
    // opening / clipping determination
    for (int8_t i = 0; i < VIEWWINDOWWIDTH; i++)
        floorclip[i] = VIEWWINDOWHEIGHT, ceilingclip[i] = -1;


    for (int8_t i = 0; i < MAXVISPLANES; i++)
        for (*freehead = _g_visplanes[i], _g_visplanes[i] = NULL; *freehead; )
            freehead = &(*freehead)->next;

    R_ClearOpenings();

#if !defined FLAT_SPAN
    static const fixed_t iprojection = (1L << FRACBITS) / (VIEWWINDOWWIDTH / 2);

    basexscale = FixedMul(viewsin,iprojection);
    baseyscale = FixedMul(viewcos,iprojection);
#endif
}


//
// R_InitFlats
//

void R_InitFlats(void)
{
	firstflat        = W_GetNumForName("F_START") + 1;

	int16_t lastflat = W_GetNumForName("F_END")   - 1;
	int16_t numflats = lastflat - firstflat + 1;

	// Create translation table for global animation.

	flattranslation = Z_MallocStatic((numflats + 1) * sizeof(*flattranslation));

	animated_flat_basepic = R_FlatNumForName("NUKAGE1");

#if defined FLAT_SPAN
	byte __far* source = Z_MallocStatic(64 * 64);

	for (int16_t i = 0; i < numflats; i++)
	{
		W_ReadLumpByNum(firstflat + i, source);
		flattranslation[i] = source[(64 / 2) * 64 + (64 / 2)];
	}

	Z_Free(source);

	animated_flat_basepic_color[0] = flattranslation[animated_flat_basepic + 0];
	animated_flat_basepic_color[1] = flattranslation[animated_flat_basepic + 1];
	animated_flat_basepic_color[2] = flattranslation[animated_flat_basepic + 2];
#else
	for (int16_t i = 0; i < numflats; i++)
		flattranslation[i] = i;
#endif
}


//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
//

int16_t R_FlatNumForName(const char *name)
{
	int16_t i = W_GetNumForName(name);
	return i - firstflat;
}


void P_UpdateAnimatedFlat(void)
{
#if defined FLAT_SPAN
	int16_t pic = animated_flat_basepic_color[(_g_leveltime >> 3) % 3];
#else
	int16_t pic = animated_flat_basepic + ((_g_leveltime >> 3) % 3);
#endif

	flattranslation[animated_flat_basepic + 0] = pic;
	flattranslation[animated_flat_basepic + 1] = pic;
	flattranslation[animated_flat_basepic + 2] = pic;
}
