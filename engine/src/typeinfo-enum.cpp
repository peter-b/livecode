/*                                                                   -*- c++ -*-
Copyright (C) 2003-2014 Runtime Revolution Ltd.

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
 * [Public] Typeinfo constants
 * ================================================================ */

MCTypeInfoRef kMCInkNamesEnumTypeInfo;
MCTypeInfoRef kMCTextStyleEnumTypeInfo;

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
	const char *t_tag = MCStringGetCString (MCNameGetString (t_name));

	/* Try to find a member of the exec enum that matches */
	for (uindex_t i = 0; i < p_info->count; ++i)
	{
		if (0 == strcmp (t_tag, p_info->elements[i].tag))
		{
			r_bits = p_info->elements[i].value;
			return true;
		}
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
