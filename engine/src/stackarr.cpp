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

#include "stack.h"
#include "card.h"
#include "group.h"
#include "aclip.h"
#include "vclip.h"
#include "uuid.h"

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

/* Create a name from a UUID */
static bool MCStackarrNameCreateFromUuid (const MCUuid & p_uuid, MCNameRef & r_name);

/* Capture an object into x_typed_state, with the given p_parent. */
/* FIXME These would arguably be better as MCObject methods. */
static bool MCStackarrCaptureObject (MCStack *object, MCStack *p_parent, MCArrayRef x_typed_state);
static bool MCStackarrCaptureObject (MCControl *stack, MCObject *p_parent, MCArrayRef x_typed_state);
static bool MCStackarrCaptureObject (MCObject *stack, MCObject *p_parent, MCArrayRef x_typed_state);

/* ================================================================
 * Utility functions
 * ================================================================ */

static bool
MCStackarrNameCreateFromUuid (const MCUuid & p_uuid, MCNameRef & r_name)
{
	char t_string_buf[kMCUuidCStringLength];
	MCUuidToCString(p_uuid, t_string_buf);
	return MCNameCreateWithNativeChars ((char_t *) t_string_buf,
	                                    kMCUuidStringLength,
	                                    r_name);
}

/* ================================================================
 * Stack capture
 * ================================================================ */

/* This template exists in order to make iterating over MCDLlist
 * less insane */
template <class List, class Parent>
bool MCStackarrCaptureList(List p_list, Parent p_parent,
                           MCArrayRef & r_typed_state)
{
	if (p_list != nil)
	{
		List p_obj = p_list;
		do
		{
			if (!MCStackarrCaptureObject(p_obj, p_parent, r_typed_state))
				return false;
			p_obj = p_obj->next ();
		}
		while (p_obj != p_list);
	}
	return true;
}

static bool
MCStackarrCaptureObject (MCStack *stack,
                         MCStack *p_parent,
                         MCArrayRef x_typed_state)
{
	/* Capture the stack's properties */
	if (!MCStackarrCaptureObject (static_cast<MCObject *> (stack),
	                              p_parent,
	                              x_typed_state))
		return false;

	/* Capture the stack's cards */
	if (!MCStackarrCaptureList (stack->getcards (), stack, x_typed_state))
		return false;

	/* Capture the controls */
	if (!MCStackarrCaptureList (stack->getcontrols (), stack, x_typed_state))
		return false;

	/* Capture the multimedia */
	if (!MCStackarrCaptureList (stack->getaclips (), stack, x_typed_state))
		return false;
	if (!MCStackarrCaptureList (stack->getvclips (), stack, x_typed_state))
		return false;

	/* Capture the substacks */
	if (!MCStackarrCaptureList (stack->getsubstacks (), stack, x_typed_state))
		return false;

	return true;
}

static bool
MCStackarrCaptureObject (MCControl *control,
                         MCObject *p_parent,
                         MCArrayRef x_typed_state)
{
	/* Capture the control's properties */
	if (!MCStackarrCaptureObject (static_cast<MCObject *> (control),
	                              p_parent,
	                              x_typed_state))
		return false;

	/* Iff this is a group, we need to capture the controls it contains. */
	if (control->gettype() == CT_GROUP)
	{
		MCGroup *t_group;
		t_group = static_cast<MCGroup *> (control);
		if (!MCStackarrCaptureList (t_group->getcontrols (),
		                            t_group, x_typed_state))
			return false;
	}

	return true;
}

