// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  r_draw.c
/// \brief span / column drawer functions, for 8bpp and 16bpp
///        All drawing to the view buffer is accomplished in this file.
///        The other refresh files only know about ccordinates,
///        not the architecture of the frame buffer.
///        The frame buffer is a linear one, and we need only the base address.

#include "doomdef.h"
#include "doomstat.h"
#include "r_local.h"
#include "st_stuff.h" // need ST_HEIGHT
#include "i_video.h"
#include "v_video.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"
#include "console.h" // Until buffering gets finished

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

// ==========================================================================
//                     COMMON DATA FOR 8bpp AND 16bpp
// ==========================================================================

/**	\brief view info
*/
INT32 viewwidth, scaledviewwidth, viewheight, viewwindowx, viewwindowy;

/**	\brief pointer to the start of each line of the screen,
*/
UINT8 *ylookup[MAXVIDHEIGHT*4];

/**	\brief pointer to the start of each line of the screen, for view1 (splitscreen)
*/
UINT8 *ylookup1[MAXVIDHEIGHT*4];

/**	\brief pointer to the start of each line of the screen, for view2 (splitscreen)
*/
UINT8 *ylookup2[MAXVIDHEIGHT*4];

/**	\brief  x byte offset for columns inside the viewwindow,
	so the first column starts at (SCRWIDTH - VIEWWIDTH)/2
*/
INT32 columnofs[MAXVIDWIDTH*4];

UINT8 *topleft;

// =========================================================================
//                      COLUMN DRAWING CODE STUFF
// =========================================================================

lighttable_t *dc_colormap;
INT32 dc_x = 0, dc_yl = 0, dc_yh = 0;

fixed_t dc_iscale, dc_texturemid;
UINT8 dc_hires; // under MSVC boolean is a byte, while on other systems, it a bit,
               // soo lets make it a byte on all system for the ASM code
UINT8 *dc_source;

// -----------------------
// translucency stuff here
// -----------------------
#define NUMTRANSTABLES 9 // how many translucency tables are used

UINT8 *transtables; // translucency tables

/**	\brief R_DrawTransColumn uses this
*/
UINT8 *dc_transmap; // one of the translucency tables

// ----------------------
// translation stuff here
// ----------------------


/**	\brief R_DrawTranslatedColumn uses this
*/
UINT8 *dc_translation;

struct r_lightlist_s *dc_lightlist = NULL;
INT32 dc_numlights = 0, dc_maxlights, dc_texheight;

// =========================================================================
//                      SPAN DRAWING CODE STUFF
// =========================================================================

INT32 ds_y, ds_x1, ds_x2;
lighttable_t *ds_colormap;
fixed_t ds_xfrac, ds_yfrac, ds_xstep, ds_ystep;

UINT8 *ds_source; // start of a 64*64 tile image
UINT8 *ds_transmap; // one of the translucency tables

#ifdef ESLOPE
pslope_t *ds_slope; // Current slope being used
floatv3_t ds_su, ds_sv, ds_sz; // Vectors for... stuff?
float focallengthf, zeroheight;
#endif

/**	\brief Variable flat sizes
*/

UINT32 nflatxshift, nflatyshift, nflatshiftup, nflatmask;

// ==========================================================================
//                        OLD DOOM FUZZY EFFECT
// ==========================================================================

// =========================================================================
//                   TRANSLATION COLORMAP CODE
// =========================================================================

#define DEFAULT_TT_CACHE_INDEX MAXSKINS
#define BOSS_TT_CACHE_INDEX (MAXSKINS + 1)
#define METALSONIC_TT_CACHE_INDEX (MAXSKINS + 2)
#define ALLWHITE_TT_CACHE_INDEX (MAXSKINS + 3)
#define SKIN_RAMP_LENGTH 16
#define DEFAULT_STARTTRANSCOLOR 160
#define NUM_PALETTE_ENTRIES 256

static UINT8** translationtablecache[MAXSKINS + 4] = {NULL};


