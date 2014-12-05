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

#ifndef _MC_TYPEINFO_H_
#define _MC_TYPEINFO_H_

#include "prefix.h"

#include "parsedef.h"
#include "sysdefs.h"

#include "exec.h"

/* This header declares functions for defining and manipulating
 * engine-specific MCValueRef variants. */

/* ----------------------------------------------------------------
 * Compatibility with exec interface types
 * ---------------------------------------------------------------- */

/* Convert an exec interface enumerated typeinfo structure into an
 * MCTypeInfoRef for an MCEnumRef enumerated type.  The r_typeinfo
 * will be a named typeinfo with the specified p_name. */
bool MCTypeInfoFromExecTypeInfo (MCExecEnumTypeInfo *p_info, MCNameRef p_name, MCTypeInfoRef & r_typeinfo);

/* ----------------------------------------------------------------
 * Initialization and finalization
 * ---------------------------------------------------------------- */

bool MCEngineTypeInfoInitialize (void);
void MCEngineTypeInfoFinalize (void);

#endif /* !_MC_TYPEINFO_H_ */
