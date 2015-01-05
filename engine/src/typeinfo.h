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

#ifndef _MC_TYPEINFO_H_
#define _MC_TYPEINFO_H_

#include "prefix.h"

#include "parsedef.h"
#include "sysdefs.h"
#include "aclip.h"
#include "bitmapeffect.h"
#include "bitmapeffectblur.h"

#include "exec.h"

/* This header declares functions for defining and manipulating
 * engine-specific MCValueRef variants. */

/* ----------------------------------------------------------------
 * Global typeinfo constants
 * ---------------------------------------------------------------- */

extern MCTypeInfoRef kMCRectangleRecordTypeInfo;
extern MCTypeInfoRef kMCTextStyleEnumTypeInfo;
extern MCTypeInfoRef kMCAudioClipFormatEnumTypeInfo;
extern MCTypeInfoRef kMCDecorationEnumTypeInfo;
extern MCTypeInfoRef kMCObjectIdCustomTypeInfo;
extern MCTypeInfoRef kMCOptionalObjectIdCustomTypeInfo;

extern MCTypeInfoRef kMCBitmapEffectRecordTypeInfo;
extern MCTypeInfoRef kMCOptionalBitmapEffectRecordTypeInfo;
extern MCTypeInfoRef kMCBitmapEffectBlendModeEnumTypeInfo;
extern MCTypeInfoRef kMCBitmapEffectFilterEnumTypeInfo;

/* ----------------------------------------------------------------
 * Compatibility with exec interface types
 * ---------------------------------------------------------------- */

/* Convert an exec interface enumerated typeinfo structure into an
 * MCTypeInfoRef for an MCEnumRef enumerated type.  The r_typeinfo
 * will be a named typeinfo with the specified p_name. */
bool MCTypeInfoFromExecTypeInfo (MCExecEnumTypeInfo *p_info, MCNameRef p_name, MCTypeInfoRef & r_typeinfo);

/* Turn an MCEnumRef into a C enumeration value using the specified
 * type information. */
bool MCTypeInfoGetValueWithExecTypeInfo (MCTypeInfoRef p_typeinfo, MCExecEnumTypeInfo *p_info, MCEnumRef p_enum, intenum_t & r_bits);
/* Turn a C enumeration value into an MCEnumRef using the specified
 * type information. */
bool MCTypeInfoMakeValueWithExecTypeInfo (MCTypeInfoRef p_typeinfo, MCExecEnumTypeInfo *p_info, intenum_t p_bits, MCEnumRef & r_enum);

/* Define the interface for an MCEnumRef type based on an exec
 * interface enumerated typeinfo. */
#define MC_TYPEINFO_DEFINE_EXEC_ENUM(tag)	  \
	extern MCTypeInfoRef kMC##tag##EnumTypeInfo; \
	static inline bool \
	MC##tag##EnumFromBits (intenum_t p_bits, MCEnumRef & r_enum) \
	{ return MCTypeInfoMakeValueWithExecTypeInfo (kMC##tag##EnumTypeInfo, \
	                                              kMCInterface##tag##TypeInfo, \
	                                              p_bits, r_enum); } \
	static inline bool \
	MC##tag##EnumToBits (MCEnumRef p_enum, intenum_t & r_bits) \
	{ return MCTypeInfoGetValueWithExecTypeInfo (kMC##tag##EnumTypeInfo, \
	                                             kMCInterface##tag##TypeInfo, \
	                                             p_enum, r_bits); }

/* ----------------------------------------------------------------
 * Ink modes
 * ---------------------------------------------------------------- */

MC_TYPEINFO_DEFINE_EXEC_ENUM(InkNames)

/* ----------------------------------------------------------------
 * Audioclip play destination
 * ---------------------------------------------------------------- */

MC_TYPEINFO_DEFINE_EXEC_ENUM(PlayDestination)

/* ----------------------------------------------------------------
 * Control layer mode
 * ---------------------------------------------------------------- */

MC_TYPEINFO_DEFINE_EXEC_ENUM(LayerMode);

/* ----------------------------------------------------------------
 * Audio clip format
 * ---------------------------------------------------------------- */

/* Convert an audio format to an enumerated value */
bool MCAudioClipFormatEnumFromBits (enum Audio_format p_bits, MCEnumRef & r_enum);
/* Convert an enumerated value to an audio format */
bool MCAudioClipFormatEnumToBits (MCEnumRef p_enum, enum Audio_format & r_bits);

/* ----------------------------------------------------------------
 * Stack fullscreen modes
 * ---------------------------------------------------------------- */

MC_TYPEINFO_DEFINE_EXEC_ENUM(StackFullscreenMode)

/* ----------------------------------------------------------------
 * Stack styles
 * ---------------------------------------------------------------- */

MC_TYPEINFO_DEFINE_EXEC_ENUM(StackStyle)

/* ----------------------------------------------------------------
 * Rectangles
 * ---------------------------------------------------------------- */

/* Convert the rectangle p_rect to an MCRecordRef instance of
 * kMCRectangleRecordTypeInfo. */
bool MCRectangleRecordFromStruct (MCRectangle & p_rect, MCRecordRef & r_record);
/* Convert an MCRecordRef instance of kMCRectangleRecordTypeInfo to a
 * rectangle r_rect. */
bool MCRectangleRecordToStruct (MCRecordRef p_record, MCRectangle & r_rect);

/* ----------------------------------------------------------------
 * Text styles
 * ---------------------------------------------------------------- */

/* Convert the text style bitset p_style_set to a MCProperSetRef r_set
 * containing MCEnumRef instances of kMCTextStyleEnumTypeInfo */
bool MCTextStyleEnumSetFromFlags (uint2 p_style_set, MCProperSetRef & r_set);
/* Convert an MCProperSetRef p_set containing MCEnumRef instances of
 * kMCTextStyleEnumTypeInfo to a text style bitset r_style_set */
bool MCTextStyleEnumSetToFlags (MCProperSetRef p_set, uint2 & r_style_set);

/* ----------------------------------------------------------------
 * Stack decorations
 * ---------------------------------------------------------------- */

/* Convert the stack decoration bitset p_decoration_set to a
 * MCProperSetRef r_set containing MCEnumRef instances of
 * kMCDecorationEnumTypeInfo */
bool MCDecorationEnumSetFromFlags (uint16_t p_decoration_set, MCProperSetRef & r_set);
/* Convert an MCProperSetRef p_set containing MCEnumRef instances of
 * kMCDecorationEnumTypeInfo to a stack decoration bitset
 * r_decoration_set */
bool MCDecorationEnumSetToFlags (MCProperSetRef p_set, uint16_t & r_decoration_set);

/* ----------------------------------------------------------------
 * Object IDs
 * ---------------------------------------------------------------- */

/* Create a custom MCValueRef that contains object identification
 * information.  N.b. the p_id argument is copied; it isn't stored in
 * the r_value. */
bool MCObjectIdValueFromInstance (const MCObjectId *p_id, MCValueRef & r_value);
/* Get object identification information from a custom MCValueRef.
 * N.b. the returned value belongs to p_value and will be destroyed
 * with p_value, so make sure to hold a reference to p_value until
 * you've finished with the object ID. */
const MCObjectId *MCObjectIdValueToInstance (MCValueRef p_value);

/* ----------------------------------------------------------------
 * Initialization and finalization
 * ---------------------------------------------------------------- */

bool MCEngineTypeInfoInitialize (void);
void MCEngineTypeInfoFinalize (void);

#endif /* !_MC_TYPEINFO_H_ */
