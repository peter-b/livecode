/*                                                                   -*- c++ -*-
Copyright (C) 2003-2015 Runtime Revolution Ltd.

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

#ifndef _MC_TYPEINFO_PRIVATE_H_
#define _MC_TYPEINFO_PRIVATE_H_

#include "typeinfo.h"

/* This header should only be included by typeinfo*.cpp files. */

/* ----------------------------------------------------------------
 * Initialization and finalization
 * ---------------------------------------------------------------- */

/* Create the audioclip format enumerated type */
bool MCAudioClipFormatTypeInfoInitialize(void);

/* Create the text style enumerated type */
bool MCTextStyleEnumTypeInfoInitialize(void);

/* Create the rectangle record type */
bool MCRectangleRecordTypeInfoInitialize (void);

/* Create the object ID custom type */
bool MCObjectIdCustomTypeInfoInitialize (void);

#endif /* !_MC_TYPEINFO_PRIVATE_H_ */