// See also the enum skincolors_t
// TODO Callum: Can this be translated?
const char *Color_Names[MAXSKINCOLORS] =
{
	"None",      // SKINCOLOR_NONE
	"White",     // SKINCOLOR_WHITE
	"Silver",    // SKINCOLOR_SILVER
	"Grey",      // SKINCOLOR_GREY
	"Black",     // SKINCOLOR_BLACK
	"Cyan",      // SKINCOLOR_CYAN
	"Aqua",      // SKINCOLOR_TEAL
	"Slate",		 // SKINCOLOR_STEELBLUE
	"Sapphire",  // SKINCOLOR_BLUE
	"Peach",     // SKINCOLOR_PEACH
	"Tan",       // SKINCOLOR_TAN
	"Bubblegum", // SKINCOLOR_PINK
	"Lavender",  // SKINCOLOR_LAVENDER
	"Purple",    // SKINCOLOR_PURPLE
	"Orange",    // SKINCOLOR_ORANGE
	"Rosewood",  // SKINCOLOR_ROSEWOOD
	"Beige",     // SKINCOLOR_BEIGE
	"Brown",     // SKINCOLOR_BROWN
	"Red",       // SKINCOLOR_RED
	"Crimson",   // SKINCOLOR_DARKRED
	"Mint",			 // SKINCOLOR_NEONGREEN
	"Green",     // SKINCOLOR_GREEN
	"Zim",       // SKINCOLOR_ZIM
	"Olive",     // SKINCOLOR_OLIVE
	"Yellow",    // SKINCOLOR_YELLOW
	"Gold",       // SKINCOLOR_GOLD
};

const UINT8 Color_Opposite[MAXSKINCOLORS*2] =
{
	SKINCOLOR_NONE,8,   // SKINCOLOR_NONE
	SKINCOLOR_BLACK,10, // SKINCOLOR_WHITE
	SKINCOLOR_GREY,4,   // SKINCOLOR_SILVER
	SKINCOLOR_SILVER,12,// SKINCOLOR_GREY
	SKINCOLOR_WHITE,8,  // SKINCOLOR_BLACK
	SKINCOLOR_NONE,8,   // SKINCOLOR_CYAN
	SKINCOLOR_NONE,8,   // SKINCOLOR_TEAL
	SKINCOLOR_NONE,8,   // SKINCOLOR_STEELBLUE
	SKINCOLOR_ORANGE,9, // SKINCOLOR_BLUE
	SKINCOLOR_NONE,8,   // SKINCOLOR_PEACH
	SKINCOLOR_NONE,8,   // SKINCOLOR_TAN
	SKINCOLOR_NONE,8,   // SKINCOLOR_PINK
	SKINCOLOR_NONE,8,   // SKINCOLOR_LAVENDER
	SKINCOLOR_NONE,8,   // SKINCOLOR_PURPLE
	SKINCOLOR_BLUE,12,  // SKINCOLOR_ORANGE
	SKINCOLOR_NONE,8,   // SKINCOLOR_ROSEWOOD
	SKINCOLOR_NONE,8,   // SKINCOLOR_BEIGE
	SKINCOLOR_NONE,8,   // SKINCOLOR_BROWN
	SKINCOLOR_GREEN,5,  // SKINCOLOR_RED
	SKINCOLOR_NONE,8,   // SKINCOLOR_DARKRED
	SKINCOLOR_NONE,8,   // SKINCOLOR_NEONGREEN
	SKINCOLOR_RED,11,   // SKINCOLOR_GREEN
	SKINCOLOR_PURPLE,3, // SKINCOLOR_ZIM
	SKINCOLOR_NONE,8,   // SKINCOLOR_OLIVE
	SKINCOLOR_NONE,8,   // SKINCOLOR_YELLOW
	SKINCOLOR_NONE,8    // SKINCOLOR_GOLD
};

CV_PossibleValue_t Color_cons_t[MAXSKINCOLORS+1];

