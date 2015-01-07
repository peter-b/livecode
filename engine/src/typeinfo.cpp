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

/* ================================================================
 * [Public] Initialization and finalization
 * ================================================================ */

#define MC_TYPEINFO_INITIALIZE_EXEC_ENUM(tag) \
	(MCTypeInfoFromExecTypeInfo (kMCInterface##tag##TypeInfo, \
	                             MCNAME("com.livecode.interface." #tag), \
	                             kMC##tag##EnumTypeInfo))
#define MC_TYPEINFO_FINALIZE_EXEC_ENUM(tag) \
	do { MCValueRelease (kMC##tag##EnumTypeInfo); \
		kMC##tag##EnumTypeInfo = nil; } while (0)


bool
MCEngineTypeInfoInitialize (void)
{
	return
		MC_TYPEINFO_INITIALIZE_EXEC_ENUM(LayerMode) &&
		MC_TYPEINFO_INITIALIZE_EXEC_ENUM(PlayDestination) &&
		MC_TYPEINFO_INITIALIZE_EXEC_ENUM(InkNames) &&
		MCObjectIdCustomTypeInfoInitialize () &&
		MCRectangleRecordTypeInfoInitialize () &&
		MCAudioClipFormatTypeInfoInitialize () &&
		MCTextStyleEnumTypeInfoInitialize () &&
		MCBitmapEffectRecordTypeInfoInitialize () &&
		MCBitmapEffectEnumTypeInfoInitialize ();
}

void
MCEngineTypeInfoFinalize (void)
{
	MC_TYPEINFO_FINALIZE_EXEC_ENUM(LayerMode);
	MC_TYPEINFO_FINALIZE_EXEC_ENUM(PlayDestination);
	MC_TYPEINFO_FINALIZE_EXEC_ENUM(InkNames);
	MCValueRelease (kMCObjectIdCustomTypeInfo);
	kMCObjectIdCustomTypeInfo = nil;
	MCValueRelease (kMCOptionalObjectIdCustomTypeInfo);
	kMCOptionalObjectIdCustomTypeInfo = nil;
	MCValueRelease (kMCRectangleRecordTypeInfo);
	kMCRectangleRecordTypeInfo = nil;
	MCValueRelease (kMCAudioClipFormatEnumTypeInfo);
	kMCAudioClipFormatEnumTypeInfo = nil;
	MCValueRelease (kMCTextStyleEnumTypeInfo);
	kMCTextStyleEnumTypeInfo = nil;

	MCValueRelease (kMCBitmapEffectRecordTypeInfo);
	kMCBitmapEffectRecordTypeInfo = nil;
	MCValueRelease (kMCOptionalBitmapEffectRecordTypeInfo);
	kMCOptionalBitmapEffectRecordTypeInfo = nil;
	MCValueRelease (kMCBitmapEffectBlendModeEnumTypeInfo);
	kMCBitmapEffectBlendModeEnumTypeInfo = nil;
	MCValueRelease (kMCBitmapEffectFilterEnumTypeInfo);
	kMCBitmapEffectFilterEnumTypeInfo = nil;
}
