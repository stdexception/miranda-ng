/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-05 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#pragma once

#include <windows.h>
#include <vssym32.h>
#include <Uxtheme.h>

#include <newpluginapi.h>
#include <m_clistint.h>
#include <m_database.h>
#include <m_gui.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_protosvc.h>

#include "resource.h"
#include "version.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	CMOption<bool> bSortByStatus, bSortByProto;

	int Load() override;
};

struct ClcCacheEntry : public ClcCacheEntryBase {};

struct ClcContact : public ClcContactBase {};

struct ClcData : public ClcDataBase
{
	HWND hwnd_list;
	bool bNeedsRebuild;
};

// shared vars
extern CLIST_INTERFACE coreCli;
extern int g_bSortByStatus, g_bSortByProto;

wchar_t* MyDBGetContactSettingTString(MCONTACT hContact, char *module, char *setting, wchar_t *out, size_t len, wchar_t *def);

int CompareContacts(const ClcContact *contact1, const ClcContact *contact2);