/**	\brief The R_InitTranslationTables

  load in color translation tables
*/
void R_InitTranslationTables(void)
{
#ifdef _NDS
	// Ugly temporary NDS hack.
	transtables = (UINT8*)0x2000000;
#else
	// Load here the transparency lookup tables 'TINTTAB'
	// NOTE: the TINTTAB resource MUST BE aligned on 64k for the asm
	// optimised code (in other words, transtables pointer low word is 0)
	transtables = Z_MallocAlign(NUMTRANSTABLES*0x10000, PU_STATIC,
		NULL, 16);

	W_ReadLump(W_GetNumForName("TRANS10"), transtables);
	W_ReadLump(W_GetNumForName("TRANS20"), transtables+0x10000);
	W_ReadLump(W_GetNumForName("TRANS30"), transtables+0x20000);
	W_ReadLump(W_GetNumForName("TRANS40"), transtables+0x30000);
	W_ReadLump(W_GetNumForName("TRANS50"), transtables+0x40000);
	W_ReadLump(W_GetNumForName("TRANS60"), transtables+0x50000);
	W_ReadLump(W_GetNumForName("TRANS70"), transtables+0x60000);
	W_ReadLump(W_GetNumForName("TRANS80"), transtables+0x70000);
	W_ReadLump(W_GetNumForName("TRANS90"), transtables+0x80000);
#endif
}


