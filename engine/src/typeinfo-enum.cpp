/*                                                                   -*- c++ -*-
Copyright (C) 2003-2015 Runtime Revolution Ltd.

This file is part of LiveCode.

LiveCode is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License v3 as published by the Free
Software Foundation.

LiveCode is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with LiveCode.  If not see <http://www.gnu.org/licenses/>.  */

#include "typeinfo-private.h"

#include "font.h"

/* ================================================================
 * [Private] File-local declarations
 * ================================================================ */

/* ---------- Native enum support */

typedef struct _MCNativeEnumValueInfo MCNativeEnumValueInfo;

struct _MCNativeEnumValueInfo {
	intenum_t m_bits;
	const char *m_cname;
};

/* Create a new native typeinfo mapping names to C enum constants */
bool MCNativeEnumTypeInfoCreateWithName (const MCNativeEnumValueInfo *p_values, index_t p_value_count, MCNameRef p_name, MCTypeInfoRef & r_typeinfo);
/* Convert a C enum constant to a MCEnumRef value of the corresponding
 * p_typeinfo */
bool MCTypeInfoNativeEnumToBits (MCTypeInfoRef p_typeinfo, const MCNativeEnumValueInfo *p_values, index_t p_value_count, MCEnumRef p_enum, intenum_t & r_bits);
/* Convert an MCEnumRef value to a corresponding C enum constant */
bool MCTypeInfoNativeEnumFromBits (MCTypeInfoRef p_typeinfo, const MCNativeEnumValueInfo *p_values, index_t p_value_count, intenum_t p_bits, MCEnumRef & r_enum);

/* ---------- Sets of enumerated values */

typedef bool (*MCTypeInfoEnumFromBitsFunc)(intenum_t, MCEnumRef &);
typedef bool (*MCTypeInfoEnumToBitsFunc)(MCEnumRef, intenum_t &);

/* Use the conversion function p_func to convert the flag word p_flags
 * into an MCProperSet of MCEnumRef values. */
bool MCTypeInfoEnumSetFromFlags (intenum_t p_flags, MCTypeInfoEnumFromBitsFunc p_func, MCProperSetRef & r_set);
/* Use the conversion function p_func to convert p_set, a MCProperSet
 * of MCEnumRef values, into a flag word r_flags. */
bool MCTypeInfoEnumSetToFlags (MCProperSetRef p_set, MCTypeInfoEnumToBitsFunc p_func, intenum_t & r_flags);

/* ---------- Text styles */

/* Convert a text style to a corresponding interned name */
static bool MCTextStyleNameFromFlags (Font_textstyle p_style, MCNameRef & r_name);
/* Convert a text style to a member of the text style enumerated
 * type. */
static bool MCTextStyleEnumFromFlags (Font_textstyle p_style, MCEnumRef & r_enum);
/* Convert a member of the text style enumerated type to a text
 * style */
static bool MCTextStyleEnumToFlags (MCEnumRef p_enum, Font_textstyle & r_style);
/* If the text style p_style is enabled in the text style bitset
 * p_styles, then add the corresponding member of the text style
 * enumerated type to set. */
static bool MCTextStyleEnumAddToSet (MCProperSetRef set, uint2 p_styles, Font_textstyle p_style);

/* ================================================================
 * Text style enumeration
 * ================================================================ */

/* ----------------------------------------------------------------
 * [Private] Utility functions
 * ---------------------------------------------------------------- */

static bool
MCTextStyleNameFromFlags (Font_textstyle p_style,
                          MCNameRef & r_name)
{
	const char *t_cstring;
	t_cstring = MCF_unparsetextstyle (p_style);

	MCNewAutoNameRef t_name;
	if (!MCNameCreateWithCString (t_cstring, &t_name))
		return false;

	MCAssert (!MCValueIsEqualTo (*t_name, kMCEmptyName));
	r_name = MCValueRetain (*t_name);
	return true;
}

static bool
MCTextStyleEnumFromFlags (Font_textstyle p_style,
                          MCEnumRef & r_enum)
{
	MCNewAutoNameRef t_name;
	if (!MCTextStyleNameFromFlags (p_style, &t_name))
		return false;
	return MCEnumCreate (kMCTextStyleEnumTypeInfo, *t_name, r_enum);
}

