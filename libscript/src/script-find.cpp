/*                                                                     -*-c++-*-
Copyright (C) 2015 Runtime Revolution Ltd.

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

#include <foundation.h>
#include <foundation-system.h>
#include <foundation-auto.h>

#include <script.h>
#include <script-auto.h>
#include "script-private.h"

#ifdef _WIN32
#	include <windows.h>
#else
#	include <unistd.h>
#endif

#define MC_SCRIPT_FIND_ENV "LC_MODULE_PATH"

////////////////////////////////////////////////////////////////////////////////

static MCProperListRef s_search_path = NULL;

bool
MCScriptSetModuleSearchPath (MCProperListRef p_directory_list)
{
	MCAssert (NULL != p_directory_list);
	MCValueRelease (s_search_path);
	return MCProperListCopy (p_directory_list, s_search_path);
}

MCProperListRef
MCScriptGetModuleSearchPath (void)
{
	return s_search_path;
}

////////////////////////////////////////////////////////////////////////////////

bool
MCScriptCreateModuleFromSearch (MCNameRef p_name,
                                MCScriptModuleRef & r_module)
{
	MCProperListRef t_search_path;
	t_search_path = MCScriptGetModuleSearchPath();

	MCAutoDataRef t_module_data;
	MCAutoStringRef t_module_dir;

	uintptr_t t_iter = 0;
	MCValueRef t_element = NULL;
	while (MCProperListIterate (t_search_path, t_iter, t_element))
	{
		MCStringRef t_dir = (MCStringRef) t_element;

		/* ---------- Construct candidate filename */
		/* FIXME there should be a generic function to do this
		 * filename construction. */
		MCAutoStringRef t_filename;
		if (!MCStringMutableCopy (t_dir, &t_filename))
			return false;

		const char *t_format;
		if (!MCStringEndsWithCString (*t_filename, (const char_t *) "/",
		                              kMCStringOptionCompareExact))
			t_format = "/%@.lcm";
		else
			t_format = "%@.lcm";

		if (!MCStringAppendFormat (*t_filename, t_format, p_name))
			return false;

		/* ---------- Attempt to load the file */
		/* FIXME this should distinguish between two exceptional
		 * cases: 1) the file is missing (in which case continue
		 * searching other directories) and 2) the file is unreadable
		 * (in which case error). */
		if (!MCSFileGetContents (*t_filename, &t_module_data))
		{
			/* Clear the error and try the next directory */
			MCAutoErrorRef t_error;
			MCErrorCatch (&t_error);
			continue;
		}
		else
		{
			t_module_dir = t_dir;
			break;
		}
	}

	/* ---------- Create the module */
	/* If the file exists but isn't usable as a module, then it's
	 * an error. */
	MCAutoScriptModuleRef t_module;
	if (!MCScriptCreateModuleFromData (*t_module_data, &t_module))
		return false;

	/* FIXME bare modules should get put into a package corresponding
	 * to the the search directory. */

	r_module = MCScriptRetainModule (*t_module);
	return r_module;
}

////////////////////////////////////////////////////////////////////////////////

