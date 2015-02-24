/*                                                                     -*-c++-*-
Copyright (C) 2015 Runtime Revolution Ltd.

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

#include <foundation.h>
#include <foundation-system.h>
#include <foundation-auto.h>

#include <script.h>
#include <script-auto.h>
#include "script-private.h"

#define MC_SCRIPT_FIND_ENV "LC_MODULE_PATH"

////////////////////////////////////////////////////////////////////////////////

static MCProperListRef s_search_path = NULL;

bool
MCScriptSetModuleSearchPath (MCProperListRef p_directory_list)
{
	MCAssert (NULL != p_directory_list);
	MCValueRelease (s_search_path);
	return MCProperListCopy (p_directory_list, s_search_path);
}

MCProperListRef
MCScriptGetModuleSearchPath (void)
{
	return s_search_path;
}