/**	\brief	Generates a translation colormap.

	\param	dest_colormap	colormap to populate
	\param	skinnum		number of skin, TC_DEFAULT or TC_BOSS
	\param	color		translation color

	\return	void
*/
static void R_GenerateTranslationColormap(UINT8 *dest_colormap, INT32 skinnum, UINT8 color)
{
    // Lach - 7 September 2019
    const UINT8 skincolorramps[MAXSKINCOLORS][SKIN_RAMP_LENGTH] = {
				{120, 120, 120, 120,   0,   2,   5,   8,   9,  11,  14,  17,  20,  22,  25,  28}, // SKINCOLOR_WHITE (Kart)
    //  {  0,   0,   1,   1,   2,   2,   3,   3,   4,   4,   5,   5,   6,   6,   7,   7}, // SKINCOLOR_WHITE (Old)
        {  3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18}, // SKINCOLOR_SILVER
        {  8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23}, // SKINCOLOR_GREY
        { 24,  24,  25,  25,  26,  26,  27,  27,  28,  28,  29,  29,  30,  30,  31,  31}, // SKINCOLOR_BLACK
        {208, 208, 209, 210, 211, 211, 212, 213, 214, 214, 215, 216, 217, 217, 218, 219}, // SKINCOLOR_CYAN
				{120, 208, 208, 210, 212, 214, 220, 220, 220, 221, 221, 222, 222, 223, 223, 191}, // SKINCOLOR_TEAL (Kart Aqua)
    //  {247, 247, 247, 247, 220, 220, 220, 221, 221, 221, 222, 222, 222, 223, 223, 223}, // SKINCOLOR_TEAL (Old)
				{120, 120, 200, 200, 200, 201, 201, 201, 202, 202, 202, 203, 204, 205, 206, 207}, // SKINCOLOR_STEELBLUE (Kart Slate)
  	//  {200, 200, 201, 201, 202, 202, 203, 203, 204, 204, 205, 205, 206, 206, 207, 207}, // SKINCOLOR_STEELBLUE (Old)
				{208, 209, 211, 213, 215, 217, 229, 230, 232, 234, 236, 238, 240, 242, 244, 246}, // SKINCOLOR_BLUE (Kart Sapphire)
    //	{226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241}, // SKINCOLOR_BLUE (Old)
        { 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79}, // SKINCOLOR_PEACH
        { 72,  73,  74,  75,  76,  77,  78,  79,  48,  49,  50,  51,  52,  53,  54,  55}, // SKINCOLOR_TAN
				{120,  96,  64, 121,  67, 144, 123, 192, 193, 194, 195, 196, 197, 198, 199,  30}, // SKINCOLOR_PINK (Kart Bubblegum)
    //  {144, 144, 145, 145, 146, 146, 147, 147, 148, 148, 149, 149, 150, 150, 151, 151}, // SKINCOLOR_PINK (Old)
        {248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253, 253, 254, 254, 255, 255}, // SKINCOLOR_LAVENDER
        {192, 192, 193, 193, 194, 194, 195, 195, 196, 196, 197, 197, 198, 198, 199, 199}, // SKINCOLOR_PURPLE
        { 82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95, 152, 153}, // SKINCOLOR_ORANGE
        { 92,  92,  93,  94,  95,  95, 152, 153, 154, 154, 155, 156, 157, 157, 158, 159}, // SKINCOLOR_ROSEWOOD
        { 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47}, // SKINCOLOR_BEIGE
        { 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63},	// SKINCOLOR_BROWN
        {125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140}, // SKINCOLOR_RED
				{123, 125, 128, 131, 133, 135, 136, 138, 140, 140, 141, 141, 142, 142, 143,  31}, // SKINCOLOR_DARKRED (Kart Crimson)
    //  {133, 133, 134, 134, 135, 135, 136, 136, 137, 137, 138, 138, 139, 139, 140, 140}, // SKINCOLOR_DARKRED (Old)
				{120, 176, 176, 176, 177, 163, 164, 165, 167, 221, 221, 222, 223, 207, 207,  31}, // SKINCOLOR_NEONGREEN (Mint)
		//	{160, 184, 184, 184, 185, 185, 186, 186, 186, 187, 187, 188, 188, 188, 189, 189}, // SKINCOLOR_NEONGREEN (Old)
        {160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175}, // SKINCOLOR_GREEN
        {176, 176, 177, 177, 178, 178, 179, 179, 180, 180, 181, 181, 182, 182, 183, 183}, // SKINCOLOR_ZIM
        {105, 105, 105, 106, 106, 107, 107, 108, 108, 108, 109, 109, 110, 110, 111, 111}, // SKINCOLOR_OLIVE
				{ 96,  97,  98, 100, 101, 102, 104, 113, 114, 115, 116, 117, 118, 119, 156, 159}, // SKINCOLOR_YELLOW (Kart)
    //	{103, 103, 104, 104, 105, 105, 106, 106, 107, 107, 108, 108, 109, 109, 110, 110}, // SKINCOLOR_YELLOW (Old)
        {112, 112, 113, 113, 114, 114, 115, 115, 116, 116, 117, 117, 118, 118, 119, 119}, // SKINCOLOR_GOLD
    };
	INT32 i;
	INT32 starttranscolor;

	// Handle a couple of simple special cases
	if (skinnum == TC_BOSS || skinnum == TC_ALLWHITE || skinnum == TC_METALSONIC || color == SKINCOLOR_NONE)
	{
		for (i = 0; i < NUM_PALETTE_ENTRIES; i++)
		{
			if (skinnum == TC_ALLWHITE) dest_colormap[i] = 0;
			else dest_colormap[i] = (UINT8)i;
		}

		// White!
		if (skinnum == TC_BOSS)
			dest_colormap[31] = 0;
		else if (skinnum == TC_METALSONIC)
			dest_colormap[239] = 0;

		return;
	}

	starttranscolor = (skinnum != TC_DEFAULT) ? skins[skinnum].starttranscolor : DEFAULT_STARTTRANSCOLOR;

	// Fill in the entries of the palette that are fixed
	for (i = 0; i < starttranscolor; i++)
		dest_colormap[i] = (UINT8)i;

	for (i = (UINT8)(starttranscolor + 16); i < NUM_PALETTE_ENTRIES; i++)
		dest_colormap[i] = (UINT8)i;

    if (color < MAXSKINCOLORS)
    {
        for (i = 0; i < SKIN_RAMP_LENGTH; i++)
            dest_colormap[starttranscolor + i] = (UINT8)(skincolorramps[color - 1][i]);
        return;
    }


	// Build the translated ramp
	switch (color)
    {

	// Super colors, from lightest to darkest!
	case SKINCOLOR_SUPER1:
		// Super White
		for (i = 0; i < 10; i++)
			dest_colormap[starttranscolor + i] = 120; // True white
		for (; i < SKIN_RAMP_LENGTH; i++) // White-yellow fade
			dest_colormap[starttranscolor + i] = (UINT8)(96 + (i-10));
		break;

	case SKINCOLOR_SUPER2:
		// Super Bright
		for (i = 0; i < 5; i++) // White-yellow fade
			dest_colormap[starttranscolor + i] = (UINT8)(96 + i);
		dest_colormap[starttranscolor + 5] = 112; // Golden shine
		for (i = 0; i < 8; i++) // Yellow
			dest_colormap[starttranscolor + (i+6)] = (UINT8)(101 + (i>>1));
		for (i = 0; i < 2; i++) // With a fine golden finish! :3
			dest_colormap[starttranscolor + (i+14)] = (UINT8)(113 + i);
		break;

	case SKINCOLOR_SUPER3:
		// Super Yellow
		for (i = 0; i < 3; i++) // White-yellow fade
			dest_colormap[starttranscolor + i] = (UINT8)(98 + i);
		dest_colormap[starttranscolor + 3] = 112; // Golden shine
		for (i = 0; i < 8; i++) // Yellow
			dest_colormap[starttranscolor + (i+4)] = (UINT8)(101 + (i>>1));
		for (i = 0; i < 4; i++) // With a fine golden finish! :3
			dest_colormap[starttranscolor + (i+12)] = (UINT8)(113 + i);
		break;

	case SKINCOLOR_SUPER4:
		// "The SSNTails"
		dest_colormap[starttranscolor] = 112; // Golden shine
		for (i = 0; i < 8; i++) // Yellow
			dest_colormap[starttranscolor + (i+1)] = (UINT8)(101 + (i>>1));
		for (i = 0; i < 7; i++) // With a fine golden finish! :3
			dest_colormap[starttranscolor + (i+9)] = (UINT8)(113 + i);
		break;

	case SKINCOLOR_SUPER5:
		// Golden Delicious
		for (i = 0; i < 8; i++) // Yellow
			dest_colormap[starttranscolor + i] = (UINT8)(101 + (i>>1));
		for (i = 0; i < 7; i++) // With a fine golden finish! :3
			dest_colormap[starttranscolor + (i+8)] = (UINT8)(113 + i);
		dest_colormap[starttranscolor + 15] = 155;
		break;

	// Super Tails
	case SKINCOLOR_TSUPER1:
		for (i = 0; i < 10; i++) // white
			dest_colormap[starttranscolor + i] = 120;
		for (; i < SKIN_RAMP_LENGTH; i++) // orange
			dest_colormap[starttranscolor + i] = (UINT8)(80 + (i-10));
		break;

	case SKINCOLOR_TSUPER2:
		for (i = 0; i < 4; i++) // white
			dest_colormap[starttranscolor + i] = 120;
		for (; i < SKIN_RAMP_LENGTH; i++) // orange
			dest_colormap[starttranscolor + i] = (UINT8)(80 + ((i-4)>>1));
		break;

	case SKINCOLOR_TSUPER3:
		dest_colormap[starttranscolor] = 120; // pure white
		dest_colormap[starttranscolor+1] = 120;
		for (i = 2; i < SKIN_RAMP_LENGTH; i++) // orange
			dest_colormap[starttranscolor + i] = (UINT8)(80 + ((i-2)>>1));
		break;

	case SKINCOLOR_TSUPER4:
		dest_colormap[starttranscolor] = 120; // pure white
		for (i = 1; i < 9; i++) // orange
			dest_colormap[starttranscolor + i] = (UINT8)(80 + (i-1));
		for (; i < SKIN_RAMP_LENGTH; i++) // gold
			dest_colormap[starttranscolor + i] = (UINT8)(115 + (5*(i-9)/7));
		break;

	case SKINCOLOR_TSUPER5:
		for (i = 0; i < 8; i++) // orange
			dest_colormap[starttranscolor + i] = (UINT8)(80 + i);
		for (; i < SKIN_RAMP_LENGTH; i++) // gold
			dest_colormap[starttranscolor + i] = (UINT8)(115 + (5*(i-8)/8));
		break;

	// Super Knuckles
	case SKINCOLOR_KSUPER1:
		for (i = 0; i < SKIN_RAMP_LENGTH; i++)
			dest_colormap[starttranscolor + i] = (UINT8)(120 + (i >> 2));
		break;

	case SKINCOLOR_KSUPER2:
		for (i = 0; i < SKIN_RAMP_LENGTH; i++)
			dest_colormap[starttranscolor + i] = (UINT8)(120 + (6*i/SKIN_RAMP_LENGTH));
		break;

	case SKINCOLOR_KSUPER3:
		for (i = 0; i < SKIN_RAMP_LENGTH; i++)
			dest_colormap[starttranscolor + i] = (UINT8)(120 + (i >> 1));
		break;

	case SKINCOLOR_KSUPER4:
		for (i = 0; i < SKIN_RAMP_LENGTH; i++)
			dest_colormap[starttranscolor + i] = (UINT8)(121 + (i >> 1));
		break;

	case SKINCOLOR_KSUPER5:
		for (i = 0; i < SKIN_RAMP_LENGTH; i++)
			dest_colormap[starttranscolor + i] = (UINT8)(122 + (i >> 1));
		break;

	default:
		I_Error("Invalid skin color #%hu.", (UINT16)color);
		break;
	}
}