static bool
MCStackarrCaptureObject (MCObject *object,
                         MCObject *p_parent,
                         MCArrayRef x_typed_state)
{

	/* ---------- 1. Create object's state array */
	/* Build the state array */
	MCAutoArrayRef t_state;
	if (!MCArrayCreateMutable (&t_state)) return false;

	/* ---------- 2. Store internal properties */
	MCAutoRecordRef t_record;
	MCAutoArrayRef t_internal_props;
	if (!(object->ExportState (&t_record) &&
	      MCRecordEncodeAsArray (*t_record, &t_internal_props) &&
	      MCArrayStoreValue (*t_state, true,
	                         kMCStackarrInternalKey,
	                         *t_internal_props)))
		return false;

	/* ---------- 3. Store the custom properties */
	MCAutoArrayRef t_custom;
	if (!(object->ExportCustomState (&t_custom) &&
	      MCArrayStoreValue (*t_state, true,
	                         kMCStackarrCustomKey,
	                         *t_custom)))
		return false;

	/* ---------- 4. Store the parent reference */
	if (p_parent != nil)
	{
		MCUuid t_parent_uuid;
		MCNewAutoNameRef t_parent_uuid_key;
		if (!(p_parent->GetUuid (t_parent_uuid) &&
		      MCStackarrNameCreateFromUuid (t_parent_uuid, &t_parent_uuid_key) &&
		      MCArrayStoreValue (*t_state, true,
		                         kMCStackarrParentKey,
		                         *t_parent_uuid_key)))
			return false;
	}

	/* ---------- 5. Store the object kind */
	MCNewAutoNameRef t_kind_key;
	const char *t_kind = object->gettypestring ();
	if (!(MCNameCreateWithNativeChars ((const char_t *) t_kind,
	                                   strlen (t_kind), &t_kind_key) &&
	      MCArrayStoreValue (*t_state, true,
	                         kMCStackarrKindKey,
	                         *t_kind_key)))
		return false;

	/* ---------- 6. Store the per-card data */

	/* If the object supports per-card data (i.e. its per-card data
	 * record typeinfo isn't the null typeinfo) then check if there
	 * are any cards.  If both these conditions are satisfied, iterate
	 * over all the cards and get the per-card data, if any. */
	MCTypeInfoRef t_shared_typeinfo;
	if (!object->GetSharedStateTypeInfo (t_shared_typeinfo))
		return false;

	MCCard *t_card_list = object -> getstack () -> getcards ();
	if (t_shared_typeinfo != kMCNullTypeInfo &&
	    t_card_list != nil)
	{
		MCAutoArrayRef t_shared;
		if (!MCArrayCreateMutable (&t_shared)) return false;

		/* Iterate over the cards in the object's stack */
		MCCard *t_card = t_card_list;
		do
		{
			MCAutoRecordRef t_card_data;
			if (!object->ExportSharedState (t_card, &t_card_data))
				return false;

			MCUuid t_card_uuid;
			MCNewAutoNameRef t_card_uuid_key;
			if (&t_card_data != nil)
			{
				if(!(object->GetUuid (t_card_uuid) &&
				     MCStackarrNameCreateFromUuid (t_card_uuid,
				                                   &t_card_uuid_key) &&
				     MCArrayStoreValue (*t_shared, false,
				                        *t_card_uuid_key,
				                        *t_card_data)))
					return false;
			}
		}
		while (t_card != t_card_list);

		/* Only store a per-card data array if there was actually some
		 * per-card data found */
		if (!MCArrayIsEmpty (*t_shared))
		{
			MCAutoArrayRef t_immutable_shared;
			if (!(MCArrayCopy (*t_shared, &t_immutable_shared) &&
			      MCArrayStoreValue (*t_state, true,
			                         kMCStackarrSharedKey,
			                         *t_immutable_shared)))
				return false;
		}
	}

	/* ---------- 7. Store the object's state array in the result array. */

	/* Create a key from the object's UUID */
	MCUuid t_uuid;
	MCNewAutoNameRef t_uuid_key;
	if (!(object->GetUuid (t_uuid) &&
	      MCStackarrNameCreateFromUuid (t_uuid, &t_uuid_key)))
		return false;

	/* Convert object's state into immutable array and store it */
	MCAutoArrayRef t_immutable_state;
	if (!(MCArrayCopy (*t_state, &t_immutable_state) &&
	      MCArrayStoreValue (x_typed_state, false,
	                         *t_uuid_key,
	                         *t_immutable_state)))
		return false;

	return true;
}

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

/* ----------------------------------------------------------------
 * [Public] Stack capture
 * ---------------------------------------------------------------- */

bool
MCStackarrCaptureStack (MCStack * stack,
                        MCArrayRef & r_typed_state)
{
	MCAssert (stack != nil);
	MCStackarrInitialize ();

	/* Create the result array */
	MCAutoArrayRef t_state;
	if (!MCArrayCreateMutable(&t_state))
		return false;

	/* Capture the stack itself */
	/* The root stack is the only stack without a parent */
	if (!MCStackarrCaptureObject (stack,
	                              static_cast<MCStack *> (NULL),
	                              *t_state))
		return false;

	return MCArrayCopy(*t_state, r_typed_state);
}
