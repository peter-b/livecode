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
		MC_TYPEINFO_INITIALIZE_EXEC_ENUM(InkNames) &&
		MCTextStyleEnumTypeInfoInitialize ();
}

void
MCEngineTypeInfoFinalize (void)
{
	MC_TYPEINFO_FINALIZE_EXEC_ENUM(InkNames);
	MCValueRelease (kMCTextStyleEnumTypeInfo);
	kMCTextStyleEnumTypeInfo = nil;
}