/**	\brief	Retrieves a translation colormap from the cache.

	\param	skinnum	number of skin, TC_DEFAULT or TC_BOSS
	\param	color	translation color
	\param	flags	set GTC_CACHE to use the cache

	\return	Colormap. If not cached, caller should Z_Free.
*/
UINT8* R_GetTranslationColormap(INT32 skinnum, skincolors_t color, UINT8 flags)
{
	UINT8* ret;
	INT32 skintableindex;

	// Adjust if we want the default colormap
	if (skinnum == TC_DEFAULT) skintableindex = DEFAULT_TT_CACHE_INDEX;
	else if (skinnum == TC_BOSS) skintableindex = BOSS_TT_CACHE_INDEX;
	else if (skinnum == TC_METALSONIC) skintableindex = METALSONIC_TT_CACHE_INDEX;
	else if (skinnum == TC_ALLWHITE) skintableindex = ALLWHITE_TT_CACHE_INDEX;
	else skintableindex = skinnum;

	if (flags & GTC_CACHE)
	{

		// Allocate table for skin if necessary
		if (!translationtablecache[skintableindex])
			translationtablecache[skintableindex] = Z_Calloc(MAXTRANSLATIONS * sizeof(UINT8**), PU_STATIC, NULL);

		// Get colormap
		ret = translationtablecache[skintableindex][color];
	}
	else ret = NULL;

	// Generate the colormap if necessary
	if (!ret)
	{
		ret = Z_MallocAlign(NUM_PALETTE_ENTRIES, (flags & GTC_CACHE) ? PU_LEVEL : PU_STATIC, NULL, 8);
		R_GenerateTranslationColormap(ret, skinnum, color);

		// Cache the colormap if desired
		if (flags & GTC_CACHE)
			translationtablecache[skintableindex][color] = ret;
	}

	return ret;
}

