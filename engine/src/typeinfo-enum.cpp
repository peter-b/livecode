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
