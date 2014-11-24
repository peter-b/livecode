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

#include "prefix.h"

#include "globdefs.h"
#include "filedefs.h"
#include "objdefs.h"
#include "parsedef.h"

#include "player.h"
#include "exec.h"

#ifdef FEATURE_PLATFORM_PLAYER
#include "player-platform.cpp"
#else
#include "player-legacy.cpp"
#endif

////////////////////////////////////////////////////////////////////////////////

MCPropertyInfo MCPlayer::kProperties[] =
{
	DEFINE_RW_OBJ_PROPERTY(P_FILE_NAME, OptionalString, MCPlayer, FileName)
	DEFINE_RW_OBJ_PROPERTY(P_DONT_REFRESH, Bool, MCPlayer, DontRefresh)
    // PM-2014-09-15: [[ Bug 13437 ]] Make sure the time-related vars are large enough to hold larger time values
	DEFINE_RW_OBJ_PROPERTY(P_CURRENT_TIME, UInt32, MCPlayer, CurrentTime)
    // PM-2014-11-03: [[ Bug 13920 ]] Make sure the loadedTime property is defined
    DEFINE_RO_OBJ_PROPERTY(P_MOVIE_LOADED_TIME, UInt32, MCPlayer, LoadedTime)
	DEFINE_RO_OBJ_PROPERTY(P_DURATION, UInt32, MCPlayer, Duration)
	DEFINE_RW_OBJ_PROPERTY(P_LOOPING, Bool, MCPlayer, Looping)
	DEFINE_RW_OBJ_PROPERTY(P_PAUSED, Bool, MCPlayer, Paused)
	DEFINE_RW_OBJ_PROPERTY(P_ALWAYS_BUFFER, Bool, MCPlayer, AlwaysBuffer)
	DEFINE_RW_OBJ_PROPERTY(P_PLAY_RATE, Double, MCPlayer, PlayRate)
	DEFINE_RW_OBJ_PROPERTY(P_START_TIME, OptionalUInt32, MCPlayer, StartTime)
	DEFINE_RW_OBJ_PROPERTY(P_END_TIME, OptionalUInt32, MCPlayer, EndTime)
	DEFINE_RW_OBJ_PROPERTY(P_SHOW_BADGE, Bool, MCPlayer, ShowBadge)
	DEFINE_RW_OBJ_PROPERTY(P_SHOW_CONTROLLER, Bool, MCPlayer, ShowController)
	DEFINE_RW_OBJ_PROPERTY(P_PLAY_SELECTION, Bool, MCPlayer, PlaySelection)
	DEFINE_RW_OBJ_PROPERTY(P_SHOW_SELECTION, Bool, MCPlayer, ShowSelection)
	DEFINE_RW_OBJ_PROPERTY(P_CALLBACKS, String, MCPlayer, Callbacks)
    DEFINE_RW_OBJ_PROPERTY(P_MOVIE_CONTROLLER_ID, Int32, MCPlayer, MovieControllerId)
	DEFINE_RO_OBJ_SET_PROPERTY(P_MEDIA_TYPES, InterfaceMediaTypes, MCPlayer, MediaTypes)
	DEFINE_RW_OBJ_PROPERTY(P_CURRENT_NODE, UInt16, MCPlayer, CurrentNode)
	DEFINE_RW_OBJ_PROPERTY(P_PAN, Double, MCPlayer, Pan)
	DEFINE_RW_OBJ_PROPERTY(P_TILT, Double, MCPlayer, Tilt)
	DEFINE_RW_OBJ_PROPERTY(P_ZOOM, Double, MCPlayer, Zoom)
	DEFINE_RO_OBJ_PROPERTY(P_TRACKS, String, MCPlayer, Tracks)
    DEFINE_RO_OBJ_PROPERTY(P_TRACK_COUNT, UInt32, MCPlayer, TrackCount)
	DEFINE_RO_OBJ_PROPERTY(P_NODES, String, MCPlayer, Nodes)
	DEFINE_RO_OBJ_PROPERTY(P_HOT_SPOTS, String, MCPlayer, HotSpots)
	DEFINE_RO_OBJ_CUSTOM_PROPERTY(P_CONSTRAINTS, MultimediaQTVRConstraints, MCPlayer, Constraints)
    DEFINE_RO_OBJ_LIST_PROPERTY(P_ENABLED_TRACKS, LinesOfUInt, MCPlayer, EnabledTracks)
    DEFINE_RW_OBJ_PROPERTY(P_PLAY_LOUDNESS, UInt16, MCPlayer, PlayLoudness)
    DEFINE_RO_OBJ_PROPERTY(P_TIME_SCALE, UInt32, MCPlayer, TimeScale)
    DEFINE_RW_OBJ_CUSTOM_PROPERTY(P_FORE_COLOR, InterfaceNamedColor, MCPlayer, ForeColor)
    DEFINE_RW_OBJ_CUSTOM_PROPERTY(P_HILITE_COLOR, InterfaceNamedColor, MCPlayer, HiliteColor)
    // SN-2014-08-06: [[ Bug 13115 ]] Missing formatted (width|height) in the property table
    DEFINE_RO_OBJ_PROPERTY(P_FORMATTED_HEIGHT, Int32, MCPlayer, FormattedHeight)
    DEFINE_RO_OBJ_PROPERTY(P_FORMATTED_WIDTH, Int32, MCPlayer, FormattedWidth)
#ifdef FEATURE_PLATFORM_PLAYER
    DEFINE_RO_OBJ_ENUM_PROPERTY(P_STATUS, InterfacePlayerStatus, MCPlayer, Status)
#endif
};

MCObjectPropertyTable MCPlayer::kPropertyTable =
{
	&MCControl::kPropertyTable,
	sizeof(kProperties) / sizeof(kProperties[0]),
	&kProperties[0],
};

////////////////////////////////////////////////////////////////

bool
MCPlayer::PopulateState (MCRecordRef x_state) const
{
	MCTypeInfoRef t_typeinfo;
	MCAssert(GetStateTypeInfo(t_typeinfo));
	MCAssert(MCRecordTypeInfoIsDerivedFrom(MCValueGetTypeInfo (x_state),
	                                       t_typeinfo));

	/* FIXME fill in the state record */
	return MCControl::PopulateState (x_state);
}

bool
MCPlayer::ApplyState (MCRecordRef p_state)
{
	MCTypeInfoRef t_typeinfo;
	MCAssert(GetStateTypeInfo(t_typeinfo));
	MCAssert(MCRecordTypeInfoIsDerivedFrom(MCValueGetTypeInfo (p_state),
	                                       t_typeinfo));

	/* FIXME reset the object using p_state */

	return MCControl::ApplyState (p_state);
}

bool
MCPlayer::GetStateTypeInfo (MCTypeInfoRef & r_type_info) const
{
	static MCTypeInfoRef s_type_info = NULL;

	static const MCRecordTypeFieldInfo s_type_info_fields[] = {
		{ nil, kMCNullTypeInfo },
	};
	if (s_type_info == NULL)
	{
		MCTypeInfoRef t_super_type_info;
		if (!(MCControl::GetStateTypeInfo (t_super_type_info) &&
			  MCRecordTypeInfoCreate (s_type_info_fields,
									 -1,
									 t_super_type_info,
									  s_type_info)))
			return false;
	}

	r_type_info = s_type_info;
	return true;
}