/**	\brief	Flushes cache of translation colormaps.

	Flushes cache of translation colormaps, but doesn't actually free the
	colormaps themselves. These are freed when PU_LEVEL blocks are purged,
	at or before which point, this function should be called.

	\return	void
*/
void R_FlushTranslationColormapCache(void)
{
	INT32 i;

	for (i = 0; i < (INT32)(sizeof(translationtablecache) / sizeof(translationtablecache[0])); i++)
		if (translationtablecache[i])
			memset(translationtablecache[i], 0, MAXTRANSLATIONS * sizeof(UINT8**));
}

UINT8 R_GetColorByName(const char *name)
{
	UINT8 color = (UINT8)atoi(name);
	if (color > 0 && color < MAXSKINCOLORS)
		return color;
	for (color = 1; color < MAXSKINCOLORS; color++)
		if (!stricmp(Color_Names[color], name))
			return color;
	return 0;
}

// ==========================================================================
//               COMMON DRAWER FOR 8 AND 16 BIT COLOR MODES
// ==========================================================================

// in a perfect world, all routines would be compatible for either mode,
// and optimised enough
//
// in reality, the few routines that can work for either mode, are
// put here

/**	\brief	The R_InitViewBuffer function

	Creates lookup tables for getting the framebuffer address
	of a pixel to draw.

	\param	width	witdh of buffer
	\param	height	hieght of buffer

	\return	void


*/

