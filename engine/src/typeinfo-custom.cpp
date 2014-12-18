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
 * [Private] File-local declarations
 * ================================================================ */

/* ---------- Object IDs */

/* Get the object ID pointer from a custom value */
static MCDelayedObjectId *MCObjectIdCustomGet (MCValueRef p_custom);
/* Set the object ID pointer in a custom value */
static void MCObjectIdCustomSet (MCValueRef p_custom, MCDelayedObjectId *p_id);
/* Custom type destructor callback */
static void MCObjectIdCustomDestroy (MCValueRef p_self);

/* ================================================================
 * Object IDs
 * ================================================================ */

/* ----------------------------------------------------------------
 * [Private] Accessors
 * ---------------------------------------------------------------- */

static MCDelayedObjectId *
MCObjectIdCustomGet (MCValueRef p_custom)
{
	void *t_ptr = MCValueGetExtraBytesPtr (p_custom);
	MCDelayedObjectId **t_contents = static_cast<MCDelayedObjectId **>(t_ptr);
	return *t_contents;
}

static void
MCObjectIdCustomSet (MCValueRef p_custom,
                     MCDelayedObjectId *p_id)
{
	void *t_ptr = MCValueGetExtraBytesPtr (p_custom);
	MCDelayedObjectId **t_contents = static_cast<MCDelayedObjectId **>(t_ptr);
	*t_contents = p_id;
}

/* ----------------------------------------------------------------
 * [Private] Custom typeinfo callbacks
 * ---------------------------------------------------------------- */

/* Only really need to override the destructor for now */
static const MCValueCustomCallbacks kMCObjectIdCustomCallbacks = {
	true,
	MCObjectIdCustomDestroy, /* destroy */
	NULL, /* copy */
	NULL, /* equal */ /* FIXME may want to implement this at some point */
	NULL, /* hash */  /* FIXME may want to implement this at some point */
	NULL, /* describe */
	NULL, /* is_mutable */
	NULL, /* mutable_copy */
};

static void
MCObjectIdCustomDestroy (MCValueRef p_self)
{
	MCDelayedObjectId *t_id = MCObjectIdCustomGet (p_self);
	delete t_id;
}

/* ----------------------------------------------------------------
 * [Private] Initialization
 * ---------------------------------------------------------------- */

bool
MCObjectIdCustomTypeInfoInitialize (void)
{
	const char *k_type_name = "com.livecode.interface.object.idref";

	MCAutoTypeInfoRef t_raw_typeinfo;
	if (!MCCustomTypeInfoCreate (kMCNullTypeInfo, &kMCObjectIdCustomCallbacks,
	                             &t_raw_typeinfo))
		return false;

	MCAutoTypeInfoRef t_typeinfo;
	if (!(MCNamedTypeInfoCreate (MCNAME (k_type_name), &t_typeinfo) &&
	      MCNamedTypeInfoBind (*t_typeinfo, *t_raw_typeinfo)))
		return false;

	MCAutoTypeInfoRef t_optional_typeinfo;
	if (!MCOptionalTypeInfoCreate (*t_typeinfo, &t_optional_typeinfo))
		return false;

	kMCObjectIdCustomTypeInfo = MCValueRetain (*t_typeinfo);
	kMCOptionalObjectIdCustomTypeInfo = MCValueRetain (*t_optional_typeinfo);
	return true;
}

/* ----------------------------------------------------------------
 * [Public] Conversion to/from object ID instances
 * ---------------------------------------------------------------- */

bool
MCObjectIdValueFromInstance (const MCObjectId *p_id,
                             MCValueRef & r_value)
{
	/* Create a new custom value instance */
	MCAutoValueRef t_value;
	if (!MCValueCreateCustom (kMCObjectIdCustomTypeInfo,
	                          sizeof (MCDelayedObjectId *),
	                          &t_value))
		return false;

	/* Copy the object ID into a new delayed ID */
	MCDelayedObjectId *t_copy_id;
	t_copy_id = new MCDelayedObjectId(p_id);

	if (!t_copy_id)
		return false;

	MCObjectIdCustomSet (*t_value, t_copy_id);

	r_value = MCValueRetain (*t_value);
	return true;
}

const MCObjectId *
MCObjectIdValueToInstance (MCValueRef p_value)
{
	return MCObjectIdCustomGet (p_value);
}

/* ================================================================
 * [Public] Typeinfo constants
 * ================================================================ */

MCTypeInfoRef kMCObjectIdCustomTypeInfo;
MCTypeInfoRef kMCOptionalObjectIdCustomTypeInfo;