static bool
MCTextStyleEnumToFlags (MCEnumRef p_enum,
                        Font_textstyle & r_style)
{
	MCAssert (MCTypeInfoConforms (MCValueGetTypeInfo (p_enum),
	                              kMCTextStyleEnumTypeInfo));
	MCValueRef t_name = MCEnumGetValue (p_enum);
	MCStringRef t_string = MCNameGetString ((MCNameRef) t_name);

	return MCF_parsetextstyle (t_string, r_style) == ES_NORMAL;
}

static bool
MCTextStyleEnumAddToSet (MCProperSetRef set,
                         uint2 p_styles,
                         Font_textstyle p_style)
{
	if (!MCF_istextstyleset (p_styles, p_style)) return true;

	MCAutoEnumRef t_enum;
	return
		MCTextStyleEnumFromFlags (p_style, &t_enum) &&
		MCProperSetAddElement (set, *t_enum);
}

/* ----------------------------------------------------------------
 * [Private] Initialization
 * ---------------------------------------------------------------- */

bool
MCTextStyleEnumTypeInfoInitialize (void)
{
	const char *k_type_name = "com.livecode.interface.object.textstyle";

	MCNameRef t_enum_values[10];
	t_enum_values[9] = nil; /* Custodian */
	if (!(MCTextStyleNameFromFlags (FTS_BOLD, t_enum_values[0]) &&
	      MCTextStyleNameFromFlags (FTS_ITALIC, t_enum_values[1]) &&
	      MCTextStyleNameFromFlags (FTS_OBLIQUE, t_enum_values[2]) &&
	      MCTextStyleNameFromFlags (FTS_CONDENSED, t_enum_values[3]) &&
	      MCTextStyleNameFromFlags (FTS_EXPANDED, t_enum_values[4]) &&
	      MCTextStyleNameFromFlags (FTS_BOX, t_enum_values[5]) &&
	      MCTextStyleNameFromFlags (FTS_3D_BOX, t_enum_values[6]) &&
	      MCTextStyleNameFromFlags (FTS_UNDERLINE, t_enum_values[7]) &&
	      MCTextStyleNameFromFlags (FTS_STRIKEOUT, t_enum_values[8])))
		return false;

	MCAutoTypeInfoRef t_raw_typeinfo;
	if (!MCEnumTypeInfoCreate ((MCValueRef *) t_enum_values, -1, &t_raw_typeinfo))
		return false;

	MCAutoTypeInfoRef t_typeinfo;
	if (!(MCNamedTypeInfoCreate (MCNAME (k_type_name), &t_typeinfo) &&
	      MCNamedTypeInfoBind (*t_typeinfo, *t_raw_typeinfo)))
		return false;

	kMCTextStyleEnumTypeInfo = MCValueRetain (*t_typeinfo);
	return true;
}

/* ----------------------------------------------------------------
 * [Public] Conversion to/from style flag fields
 * ---------------------------------------------------------------- */

bool
MCTextStyleEnumSetFromFlags (uint2 p_style_set,
                             MCProperSetRef & r_set)
{
	MCAutoProperSetRef t_set;
	if (!MCProperSetCreateMutable (&t_set))
		return false;

	return
		MCTextStyleEnumAddToSet(*t_set,p_style_set,FTS_BOLD) &&
		MCTextStyleEnumAddToSet(*t_set,p_style_set,FTS_ITALIC) &&
		MCTextStyleEnumAddToSet(*t_set,p_style_set,FTS_OBLIQUE) &&
		MCTextStyleEnumAddToSet(*t_set,p_style_set,FTS_EXPANDED) &&
		MCTextStyleEnumAddToSet(*t_set,p_style_set,FTS_CONDENSED) &&
		MCTextStyleEnumAddToSet(*t_set,p_style_set,FTS_BOX) &&
		MCTextStyleEnumAddToSet(*t_set,p_style_set,FTS_3D_BOX) &&
		MCTextStyleEnumAddToSet(*t_set,p_style_set,FTS_UNDERLINE) &&
		MCTextStyleEnumAddToSet(*t_set,p_style_set,FTS_STRIKEOUT) &&
		MCProperSetCopy (*t_set, r_set);
}

bool
MCTextStyleEnumSetToFlags (MCProperSetRef p_set,
                           uint2 & r_style_set)
{
	uint2 t_style_set = FA_DEFAULT_STYLE;
	uintptr_t t_iterator = 0;
	MCValueRef t_element;
	while (MCProperSetIterate (p_set, t_iterator, t_element))
	{
		Font_textstyle t_style;
		bool t_success;
		t_success = MCTextStyleEnumToFlags ((MCEnumRef) t_element, t_style);
		MCAssert (t_success);

		MCF_changetextstyle (t_style_set, t_style, true);
	}

	r_style_set = t_style_set;
	return true;
}