void R_InitViewBuffer(INT32 width, INT32 height)
{
	INT32 i, bytesperpixel = vid.bpp;

	if (width > MAXVIDWIDTH)
		width = MAXVIDWIDTH;
	if (height > MAXVIDHEIGHT)
		height = MAXVIDHEIGHT;
	if (bytesperpixel < 1 || bytesperpixel > 4)
		I_Error("R_InitViewBuffer: wrong bytesperpixel value %d\n", bytesperpixel);

	// Handle resize, e.g. smaller view windows with border and/or status bar.
	viewwindowx = (vid.width - width) >> 1;

	// Column offset for those columns of the view window, but relative to the entire screen
	for (i = 0; i < width; i++)
		columnofs[i] = (viewwindowx + i) * bytesperpixel;

	// Same with base row offset.
	if (width == vid.width)
		viewwindowy = 0;
	else
		viewwindowy = (vid.height - height) >> 1;

	// Precalculate all row offsets.
	for (i = 0; i < height; i++)
	{
		ylookup[i] = ylookup1[i] = screens[0] + (i+viewwindowy)*vid.width*bytesperpixel;
		ylookup2[i] = screens[0] + (i+(vid.height>>1))*vid.width*bytesperpixel; // for splitscreen
	}
}

/**	\brief viewborder patches lump numbers
*/
lumpnum_t viewborderlump[8];

/**	\brief Store the lumpnumber of the viewborder patches
*/

void R_InitViewBorder(void)
{
	viewborderlump[BRDR_T] = W_GetNumForName("brdr_t");
	viewborderlump[BRDR_B] = W_GetNumForName("brdr_b");
	viewborderlump[BRDR_L] = W_GetNumForName("brdr_l");
	viewborderlump[BRDR_R] = W_GetNumForName("brdr_r");
	viewborderlump[BRDR_TL] = W_GetNumForName("brdr_tl");
	viewborderlump[BRDR_BL] = W_GetNumForName("brdr_bl");
	viewborderlump[BRDR_TR] = W_GetNumForName("brdr_tr");
	viewborderlump[BRDR_BR] = W_GetNumForName("brdr_br");
}

