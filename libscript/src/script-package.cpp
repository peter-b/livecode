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
    MCMemoryDeleteArray (self -> modules);
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

bool
MCScriptRemovePackageFromModule(MCScriptModuleRef x_module)
{
	MCAssert (NULL != x_module);

	MCScriptPackageRef t_package;
	t_package = x_module->package;

	if (NULL == t_package)
		return true;

	for (size_t i = 0; i < t_package->module_count; ++i)
	{
		if (t_package->modules[i] != x_module)
			continue;

		/* Remove the module from the package's module list, filling
		 * the gap with the last module in the module list. */
		--t_package->module_count;
		t_package->modules[i] =
			t_package->modules[t_package->module_count];
		break;
	}

	/* Release the module's pointer to the package */
	MCScriptReleasePackage (t_package);
	x_module->package = NULL;

	return true;
}

bool
MCScriptAddPackageToModule(MCScriptModuleRef x_module,
                           MCScriptPackageRef x_package)
{
	MCAssert (NULL != x_module);
	MCAssert (NULL != x_package);

	if (x_module->package == x_package)
		return true;

	/* Delete the module's existing package reference */
	if (!MCScriptRemovePackageFromModule (x_module))
		return false;

	/* Expand the package's module list and append the module */
	if (!MCMemoryResizeArray (x_package->module_count + 1,
	                          x_package->modules,
	                          x_package->module_count))
		return false;

	x_package->modules[x_package->module_count - 1] = x_module;

	/* Add a strong reference from the module to the package */
	x_module->package = MCScriptRetainPackage (x_package);

	return true;
}