/* ================================================================
 * Audio clip format enumeration
 * ================================================================ */

static const MCNativeEnumValueInfo kMCAudioClipFormatEnumValues[] = {
	{ AF_SLINEAR, "slinear" },
	{ AF_MULAW, "mulaw" },
	{ AF_ULINEAR, "ulinear" },
	{ 0, NULL },
};

/* ----------------------------------------------------------------
 * [Private] Initialization
 * ---------------------------------------------------------------- */

bool
MCAudioClipFormatTypeInfoInitialize (void)
{
	const char *k_type_name = "com.livecode.interface.audioclip.format";

	return MCNativeEnumTypeInfoCreateWithName (kMCAudioClipFormatEnumValues,
	                                           -1,
	                                           MCNAME (k_type_name),
	                                           kMCAudioClipFormatEnumTypeInfo);
}

/* ----------------------------------------------------------------
 * [Public] Conversion to/from C enum
 * ---------------------------------------------------------------- */

bool
MCAudioClipFormatEnumFromBits (enum Audio_format p_bits,
                               MCEnumRef & r_enum)
{
	return MCTypeInfoNativeEnumFromBits (kMCAudioClipFormatEnumTypeInfo,
	                                     kMCAudioClipFormatEnumValues,
	                                     -1,
	                                     p_bits, r_enum);
}

bool
MCAudioClipFormatEnumToBits (MCEnumRef p_enum,
                             enum Audio_format & r_bits)
{
	intenum_t t_bits;
	if (!MCTypeInfoNativeEnumToBits (kMCAudioClipFormatEnumTypeInfo,
	                                 kMCAudioClipFormatEnumValues,
	                                 -1,
	                                 p_enum, t_bits))
		return false;

	r_bits = (enum Audio_format) t_bits;
	return true;
}

/* ================================================================
 * Bitmap effect enumerated types
 * ================================================================ */

static const MCNativeEnumValueInfo kMCBitmapEffectBlendModeEnumValues[] = {
	{ kMCBitmapEffectBlendModeNormal, "normal" },
	{ kMCBitmapEffectBlendModeMultiply, "multiply" },
	{ kMCBitmapEffectBlendModeScreen, "screen" },
	{ kMCBitmapEffectBlendModeOverlay, "overlay" },
	{ kMCBitmapEffectBlendModeDarken, "darken" },
	{ kMCBitmapEffectBlendModeLighten, "lighten" },
	{ kMCBitmapEffectBlendModeColorDodge, "dodge" },
	{ kMCBitmapEffectBlendModeColorBurn, "burn" },
	{ kMCBitmapEffectBlendModeHardLight, "hardLight" },
	{ kMCBitmapEffectBlendModeSoftLight, "softLight" },
	{ kMCBitmapEffectBlendModeDifference, "difference" },
	{ kMCBitmapEffectBlendModeExclusion, "exclusion" },
	{ kMCBitmapEffectBlendModeHue, "hue" },
	{ kMCBitmapEffectBlendModeSaturation, "saturation" },
	{ kMCBitmapEffectBlendModeColor, "color" },
	{ kMCBitmapEffectBlendModeLuminosity, "luminosity" },
	{ 0, NULL },
};

static const MCNativeEnumValueInfo kMCBitmapEffectFilterEnumValues[] = {
	{ kMCBitmapEffectFilterFastGaussian, "gaussian" },
	{ kMCBitmapEffectFilterOnePassBox, "box1pass" },
	{ kMCBitmapEffectFilterTwoPassBox, "box2pass" },
	{ kMCBitmapEffectFilterThreePassBox, "box3pass" },
	{ 0, NULL },
};

/* ----------------------------------------------------------------
 * [Private] Initialization
 * ---------------------------------------------------------------- */

bool
MCBitmapEffectEnumTypeInfoInitialize (void)
{
	{
		const char *k_type_name = "com.livecode.interface.bitmapeffect.blendmode";

		if (!MCNativeEnumTypeInfoCreateWithName (kMCBitmapEffectBlendModeEnumValues,
		                                         -1,
		                                         MCNAME (k_type_name),
		                                         kMCBitmapEffectBlendModeEnumTypeInfo))
			return false;
	}
	{
		const char *k_type_name = "com.livecode.interface.bitmapeffect.filter";

		if (!MCNativeEnumTypeInfoCreateWithName (kMCBitmapEffectFilterEnumValues,
		                                         -1,
		                                         MCNAME (k_type_name),
		                                         kMCBitmapEffectFilterEnumTypeInfo))
			return false;
	}
	return true;
}