#if 0
/**	\brief R_FillBackScreen

	Fills the back screen with a pattern for variable screen sizes
	Also draws a beveled edge.
*/
void R_FillBackScreen(void)
{
	UINT8 *src, *dest;
	patch_t *patch;
	INT32 x, y, step, boff;

	// quickfix, don't cache lumps in both modes
	if (rendermode != render_soft)
		return;

	// draw pattern around the status bar too (when hires),
	// so return only when in full-screen without status bar.
	if (scaledviewwidth == vid.width && viewheight == vid.height)
		return;

	src = scr_borderpatch;
	dest = screens[1];

	for (y = 0; y < vid.height; y++)
	{
		for (x = 0; x < vid.width/128; x++)
		{
			M_Memcpy (dest, src+((y&127)<<7), 128);
			dest += 128;
		}

		if (vid.width&127)
		{
			M_Memcpy(dest, src+((y&127)<<7), vid.width&127);
			dest += (vid.width&127);
		}
	}

	// don't draw the borders when viewwidth is full vid.width.
	if (scaledviewwidth == vid.width)
		return;

	step = 8;
	boff = 8;

	patch = W_CacheLumpNum(viewborderlump[BRDR_T], PU_CACHE);
	for (x = 0; x < scaledviewwidth; x += step)
		V_DrawPatch(viewwindowx + x, viewwindowy - boff, 1, patch);

	patch = W_CacheLumpNum(viewborderlump[BRDR_B], PU_CACHE);
	for (x = 0; x < scaledviewwidth; x += step)
		V_DrawPatch(viewwindowx + x, viewwindowy + viewheight, 1, patch);

	patch = W_CacheLumpNum(viewborderlump[BRDR_L], PU_CACHE);
	for (y = 0; y < viewheight; y += step)
		V_DrawPatch(viewwindowx - boff, viewwindowy + y, 1, patch);

	patch = W_CacheLumpNum(viewborderlump[BRDR_R],PU_CACHE);
	for (y = 0; y < viewheight; y += step)
		V_DrawPatch(viewwindowx + scaledviewwidth, viewwindowy + y, 1,
			patch);

	// Draw beveled corners.
	V_DrawPatch(viewwindowx - boff, viewwindowy - boff, 1,
		W_CacheLumpNum(viewborderlump[BRDR_TL], PU_CACHE));
	V_DrawPatch(viewwindowx + scaledviewwidth, viewwindowy - boff, 1,
		W_CacheLumpNum(viewborderlump[BRDR_TR], PU_CACHE));
	V_DrawPatch(viewwindowx - boff, viewwindowy + viewheight, 1,
		W_CacheLumpNum(viewborderlump[BRDR_BL], PU_CACHE));
	V_DrawPatch(viewwindowx + scaledviewwidth, viewwindowy + viewheight, 1,
		W_CacheLumpNum(viewborderlump[BRDR_BR], PU_CACHE));
}
#endif

/**	\brief	The R_VideoErase function

	Copy a screen buffer.

	\param	ofs	offest from buffer
	\param	count	bytes to erase

	\return	void


*/
void R_VideoErase(size_t ofs, INT32 count)
{
	// LFB copy.
	// This might not be a good idea if memcpy
	//  is not optimal, e.g. byte by byte on
	//  a 32bit CPU, as GNU GCC/Linux libc did
	//  at one point.
	M_Memcpy(screens[0] + ofs, screens[1] + ofs, count);
}

#if 0
/**	\brief The R_DrawViewBorder

  Draws the border around the view
	for different size windows?
*/
void R_DrawViewBorder(void)
{
	INT32 top, side, ofs;

	if (rendermode == render_none)
		return;
#ifdef HWRENDER
	if (rendermode != render_soft)
	{
		HWR_DrawViewBorder(0);
		return;
	}
	else
#endif

#ifdef DEBUG
	fprintf(stderr,"RDVB: vidwidth %d vidheight %d scaledviewwidth %d viewheight %d\n",
		vid.width, vid.height, scaledviewwidth, viewheight);
#endif

	if (scaledviewwidth == vid.width)
		return;

	top = (vid.height - viewheight)>>1;
	side = (vid.width - scaledviewwidth)>>1;

	// copy top and one line of left side
	R_VideoErase(0, top*vid.width+side);

	// copy one line of right side and bottom
	ofs = (viewheight+top)*vid.width - side;
	R_VideoErase(ofs, top*vid.width + side);

	// copy sides using wraparound
	ofs = top*vid.width + vid.width-side;
	side <<= 1;

    // simpler using our VID_Blit routine
	VID_BlitLinearScreen(screens[1] + ofs, screens[0] + ofs, side, viewheight - 1,
		vid.width, vid.width);
}
#endif

// ==========================================================================
//                   INCLUDE 8bpp DRAWING CODE HERE
// ==========================================================================

#include "r_draw8.c"

// ==========================================================================
//                   INCLUDE 16bpp DRAWING CODE HERE
// ==========================================================================

#ifdef HIGHCOLOR
#include "r_draw16.c"
#endif
