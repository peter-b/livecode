/* Copyright (C) 2003-2013 Runtime Revolution Ltd.
 
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

#include "script.h"
#include "script-private.h"
#include "foundation-auto.h"

////////////////////////////////////////////////////////////////////////////////

void MCScriptDestroyPackage(MCScriptPackageRef self)
{
    __MCScriptValidateObjectAndKind__(self, kMCScriptObjectKindPackage);
    
    MCValueRelease(self -> filename);
    MCValueRelease(self -> name);
    MCScriptReleaseArray(self -> modules, self -> module_count);
}

////////////////////////////////////////////////////////////////////////////////

MCScriptPackageRef
MCScriptRetainPackage (MCScriptPackageRef self)
{
	__MCScriptValidateObjectAndKind__ (self, kMCScriptObjectKindPackage);

	return (MCScriptPackageRef) MCScriptRetainObject (self);
}

void
MCScriptReleasePackage (MCScriptPackageRef self)
{
	if (nil == self)
		return;

	__MCScriptValidateObjectAndKind__ (self, kMCScriptObjectKindPackage);

	MCScriptReleaseObject (self);
}

////////////////////////////////////////////////////////////////////////////////

bool
MCScriptCopyModulesOfPackage (MCScriptPackageRef self,
                              MCProperListRef & r_module_names)
{
	MCAutoProperListRef t_module_names;

	MCAssert (NULL != self);

	if (!MCProperListCreateMutable (&t_module_names))
		return false;

	for (uindex_t i = 0; i < self->module_count; ++i)
	{
		if (!MCProperListPushElementOntoBack (*t_module_names,
		                                      self->modules[i]->name))
			return false;
	}

	if (!MCProperListCopy (*t_module_names, r_module_names))
		return false;

	return true;
}