/* ================================================================
 * [Public] Typeinfo constants
 * ================================================================ */

MCTypeInfoRef kMCLayerModeEnumTypeInfo;
MCTypeInfoRef kMCPlayDestinationEnumTypeInfo;
MCTypeInfoRef kMCStackFullscreenModeEnumTypeInfo;
MCTypeInfoRef kMCInkNamesEnumTypeInfo;
MCTypeInfoRef kMCAudioClipFormatEnumTypeInfo;
MCTypeInfoRef kMCTextStyleEnumTypeInfo;

MCTypeInfoRef kMCBitmapEffectBlendModeEnumTypeInfo;
MCTypeInfoRef kMCBitmapEffectFilterEnumTypeInfo;

/* ================================================================
 * [Public] Compatibility with exec interface types.
 * ================================================================ */

bool
MCTypeInfoFromExecTypeInfo (MCExecEnumTypeInfo *p_info,
                            MCNameRef p_name,
                            MCTypeInfoRef & r_typeinfo)
{
	MCNameRef *t_enum_values;
	MCAutoTypeInfoRef t_raw_typeinfo, t_typeinfo;
	bool t_success = true;
	uindex_t t_size = p_info->count;

	if (!MCMemoryNewArray (t_size, t_enum_values))
		return false;

	/* Build array of element names */
	for (uindex_t i = 0; t_success && i < t_size; ++i)
	{
		t_success = MCNameCreateWithCString (p_info->elements[i].tag,
		                                     t_enum_values[i]);
	}

	/* Create raw enum typeinfo */
	if (t_success)
		t_success = MCEnumTypeInfoCreate ((MCValueRef *) t_enum_values,
		                                  t_size,
		                                  &t_raw_typeinfo);

	/* Bind named typeinfo */
	if (t_success)
		t_success = (MCNamedTypeInfoCreate (p_name, &t_typeinfo) &&
		             MCNamedTypeInfoBind (&t_typeinfo, *t_raw_typeinfo));

	if (t_success)
		r_typeinfo = MCValueRetain (*t_typeinfo);

	/* Unconditionally clean up array of element names */
	for (uindex_t i = 0; i < t_size; ++i)
	{
		MCValueRelease (t_enum_values[i]);
	}
	MCMemoryDeleteArray (t_enum_values);

	return t_success;
}

bool
MCTypeInfoGetValueWithExecTypeInfo (MCTypeInfoRef p_typeinfo,
                                    MCExecEnumTypeInfo *p_info,
                                    MCEnumRef p_enum,
                                    intenum_t & r_bits)
{
	MCAssert (MCTypeInfoConforms (MCValueGetTypeInfo (p_enum),
	                              p_typeinfo));

	MCNameRef t_name = (MCNameRef) (MCEnumGetValue (p_enum));

	/* Try to find a member of the exec enum that matches */
	for (uindex_t i = 0; i < p_info->count; ++i)
	{
		if (!MCNameIsEqualTo (t_name, MCNAME(p_info->elements[i].tag)))
			continue;

		r_bits = p_info->elements[i].value;
		return true;
	}

	MCUnreachable (); /* No match found iff programming error */
}

bool
MCTypeInfoMakeValueWithExecTypeInfo (MCTypeInfoRef p_typeinfo,
                                     MCExecEnumTypeInfo *p_info,
                                     intenum_t p_bits,
                                     MCEnumRef & r_enum)
{
	/* Try to find a member of the exec enum that matches */
	for (uindex_t i = 0; i < p_info->count; ++i)
	{
		if (p_bits == p_info->elements[i].value)
		{
			MCNewAutoNameRef t_name;
			return
				MCNameCreateWithCString (p_info->elements[i].tag, &t_name) &&
				MCEnumCreate (p_typeinfo, *t_name, r_enum);
		}
	}

	MCUnreachable (); /* No match found iff programming error */
}

/* ================================================================
 * [Private] Compatibility with native enum types.
 * ================================================================ */

