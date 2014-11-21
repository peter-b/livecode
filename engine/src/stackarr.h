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

#ifndef _MC_STACKARR_H_
#define _MC_STACKARR_H_

/* Stack arrays are a serialized representation of a stack.  They come
 * in two forms:
 *
 * 1. Typed stack arrays have all values in their original type.
 *
 * 2. Detyped stack arrays have all value converted to a (type name,
 *    base value) pair.
 */

/* ----------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------- */

extern MCNameRef kMCStackarrCustomKey;
extern MCNameRef kMCStackarrInternalKey;
extern MCNameRef kMCStackarrKindKey;
extern MCNameRef kMCStackarrLiteralKey;
extern MCNameRef kMCStackarrParentKey;
extern MCNameRef kMCStackarrSharedKey;
extern MCNameRef kMCStackarrTypeKey;

/* ----------------------------------------------------------------
 * High-level stack array functions.
 * ---------------------------------------------------------------- */

/* Initialize stack array support */
void MCStackarrInitialize (void);

bool MCStackarrCaptureStack (MCStack *stack, MCArrayRef & r_typed_state);

bool MCStackarrConstructStack (MCArrayRef p_typed_state, MCStack *& r_stack);

bool MCStackarrMakeTyped (MCArrayRef p_detyped_state, MCArrayRef & r_typed_state);

bool MCStackarrMakeDetyped (MCArrayRef p_typed_state, MCArrayRef & r_detyped_state);

/* ----------------------------------------------------------------
 * Internal debugging commands
 * ---------------------------------------------------------------- */

void MCStackarrExecInternalExport (MCExecContext & ctxt, MCStringRef p_stack, MCVariableChunkPtr p_var);

#endif /* !_MC_STACKARR_H_ */
