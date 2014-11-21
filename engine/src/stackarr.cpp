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

#include "prefix.h"

#include "globdefs.h"
#include "filedefs.h"
#include "objdefs.h"
#include "parsedef.h"
#include "mcio.h"
#include "sysdefs.h"

#include "stackarr.h"

/* ================================================================
 * File-local declarations
 * ================================================================ */

MCNameRef kMCStackarrCustomKey;
MCNameRef kMCStackarrInternalKey;
MCNameRef kMCStackarrKindKey;
MCNameRef kMCStackarrLiteralKey;
MCNameRef kMCStackarrParentKey;
MCNameRef kMCStackarrSharedKey;
MCNameRef kMCStackarrTypeKey;

/* ================================================================
 * High-level entry points
 * ================================================================ */

/* ----------------------------------------------------------------
 * [Public] Initialisation
 * ---------------------------------------------------------------- */

void
MCStackarrInitialize (void)
{
	static bool s_once = false;
	if (s_once) return;

	/* Constants */
	kMCStackarrCustomKey   = MCNAME("_custom");
	kMCStackarrInternalKey = MCNAME("_internal");
	kMCStackarrKindKey     = MCNAME("_kind");
	kMCStackarrLiteralKey  = MCNAME("_literal");
	kMCStackarrParentKey   = MCNAME("_parent");
	kMCStackarrSharedKey   = MCNAME("_shared");
	kMCStackarrTypeKey     = MCNAME("_type");

	s_once = true;
}
