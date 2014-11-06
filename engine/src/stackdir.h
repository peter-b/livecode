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

#ifndef _MC_STACKDIR_H_
#define _MC_STACKDIR_H_

/* Expanded LiveCode Stacks provide a (mostly) plain text
 * representation of the contents of a LiveCode stack.
 *
 * This API is used to convert an MCArrayRef representation of the
 * stack state to and from the corresponding on-disk representation.
 */

enum MCStackdirResult
{
	kMCStackdirResultOurs = 1,
	kMCStackdirResultTheirs = 2,
	kMCStackdirResultDefault = kMCStackdirResultOurs,
};

enum MCStackdirStatus
{
	kMCStackdirStatusSuccess     = 0,    /* Success */

	kMCStackdirStatusOutOfMemory = 1,    /* Memory allocation error */
	kMCStackdirStatusIOError     = 2,    /* Error during file IO */
	kMCStackdirStatusSyntaxError = 3,    /* Syntax error */
	kMCStackdirStatusConflict    = 4,    /* Unrecoverable VCS conflict */

	kMCStackdirStatusBadPath     = 5,    /* Stack path is invalid */
	kMCStackdirStatusBadState    = 6,    /* Stack state array is invalid */
};

typedef struct _MCStackdirIO MCStackdirIO;
typedef MCStackdirIO *MCStackdirIORef;

/* ----------------------------------------------------------------
 * Basic functions common to all expanded stackfile ops
 * ---------------------------------------------------------------- */

/* Set the path of the expanded stackfile to operate on.
 *
 * p_path must be an standard (non-native) path.
 */
bool MCStackdirIOSetPath (MCStackdirIORef op, MCStringRef p_path);

/* Retrieve the path of the expanded stackfile.
 *
 * r_path will be returned as an standard (non-native) path.
 */
void MCStackdirIOGetPath (MCStackdirIORef op, MCStringRef & r_path);

/* Perform the expanded stack operation.
 *
 * Check the result using MCStackdirIOGetStatus().
 */
void MCStackdirIOCommit (MCStackdirIORef op);

/* Get the result of performing the expanded stack operation.
 *
 * Returns additional error information in the r_error_info, unless
 * nil is passed.  Even if the operation is successful, warning
 * information may be available in the r_error_info.  If the
 * MCStackdirStatus is kMCStackdirStatusOOM, then the r_error_info may be
 * incomplete.
 *
 * The r_error_info is an array with integer keys, starting from 1.
 */
MCStackdirStatus MCStackdirIOGetStatus (MCStackdirIORef op, MCArrayRef *r_error_info);

/* Clean up a expanded stack operation. */
void MCStackdirIODestroy (MCStackdirIORef & op);

/* ----------------------------------------------------------------
 * Save operations
 * ---------------------------------------------------------------- */

/* Create a new save operation */
bool MCStackdirIONewSave (MCStackdirIORef & op);

/* Provide a set of state data to save */
void MCStackdirIOSetState (MCStackdirIORef op, MCArrayRef p_state);

/* ----------------------------------------------------------------
 * Load operations
 * ---------------------------------------------------------------- */

/* Create a new load operation */
bool MCStackdirIONewLoad (MCStackdirIORef & op);

/* Set whether conflicts are permitted.
 *
 * If enabled is false, loading will fail if a conflict is
 * detected.
 */
void MCStackdirIOSetConflictPermitted (MCStackdirIORef op, bool enabled);

/* Query whether conflicts are permitted. */
bool MCStackdirIOGetConflictPermitted (MCStackdirIORef op);

/* Query whether a conflict was detected.
 *
 * If the operation has not yet been performed, returns false.
 */
bool MCStackdirIOHasConflict (MCStackdirIORef op);

/* Retrieve the state and/or source location information.
 *
 * If either the r_state or r_source_info arguments are nil, no data
 * will be returned.  If a conflict was detected, "yours" and "ours"
 * versions of the information can be obtained by passing
 * kMCStackdirResultOurs or kMCStackdirResultTheirs in the p_mode.
 *
 * If the output variables were not set (e.g. because an out of memory
 * error occurred or the operation has not yet been performed),
 * returns false.  Otherwise, returns true.
 */
bool MCStackdirIOGetState (MCStackdirIORef op,
						   MCArrayRef *r_state, MCArrayRef *r_source_info,
						   MCStackdirResult mode = kMCStackdirResultDefault);

/* ----------------------------------------------------------------
 * Internal debugging commands
 * ---------------------------------------------------------------- */

void MCStackdirExecInternalSave (MCExecContext & ctxt, MCStringRef p_path, MCArrayRef p_state_array);

#endif /* ! _MC_STACKDIR_H_ */
