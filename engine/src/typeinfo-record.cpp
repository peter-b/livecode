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
 * Rectangle record
 * ================================================================ */

/* ----------------------------------------------------------------
 * [Private] Initialization
 * ---------------------------------------------------------------- */

bool
MCRectangleRecordTypeInfoInitialize (void)
{
	static const MCRecordTypeFieldInfo s_type_info_fields[] = {
		{ MCNAME ("x"), kMCNumberTypeInfo },
		{ MCNAME ("y"), kMCNumberTypeInfo },
		{ MCNAME ("width"), kMCNumberTypeInfo },
		{ MCNAME ("height"), kMCNumberTypeInfo },
		{ nil, kMCNullTypeInfo }, /* Custodian */
	};
	static const char s_type_info_name[] = "com.livecode.interface.Rectangle";

	MCAutoTypeInfoRef t_raw_typeinfo;
	if (!MCRecordTypeInfoCreate (s_type_info_fields,
	                             -1,
	                             kMCNullTypeInfo,
	                             &t_raw_typeinfo))
		return false;

	MCAutoTypeInfoRef t_typeinfo;
	if (!MCNamedTypeInfoCreate (MCNAME (s_type_info_name), &t_typeinfo))
		return false;

	kMCRectangleRecordTypeInfo = MCValueRetain (*t_typeinfo);
	return true;
}

/* ----------------------------------------------------------------
 * [Public] Conversion to/from rectangle structs
 * ---------------------------------------------------------------- */

bool
MCRectangleRecordFromStruct (MCRectangle & p_rect,
                             MCRecordRef & r_record)
{
	MCAutoNumberRef t_num_x, t_num_y, t_num_width, t_num_height;
	if (!(MCNumberCreateWithInteger (p_rect.x, &t_num_x) &&
	      MCNumberCreateWithInteger (p_rect.y, &t_num_y) &&
	      MCNumberCreateWithUnsignedInteger (p_rect.width, &t_num_width) &&
	      MCNumberCreateWithUnsignedInteger (p_rect.height, &t_num_height)))
		return false;

	MCValueRef t_record_values[] = { *t_num_x, *t_num_y,
	                                 *t_num_width, *t_num_height };

	return MCRecordCreate (kMCRectangleRecordTypeInfo,
	                       t_record_values,
	                       4,
	                       r_record);
}

bool
MCRectangleRecordToStruct (MCRecordRef p_record,
                           MCRectangle & r_rect)
{
	MCAssert (MCTypeInfoConforms (MCValueGetTypeInfo (p_record),
	                              kMCRectangleRecordTypeInfo));

	MCValueRef t_num_x, t_num_y, t_num_width, t_num_height;
	if (!(MCRecordFetchValue (p_record, MCNAME("x"), t_num_x) &&
	      MCRecordFetchValue (p_record, MCNAME("y"), t_num_y) &&
	      MCRecordFetchValue (p_record, MCNAME("width"), t_num_width) &&
	      MCRecordFetchValue (p_record, MCNAME("height"), t_num_height)))
		return false;

	/* FIXME Check for overflow */
	r_rect.x = MCNumberFetchAsInteger ((MCNumberRef) t_num_x);
	r_rect.y = MCNumberFetchAsInteger ((MCNumberRef) t_num_y);
	r_rect.width = MCNumberFetchAsUnsignedInteger ((MCNumberRef) t_num_width);
	r_rect.width = MCNumberFetchAsUnsignedInteger ((MCNumberRef) t_num_height);

	return true;
}

/* ================================================================
 * [Public] Typeinfo constants
 * ================================================================ */

MCTypeInfoRef kMCRectangleRecordTypeInfo;