bool
MCTypeInfoNativeEnumFromBits (MCTypeInfoRef p_typeinfo,
                              const MCNativeEnumValueInfo *p_values,
                              index_t p_value_count,
                              intenum_t p_bits,
                              MCEnumRef & r_enum)
{
	/* Search for a matching member of the native enum */
	for (index_t i = 0;
	     (p_value_count < 0) ? (NULL != p_values[i].m_cname) : (i < p_value_count);
	     ++i)
	{
		if (p_bits != p_values[i].m_bits) continue;

		MCNewAutoNameRef t_name;
		return
			MCNameCreateWithCString (p_values[i].m_cname, &t_name) &&
			MCEnumCreate (p_typeinfo, *t_name, r_enum);
	}

	MCUnreachable(); /* No match found iff programming error */
}

bool
MCTypeInfoNativeEnumToBits (MCTypeInfoRef p_typeinfo,
                            const MCNativeEnumValueInfo *p_values,
                            index_t p_value_count,
                            MCEnumRef p_enum,
                            intenum_t & r_bits)
{
	MCAssert (MCTypeInfoConforms (MCValueGetTypeInfo (p_enum), p_typeinfo));

	MCNameRef t_name = (MCNameRef) (MCEnumGetValue (p_enum));

	/* Search for a matching member of the native enum */
	for (index_t i = 0;
	     (p_value_count < 0) ? (NULL != p_values[i].m_cname) : (i < p_value_count);
	     ++i)
	{
		if (!MCNameIsEqualTo (t_name, MCNAME(p_values[i].m_cname)))
			continue;

		r_bits = p_values[i].m_bits;
		return true;
	}

	MCUnreachable (); /* No match found iff programming error */
}

bool
MCNativeEnumTypeInfoCreateWithName (const MCNativeEnumValueInfo *p_values,
                                    index_t p_value_count,
                                    MCNameRef p_name,
                                    MCTypeInfoRef & r_typeinfo)
{
	bool t_success = true;

	MCAssert (p_values);
	MCAssert (p_name);

	/* If no count of values was provided, count by looking for custodian. */
	if (p_value_count < 0)
		for (p_value_count = 0;
		     p_values[p_value_count].m_cname;
		     ++p_value_count);

	/* Create array of element names */
	MCNameRef *t_enum_values;
	if (!MCMemoryNewArray (p_value_count, t_enum_values))
		return false;

	for (index_t i = 0; t_success && i < p_value_count; ++i)
		t_success = MCNameCreateWithCString (p_values[i].m_cname,
		                                     t_enum_values[i]);

	/* Create raw enum typeinfo */
	if (t_success)
		t_success = MCEnumTypeInfoCreateWithName ((MCValueRef *) t_enum_values,
		                                          p_value_count,
		                                          p_name,
		                                          r_typeinfo);

	/* Unconditionally clean up array of element names */
	for (index_t i = 0; i < p_value_count; ++i)
		MCValueRelease (t_enum_values[i]);
	MCMemoryDeleteArray (t_enum_values);

	return t_success;
}

/* ================================================================
 * [Private] Sets of enumerated values
 * ================================================================ */

bool
MCTypeInfoEnumSetFromFlags (intenum_t p_flags,
                            MCTypeInfoEnumFromBitsFunc p_func,
                            MCProperSetRef & r_set)
{
	MCAutoProperSetRef t_set;
	if (!MCProperSetCreateMutable (&t_set))
		return false;

	for (uindex_t i = 0; i < 8*sizeof(p_flags); ++i)
	{
		/* Check if each bit is set */
		intenum_t t_bit = 1 << i;
		if (0 == (p_flags & t_bit)) continue;

		/* If set, include the corresponding enum member in the result */
		MCAutoEnumRef t_enum;
		if (!(p_func (t_bit, &t_enum) &&
		      MCProperSetAddElement (*t_set, *t_enum)))
			return false;
	}

	return MCProperSetCopy (*t_set, r_set);
}

bool
MCTypeInfoEnumSetToFlags (MCProperSetRef p_set,
                          MCTypeInfoEnumToBitsFunc p_func,
                          intenum_t & r_flags)
{
	intenum_t t_flags = 0;
	uintptr_t t_iterator = 0;
	MCValueRef t_element;
	while (MCProperSetIterate (p_set, t_iterator, t_element))
	{
		intenum_t t_bit = 0;
		if (!p_func ((MCEnumRef) t_element, t_bit))
			return false;

		t_flags |= t_bit;
	}

	r_flags = t_flags;
	return true;
}