bool
MCScriptCreateDefaultModuleSearchPath (MCProperListRef & r_directory_list)
{
	/* The "compiled-in" default search path is empty,for now. */
	MCAutoProperListRef t_compiled_in;
	t_compiled_in = kMCEmptyProperList;

	/* ---------- Read environment variable. */
	MCAutoStringRef t_env_string;

#ifdef __WINDOWS__
	{
		MCAutoStringRef t_env_key;
		if (!MCStringCreateWithCString (MC_SCRIPT_FIND_ENV, &t_env_key))
			return false;

		MCAutoStringRefAsWString t_env_key_w32;
		if (!t_env_key_w32.Lock (t_env_key))
			return false;

		MCAutoArray<unichar_t> t_buffer_w32;
		if (!t_buffer_w32.New (1024))
			return false;

		uint32_t t_env_len_w32;
		while (true)
		{
			t_env_len_w32 = GetEnvironmentVariableW (*t_env_key_w32,
			                                         t_buffer_w32.Ptr(),
			                                         t_buffer_w32.Size());

			/* Check whether no variable was found */
			if (0 == t_env_len_w32)
			{
				t_env_string = kMCEmptyString;
				break;
			}

			/* Check if retrieved variable successfully */
			if (t_env_len_w32 <= t_buffer_w32.Size())
			{
				if (!MCStringCreateWithWString (t_buffer_w32.Ptr(),
				                                &t_env_string))
					return false;
				break;
			}

			/* Expand buffer */
			if (!t_buffer_w32.Resize (t_env_len_w32))
				return false;
		}
	}
#else /* ! __WINDOWS__  (non-Windows platforms) */
	{
		/* SECURITY */
		/* Ignore -- and delete! -- the environment variable if in a
		 * secure execution context (typically due to running in a
		 * setgid or setuid program) */
		bool t_secure = false;
		t_secure |= (getuid() != geteuid());
		t_secure |= (getgid() != getegid());

		/* FIXME check whether process has a nonempty permitted
		 * capability set on Linux */

		if (t_secure)
		{
			unsetenv (MC_SCRIPT_FIND_ENV);
			t_env_string = kMCEmptyString;
		}
		else
		{
			char *t_env = getenv (MC_SCRIPT_FIND_ENV);
#	ifdef __LINUX__
			if (!MCStringCreateWithSysString (t_env, &t_env_string))
				return false;
#	else
			if (!MCStringCreateWithBytes (t_env, strlen (t_env),
			                              kMCStringEncodingUTF8,
			                              false,
			                              &t_env_string))
				return false;
#   endif
		}
	}
#endif /* ! __WINDOWS__ */

	/* ---------- Split environment into list. */
	/* We split on [;:]. First replace every incidence of ':' with ';', then
	 * split on ';'. */
	MCAutoStringRef t_env_string_subst;
	if (!MCStringMutableCopy (*t_env_string, &t_env_string_subst))
		return false;
	if (!MCStringFindAndReplaceChar (*t_env_string_subst, ':', ';',
	                                 kMCStringOptionCompareExact))
		return false;

	MCAutoProperListRef t_env_paths;
	if (!MCStringSplitByDelimiter (*t_env_string_subst, MCSTR(";"),
	                               kMCStringOptionCompareExact,
	                               &t_env_paths))
		return false;

	/* ---------- Build final environment list */
	/* Replace the first empty element in the environment search path
	 * with the compiled-in search path, and delete any other empty
	 * elements.  This allows the environment variable to be used to
	 * prepend and/or append elements to the compiled-in search path.
	 * If no empty element is found in the environment search path,
	 * prepend the environment search path to the compiled-in search
	 * path.*/
	bool spliced = false;
	MCAutoProperListRef t_new_search_path;
	if (!MCProperListMutableCopy (*t_env_paths, &t_new_search_path))
		return false;

	for (uindex_t i = 0; i < MCProperListGetLength (*t_new_search_path); ++i)
	{
		MCStringRef t_element;
		t_element = (MCStringRef) MCProperListFetchElementAtIndex (*t_new_search_path, i);
		MCAssert (NULL != t_element);

		if (!MCStringIsEmpty (t_element))
			continue;

		/* Delete empty elements */
		if (spliced)
		{
			if (!MCProperListRemoveElement (*t_new_search_path, i))
				return false;
			continue;
		}

		/* Splice in compiled-in list */
		if (!MCProperListInsertList (*t_new_search_path,
		                             *t_compiled_in, i))
			return false;
		spliced = true;
	}

	if (!spliced)
	{
		if (!MCProperListAppendList (*t_new_search_path, *t_compiled_in))
			return false;
	}

	return MCProperListCopy (*t_new_search_path, r_directory_list);
}
