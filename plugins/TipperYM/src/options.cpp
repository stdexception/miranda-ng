/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "stdafx.h"

OPTIONS opt;
ICONSTATE exIcons[EXICONS_COUNT];

// tray tooltip items
static wchar_t *trayTipItems[] =
{
	LPGENW("Number of contacts"),
	LPGENW("Protocol lock status"),
	LPGENW("Logon time"),
	LPGENW("Unread emails"),
	LPGENW("Status"),
	LPGENW("Status message"),
	LPGENW("Extra status"),
	LPGENW("Listening to"),
	LPGENW("Favorite contacts"),
	LPGENW("Miranda uptime"),
	LPGENW("Contact list event")
};

// extra icons
static wchar_t *extraIconName[] =
{
	LPGENW("Status"),
	LPGENW("Extra status"),
	LPGENW("Jabber activity"),
	LPGENW("Gender"),
	LPGENW("Country flag"),
	LPGENW("Client")
};

// preset items

#define MAX_PRESET_SUBST_COUNT	3

struct PRESETITEM
{
	const char *szID;
	const wchar_t *swzName;
	const wchar_t *swzLabel;
	const wchar_t *swzValue;
	const char *szNeededSubst[MAX_PRESET_SUBST_COUNT];
};

static PRESETITEM presetItems[] =
{
	{ "account", LPGENW("Account"), LPGENW("Account:"), L"%sys:account%" },
	{ "birth", LPGENW("Birthday"), LPGENW("Birthday:"), L"%birthday_date% (%birthday_age%) @ Next: %birthday_next%", "birthdate", "birthage", "birthnext" },
	{ "client", LPGENW("Client"), LPGENW("Client:"), L"%raw:/MirVer%" },
	{ "email", LPGENW("Email"), LPGENW("Email:"), L"%raw:/e-mail%" },
	{ "gender", LPGENW("Gender"), LPGENW("Gender:"), L"%gender%" },
	{ "homepage", LPGENW("Homepage"), LPGENW("Homepage:"), L"%raw:/Homepage%" },
	{ "id", LPGENW("Identifier"), L"%sys:uidname|UID^!MetaContacts%:", L"%sys:uid%" },
	{ "idle", LPGENW("Idle"), LPGENW("Idle:"), L"%idle% (%idle_diff% ago)", "idle", "idlediff" },
	{ "ip", L"IP", L"IP:", L"%ip%", "ip" },
	{ "ipint", LPGENW("IP internal"), LPGENW("IP internal:"), L"%ip_internal%", "ipint" },
	{ "lastmsg", LPGENW("Last message"), LPGENW("Last message: (%sys:last_msg_reltime% ago)"), L"%sys:last_msg%" },
	{ "listening", LPGENW("Listening to"), LPGENW("Listening to:"), L"%raw:/ListeningTo%" },
	{ "name", LPGENW("Name"), LPGENW("Name:"), L"%raw:/FirstName|% %raw:/LastName%" },
	{ "received", LPGENW("Number of received messages"), LPGENW("Number of msg [IN]:"), L"%sys:msg_count_in%" },
	{ "sended", LPGENW("Number of sent messages"), LPGENW("Number of msg [OUT]:"), L"%sys:msg_count_out%" },
	{ "status", LPGENW("Status"), LPGENW("Status:"), L"%Status%", "status" },
	{ "statusmsg", LPGENW("Status message"), LPGENW("Status message:"), L"%sys:status_msg%" },
	{ "time", LPGENW("Contact time"), LPGENW("Time:"), L"%sys:time%" },
	{ "xtitle", LPGENW("xStatus title"), LPGENW("xStatus title:"), L"%xsname%", "xname" },
	{ "xtext", LPGENW("xStatus text"), LPGENW("xStatus text:"), L"%raw:/XStatusMsg%" },
	{ "acttitle", LPGENW("[jabber.dll] Activity title"), LPGENW("Activity title:"), L"%raw:AdvStatus/?dbsetting(%subject%,Protocol,p)/activity/title%" },
	{ "acttext", LPGENW("[jabber.dll] Activity text"), LPGENW("Activity text:"), L"%raw:AdvStatus/?dbsetting(%subject%,Protocol,p)/activity/text%" },
	{ "lastseentime", LPGENW("[seenplugin.dll] Last seen time"), LPGENW("Last seen time:"), L"%lastseen_date% @ %lastseen_time%", "lsdate", "lstime" },
	{ "lastseenstatus", LPGENW("[seenplugin.dll] Last seen status"), LPGENW("Last seen status:"), L"%lastseen_status% (%lastseen_ago% ago)", "lsstatus", "lsago" },
	{ "cond", LPGENW("[weather.dll] Condition"), LPGENW("Condition:"), L"%raw:Current/Condition%" },
	{ "humidity", LPGENW("[weather.dll] Humidity"), LPGENW("Humidity:"), L"%raw:Current/Humidity%" },
	{ "minmaxtemp", LPGENW("[weather.dll] Max/Min temperature"), LPGENW("Max/Min:"), L"%raw:Current/High%/%raw:Current/Low%" },
	{ "moon", LPGENW("[weather.dll] Moon"), LPGENW("Moon:"), L"%raw:Current/Moon%" },
	{ "pressure", LPGENW("[weather.dll] Pressure"), LPGENW("Pressure:"), L"%raw:Current/Pressure% (%raw:Current/Pressure Tendency%)" },
	{ "sunrise", LPGENW("[weather.dll] Sunrise"), LPGENW("Sunrise:"), L"%raw:Current/Sunrise%" },
	{ "sunset", LPGENW("[weather.dll] Sunset"), LPGENW("Sunset:"), L"%raw:Current/Sunset%" },
	{ "temp", LPGENW("[weather.dll] Temperature"), LPGENW("Temperature:"), L"%raw:Current/Temperature%" },
	{ "uptime", LPGENW("[weather.dll] Update time"), LPGENW("Update time:"), L"%raw:Current/Update%" },
	{ "uvindex", LPGENW("[weather.dll] UV Index"), LPGENW("UV Index:"), L"%raw:Current/UV% - %raw:Current/UVI%" },
	{ "vis", LPGENW("[weather.dll] Visibility"), LPGENW("Visibility:"), L"%raw:Current/Visibility%" },
	{ "wind", LPGENW("[weather.dll] Wind"), LPGENW("Wind:"), L"%raw:Current/Wind Direction% (%raw:Current/Wind Direction DEG%)/%raw:Current/Wind Speed%" }
};

PRESETITEM* GetPresetItemByName(const char *szName)
{
	for (auto &it : presetItems)
		if (mir_strcmp(it.szID, szName) == 0)
			return &it;

	return nullptr;
}

// preset substitutions

struct PRESETSUBST
{
	const char *szID;
	const wchar_t *swzName;
	const char *szModuleName;
	const char *szSettingName;
	int iTranslateFuncId;
};

PRESETSUBST presetSubsts[] =
{
	{ "gender",    L"gender",           nullptr,     "Gender",       5 },
	{ "status",    L"Status",           nullptr,     "Status",       1 },
	{ "ip",        L"ip",               nullptr,     "IP",           7 },
	{ "ipint",     L"ip_internal",      nullptr,     "RealIP",       7 },
	{ "idle",      L"idle",             nullptr,     "IdleTS",       2 },
	{ "idlediff",  L"idle_diff",        nullptr,     "IdleTS",       3 },
	{ "xname",     L"xsname",           nullptr,     "XStatusName", 17 },
	{ "lsdate",    L"lastseen_date",   "SeenModule", nullptr,        8 },
	{ "lstime",    L"lastseen_time",   "SeenModule", nullptr,       10 },
	{ "lsstatus",  L"lastseen_status", "SeenModule", "OldStatus",    1 },
	{ "lsago",     L"lastseen_ago",    "SeenModule", "seenTS",       3 },
	{ "birthdate", L"birthday_date",    nullptr,     "Birth",        8 },
	{ "birthage",  L"birthday_age",     nullptr,     "Birth",        9 },
	{ "birthnext", L"birthday_next",    nullptr,     "Birth",       12 },
	{ "logondate", L"logon_date",       nullptr,     "LogonTS",     15 },
	{ "logontime", L"logon_time",       nullptr,     "LogonTS",     13 },
	{ "logonago",  L"logon_ago",        nullptr,     "LogonTS",      3 },
};

PRESETSUBST* GetPresetSubstByName(const char *szName)
{
	if (!szName)
		return nullptr;

	for (auto &it : presetSubsts)
		if (mir_strcmp(it.szID, szName) == 0)
			return &it;

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern int IsTrayProto(const wchar_t *swzProto, BOOL bExtendedTip)
{
	if (swzProto == nullptr)
		return 0;

	char szSetting[64];
	if (bExtendedTip)
		mir_strcpy(szSetting, "TrayProtocolsEx");
	else
		mir_strcpy(szSetting, "TrayProtocols");

	DBVARIANT dbv;
	int result = 1;
	if (!g_plugin.getWString(szSetting, &dbv)) {
		result = wcsstr(dbv.pwszVal, swzProto) ? 1 : 0;
		db_free(&dbv);
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// default items

struct
{
	char *szName;
	bool bValueNewline;
}
static defaultItemList[] =
{
	"statusmsg", true,
	"-", false,
	"lastmsg", true,
	"-", false,
	"client", false,
	"homepage", false,
	"email", false,
	"birth", false,
	"name", false,
	"-", false,
	"time", false,
	"id", false,
	"status", false,
};

void CreateDefaultItems()
{
	for (auto &it : defaultItemList) {
		if (it.szName[0] == '-') {
			auto *di_node = (DIListNode *)mir_calloc(sizeof(DIListNode));
			wcsncpy(di_node->di.swzLabel, L"", LABEL_LEN);
			wcsncpy(di_node->di.swzValue, L"", VALUE_LEN);
			di_node->di.bLineAbove = true;
			di_node->di.bIsVisible = true;
			di_node->di.bParseTipperVarsFirst = false;
			di_node->next = opt.diList;
			opt.diList = di_node;
			opt.iDiCount++;
		}
		else {
			auto *item = GetPresetItemByName(it.szName);
			if (item == nullptr)
				continue;

			for (int j = 0; j < MAX_PRESET_SUBST_COUNT; j++) {
				if (auto *subst = GetPresetSubstByName(item->szNeededSubst[j])) {
					auto *ds_node = (DSListNode *)mir_calloc(sizeof(DSListNode));
					wcsncpy(ds_node->ds.swzName, subst->swzName, LABEL_LEN);
					ds_node->ds.type = subst->szModuleName == nullptr ? DVT_PROTODB : DVT_DB;
					strncpy(ds_node->ds.szSettingName, subst->szSettingName, SETTING_NAME_LEN);
					ds_node->ds.iTranslateFuncId = subst->iTranslateFuncId;
					ds_node->next = opt.dsList;
					opt.dsList = ds_node;
					opt.iDsCount++;
				}
			}

			auto *di_node = (DIListNode *)mir_calloc(sizeof(DIListNode));
			wcsncpy(di_node->di.swzLabel, TranslateW(item->swzLabel), LABEL_LEN);
			wcsncpy(di_node->di.swzValue, item->swzValue, VALUE_LEN);
			di_node->di.bLineAbove = false;
			di_node->di.bValueNewline = it.bValueNewline;
			di_node->di.bIsVisible = true;
			di_node->di.bParseTipperVarsFirst = false;
			di_node->next = opt.diList;
			opt.diList = di_node;
			opt.iDiCount++;
		}
	}
}

bool LoadDS(DISPLAYSUBST *ds, int index)
{
	char setting[512];
	DBVARIANT dbv;

	mir_snprintf(setting, "Name%d", index);
	ds->swzName[0] = 0;
	if (db_get_ws(0, MODULE_ITEMS, setting, &dbv))
		return false;

	wcsncpy(ds->swzName, dbv.pwszVal, _countof(ds->swzName));
	ds->swzName[_countof(ds->swzName) - 1] = 0;
	db_free(&dbv);

	mir_snprintf(setting, "Type%d", index);
	ds->type = (DisplaySubstType)db_get_b(0, MODULE_ITEMS, setting, DVT_PROTODB);

	mir_snprintf(setting, "Module%d", index);
	ds->szModuleName[0] = 0;
	if (!db_get_s(0, MODULE_ITEMS, setting, &dbv)) {
		strncpy(ds->szModuleName, dbv.pszVal, MODULE_NAME_LEN);
		ds->szModuleName[MODULE_NAME_LEN - 1] = 0;
		db_free(&dbv);
	}

	mir_snprintf(setting, "Setting%d", index);
	ds->szSettingName[0] = 0;
	if (!db_get_s(0, MODULE_ITEMS, setting, &dbv)) {
		strncpy(ds->szSettingName, dbv.pszVal, SETTING_NAME_LEN);
		ds->szSettingName[SETTING_NAME_LEN - 1] = 0;
		db_free(&dbv);
	}

	mir_snprintf(setting, "TransFuncId%d", index);
	ds->iTranslateFuncId = db_get_dw(0, MODULE_ITEMS, setting, (uint32_t)-1);

	// a little backward compatibility
	if ((uint32_t)ds->iTranslateFuncId == (uint32_t)-1) {
		mir_snprintf(setting, "TransFunc%d", index);
		ds->iTranslateFuncId = (uint32_t)db_get_w(0, MODULE_ITEMS, setting, 0);
	}

	return true;
}

void SaveDS(DISPLAYSUBST *ds, int index)
{
	char setting[512];

	mir_snprintf(setting, "Name%d", index);
	db_set_ws(0, MODULE_ITEMS, setting, ds->swzName);
	mir_snprintf(setting, "Type%d", index);
	db_set_b(0, MODULE_ITEMS, setting, (uint8_t)ds->type);
	mir_snprintf(setting, "Module%d", index);
	db_set_s(0, MODULE_ITEMS, setting, ds->szModuleName);
	mir_snprintf(setting, "Setting%d", index);
	db_set_s(0, MODULE_ITEMS, setting, ds->szSettingName);
	mir_snprintf(setting, "TransFuncId%d", index);
	db_set_dw(0, MODULE_ITEMS, setting, (uint16_t)ds->iTranslateFuncId);
}

bool LoadDI(DISPLAYITEM *di, int index)
{
	char setting[512];
	DBVARIANT dbv;

	mir_snprintf(setting, "DILabel%d", index);
	di->swzLabel[0] = 0;
	if (db_get_ws(0, MODULE_ITEMS, setting, &dbv))
		return false;

	wcsncpy(di->swzLabel, dbv.pwszVal, _countof(di->swzLabel));
	di->swzLabel[_countof(di->swzLabel) - 1] = 0;
	db_free(&dbv);

	mir_snprintf(setting, "DIValue%d", index);
	di->swzValue[0] = 0;
	if (!db_get_ws(0, MODULE_ITEMS, setting, &dbv)) {
		wcsncpy(di->swzValue, dbv.pwszVal, _countof(di->swzValue));
		di->swzValue[_countof(di->swzValue) - 1] = 0;
		db_free(&dbv);
	}

	mir_snprintf(setting, "DIType%d", index);
	di->type = (DisplayItemType)db_get_b(0, MODULE_ITEMS, setting, DIT_ALL);
	mir_snprintf(setting, "DILineAbove%d", index);
	di->bLineAbove = (db_get_b(0, MODULE_ITEMS, setting, 0) == 1);
	mir_snprintf(setting, "DIValNewline%d", index);
	di->bValueNewline = (db_get_b(0, MODULE_ITEMS, setting, 0) == 1);
	mir_snprintf(setting, "DIVisible%d", index);
	di->bIsVisible = (db_get_b(0, MODULE_ITEMS, setting, 1) == 1);
	mir_snprintf(setting, "DITipperVarsFirst%d", index);
	di->bParseTipperVarsFirst = (db_get_b(0, MODULE_ITEMS, setting, 0) == 1);

	return true;
}

void SaveDI(DISPLAYITEM *di, int index)
{
	char setting[512];

	mir_snprintf(setting, "DILabel%d", index);
	if (db_set_ws(0, MODULE_ITEMS, setting, di->swzLabel)) {
		char buff[LABEL_LEN];
		w2a(di->swzLabel, buff, LABEL_LEN);
		db_set_s(0, MODULE_ITEMS, setting, buff);
	}

	mir_snprintf(setting, "DIValue%d", index);
	if (db_set_ws(0, MODULE_ITEMS, setting, di->swzValue)) {
		char buff[VALUE_LEN];
		w2a(di->swzValue, buff, VALUE_LEN);
		db_set_s(0, MODULE_ITEMS, setting, buff);
	}

	mir_snprintf(setting, "DIType%d", index);
	db_set_b(0, MODULE_ITEMS, setting, (uint8_t)di->type);
	mir_snprintf(setting, "DILineAbove%d", index);
	db_set_b(0, MODULE_ITEMS, setting, di->bLineAbove ? 1 : 0);
	mir_snprintf(setting, "DIValNewline%d", index);
	db_set_b(0, MODULE_ITEMS, setting, di->bValueNewline ? 1 : 0);
	mir_snprintf(setting, "DIVisible%d", index);
	db_set_b(0, MODULE_ITEMS, setting, di->bIsVisible ? 1 : 0);
	mir_snprintf(setting, "DITipperVarsFirst%d", index);
	db_set_b(0, MODULE_ITEMS, setting, di->bParseTipperVarsFirst ? 1 : 0);
}

void SaveOptions()
{
	g_plugin.setDword("MaxWidth", opt.iWinWidth);
	g_plugin.setDword("MaxHeight", opt.iWinMaxHeight);
	g_plugin.setByte("AvatarOpacity", (uint8_t)opt.iAvatarOpacity);
	g_plugin.setByte("AvatarRoundCorners", (opt.bAvatarRound ? 1 : 0));
	g_plugin.setByte("TitleIconLayout", (uint8_t)opt.titleIconLayout);
	g_plugin.setByte("TitleShow", (opt.bShowTitle ? 1 : 0));
	if (ServiceExists(MS_AV_DRAWAVATAR))
		g_plugin.setByte("AVLayout", (uint8_t)opt.avatarLayout);
	opt.bWaitForAvatar = (opt.avatarLayout == PAV_NONE) ? false : true;

	g_plugin.setDword("AVSize", opt.iAvatarSize);
	g_plugin.setDword("TextIndent", opt.iTextIndent);
	g_plugin.setDword("TitleIndent", opt.iTitleIndent);
	g_plugin.setDword("ValueIndent", opt.iValueIndent);
	g_plugin.setByte("ShowNoFocus", (opt.bShowNoFocus ? 1 : 0));

	g_plugin.setWord("TimeIn", opt.iTimeIn);
	CallService(MS_CLC_SETINFOTIPHOVERTIME, opt.iTimeIn, 0);

	g_plugin.setWord("Padding", opt.iPadding);
	g_plugin.setWord("OuterAvatarPadding", opt.iOuterAvatarPadding);
	g_plugin.setWord("InnerAvatarPadding", opt.iInnerAvatarPadding);
	g_plugin.setWord("TextPadding", opt.iTextPadding);
	g_plugin.setByte("Position", (uint8_t)opt.pos);
	g_plugin.setDword("MinWidth", (uint32_t)opt.iMinWidth);
	g_plugin.setDword("MinHeight", (uint32_t)opt.iMinHeight);
	g_plugin.setDword("SidebarWidth", (uint32_t)opt.iSidebarWidth);
	g_plugin.setByte("MouseTollerance", (uint8_t)opt.iMouseTollerance);
	g_plugin.setByte("SBarTips", (opt.bStatusBarTips ? 1 : 0));

	g_plugin.setWord("LabelVAlign", opt.iLabelValign);
	g_plugin.setWord("LabelHAlign", opt.iLabelHalign);
	g_plugin.setWord("ValueVAlign", opt.iValueValign);
	g_plugin.setWord("ValueHAlign", opt.iValueHalign);

	g_plugin.setByte("OriginalAvSize", (opt.bOriginalAvatarSize ? 1 : 0));
	g_plugin.setByte("AvatarBorder", (opt.bAvatarBorder ? 1 : 0));
}

void SaveItems()
{
	int index = 0;
	DSListNode *ds_node = opt.dsList;
	while (ds_node) {
		SaveDS(&ds_node->ds, index);
		ds_node = ds_node->next;
		index++;
	}

	db_set_w(0, MODULE_ITEMS, "DSNumValues", index);

	index = 0;
	DIListNode *di_node = opt.diList;
	opt.bWaitForStatusMsg = false;
	while (di_node) {
		SaveDI(&di_node->di, index);
		if (di_node->di.bIsVisible && wcsstr(di_node->di.swzValue, L"sys:status_msg"))
			opt.bWaitForStatusMsg = true;
		di_node = di_node->next;
		index++;
	}

	db_set_w(0, MODULE_ITEMS, "DINumValues", index);
}

void SaveSkinOptions()
{
	g_plugin.setByte("Border", (opt.bBorder ? 1 : 0));
	g_plugin.setByte("DropShadow", (opt.bDropShadow ? 1 : 0));
	g_plugin.setByte("RoundCorners", (opt.bRound ? 1 : 0));
	g_plugin.setByte("AeroGlass", (opt.bAeroGlass ? 1 : 0));
	g_plugin.setByte("Opacity", (uint8_t)opt.iOpacity);
	g_plugin.setByte("ShowEffect", (uint8_t)opt.showEffect);
	g_plugin.setByte("ShowEffectSpeed", (uint8_t)opt.iAnimateSpeed);
	g_plugin.setByte("LoadFonts", (opt.bLoadFonts ? 1 : 0));
	g_plugin.setByte("LoadProportions", (opt.bLoadProportions ? 1 : 0));
	g_plugin.setDword("EnableColoring", opt.iEnableColoring);
}

void LoadObsoleteSkinSetting()
{
	char setting[128];
	DBVARIANT dbv;

	for (int i = 0; i < SKIN_ITEMS_COUNT; i++) {
		mir_snprintf(setting, "SPaintMode%d", i);
		opt.transfMode[i] = (TransformationMode)g_plugin.getByte(setting, 0);
		mir_snprintf(setting, "SImgFile%d", i);
		if (!g_plugin.getWString(setting, &dbv)) {
			opt.szImgFile[i] = mir_wstrdup(dbv.pwszVal);
			db_free(&dbv);
		}

		mir_snprintf(setting, "SGlyphMargins%d", i);
		uint32_t margins = g_plugin.getDword(setting, 0);
		opt.margins[i].top = LOBYTE(LOWORD(margins));
		opt.margins[i].right = HIBYTE(LOWORD(margins));
		opt.margins[i].bottom = LOBYTE(HIWORD(margins));
		opt.margins[i].left = HIBYTE(HIWORD(margins));
	}
}

void LoadOptions()
{
	opt.iWinWidth = g_plugin.getDword("MaxWidth", 420);
	opt.iWinMaxHeight = g_plugin.getDword("MaxHeight", 400);
	opt.iAvatarOpacity = g_plugin.getByte("AvatarOpacity", 100);
	if (opt.iAvatarOpacity > 100) opt.iAvatarOpacity = 100;
	opt.bAvatarRound = (g_plugin.getByte("AvatarRoundCorners", opt.bRound ? 1 : 0) == 1);
	opt.titleIconLayout = (PopupIconTitleLayout)g_plugin.getByte("TitleIconLayout", (uint8_t)PTL_LEFTICON);
	opt.bShowTitle = (g_plugin.getByte("TitleShow", 1) == 1);
	if (ServiceExists(MS_AV_DRAWAVATAR))
		opt.avatarLayout = (PopupAvLayout)g_plugin.getByte("AVLayout", PAV_RIGHT);
	else
		opt.avatarLayout = PAV_NONE;

	opt.bWaitForAvatar = (opt.avatarLayout == PAV_NONE) ? false : true;
	opt.iAvatarSize = g_plugin.getDword("AVSize", 60); //tweety
	opt.iTextIndent = g_plugin.getDword("TextIndent", 22);
	opt.iTitleIndent = g_plugin.getDword("TitleIndent", 22);
	opt.iValueIndent = g_plugin.getDword("ValueIndent", 10);
	opt.iSidebarWidth = g_plugin.getDword("SidebarWidth", 22);
	opt.bShowNoFocus = (g_plugin.getByte("ShowNoFocus", 1) == 1);

	int i, real_count = 0;
	opt.dsList = nullptr;
	DSListNode *ds_node;

	opt.iDsCount = db_get_w(0, MODULE_ITEMS, "DSNumValues", 0);
	for (i = opt.iDsCount - 1; i >= 0; i--) {
		ds_node = (DSListNode *)mir_calloc(sizeof(DSListNode));
		if (LoadDS(&ds_node->ds, i)) {
			ds_node->next = opt.dsList;
			opt.dsList = ds_node;
			real_count++;
		}
		else mir_free(ds_node);
	}
	opt.iDsCount = real_count;

	real_count = 0;
	opt.diList = nullptr;
	DIListNode *di_node;

	opt.bWaitForStatusMsg = false;
	opt.iDiCount = db_get_w(0, MODULE_ITEMS, "DINumValues", 0);
	for (i = opt.iDiCount - 1; i >= 0; i--) {
		di_node = (DIListNode *)mir_calloc(sizeof(DIListNode));
		if (LoadDI(&di_node->di, i)) {
			di_node->next = opt.diList;
			opt.diList = di_node;
			real_count++;
			if (di_node->di.bIsVisible && wcsstr(di_node->di.swzValue, L"sys:status_msg"))
				opt.bWaitForStatusMsg = true;
		}
		else mir_free(di_node);
	}
	opt.iDiCount = real_count;

	opt.iTimeIn = g_plugin.getWord("TimeIn", 750);
	opt.iPadding = g_plugin.getWord("Padding", 4);
	opt.iOuterAvatarPadding = g_plugin.getWord("OuterAvatarPadding", 6);
	opt.iInnerAvatarPadding = g_plugin.getWord("InnerAvatarPadding", 10);
	opt.iTextPadding = g_plugin.getWord("TextPadding", 4);
	opt.pos = (PopupPosition)g_plugin.getByte("Position", (uint8_t)PP_BOTTOMRIGHT);
	opt.iMinWidth = g_plugin.getDword("MinWidth", 0);
	opt.iMinHeight = g_plugin.getDword("MinHeight", 0);

	opt.iMouseTollerance = g_plugin.getByte("MouseTollerance", (uint8_t)GetSystemMetrics(SM_CXSMICON));
	opt.bStatusBarTips = (g_plugin.getByte("SBarTips", 1) == 1);

	// convert defunct last message and status message options to new 'sys' items, and remove the old settings
	if (g_plugin.getByte("ShowLastMessage", 0)) {
		g_plugin.delSetting("ShowLastMessage");

		// find end of list
		di_node = opt.diList;
		while (di_node && di_node->next)
			di_node = di_node->next;

		// last message item
		if (di_node) {
			di_node->next = (DIListNode *)mir_calloc(sizeof(DIListNode));
			di_node = di_node->next;
		}
		else {
			opt.diList = (DIListNode *)mir_calloc(sizeof(DIListNode));
			di_node = opt.diList;
		}

		wcsncpy(di_node->di.swzLabel, L"Last message: (%sys:last_msg_reltime% ago)", LABEL_LEN);
		wcsncpy(di_node->di.swzValue, L"%sys:last_msg%", VALUE_LEN);
		di_node->di.bLineAbove = di_node->di.bValueNewline = true;
		di_node->next = nullptr;
		opt.iDiCount++;
	}

	if (g_plugin.getByte("ShowStatusMessage", 0)) {
		g_plugin.delSetting("ShowStatusMessage");

		// find end of list
		di_node = opt.diList;
		while (di_node && di_node->next)
			di_node = di_node->next;

		// status message item
		if (di_node) {
			di_node->next = (DIListNode *)mir_calloc(sizeof(DIListNode));
			di_node = di_node->next;
		}
		else {
			opt.diList = (DIListNode *)mir_calloc(sizeof(DIListNode));
			di_node = opt.diList;
		}

		wcsncpy(di_node->di.swzLabel, L"Status message:", LABEL_LEN);
		wcsncpy(di_node->di.swzValue, L"%sys:status_msg%", VALUE_LEN);
		di_node->di.bLineAbove = di_node->di.bValueNewline = true;
		di_node->next = nullptr;
		opt.iDiCount++;
	}

	opt.iLabelValign = g_plugin.getWord("LabelVAlign", DT_TOP /*DT_VCENTER*/);
	opt.iLabelHalign = g_plugin.getWord("LabelHAlign", DT_LEFT);
	opt.iValueValign = g_plugin.getWord("ValueVAlign", DT_TOP /*DT_VCENTER*/);
	opt.iValueHalign = g_plugin.getWord("ValueHAlign", DT_LEFT);

	// tray tooltip
	opt.bTraytip = g_plugin.getByte("TrayTip", 1) ? true : false;
	opt.bHandleByTipper = g_plugin.getByte("ExtendedTrayTip", 1) ? true : false;
	opt.bExpandTraytip = g_plugin.getByte("ExpandTrayTip", 1) ? true : false;
	opt.bHideOffline = g_plugin.getByte("HideOffline", 0) ? true : false;
	opt.iExpandTime = g_plugin.getDword("ExpandTime", 1000);
	opt.iFirstItems = g_plugin.getDword("TrayTipItems", TRAYTIP_NUMCONTACTS | TRAYTIP_LOGON | TRAYTIP_STATUS | TRAYTIP_CLIST_EVENT);
	opt.iSecondItems = g_plugin.getDword("TrayTipItemsEx", TRAYTIP_NUMCONTACTS | TRAYTIP_LOGON | TRAYTIP_STATUS | TRAYTIP_STATUS_MSG | TRAYTIP_EXTRA_STATUS | TRAYTIP_MIRANDA_UPTIME | TRAYTIP_CLIST_EVENT);
	opt.iFavoriteContFlags = g_plugin.getDword("FavContFlags", FAVCONT_APPEND_PROTO);

	// extra setting
	opt.bWaitForContent = g_plugin.getByte("WaitForContent", 0) ? true : false;
	opt.bGetNewStatusMsg = g_plugin.getByte("GetNewStatusMsg", 0) ? true : false;
	opt.bRetrieveXstatus = g_plugin.getByte("RetrieveXStatus", 0) ? true : false;
	opt.bOriginalAvatarSize = g_plugin.getByte("OriginalAvSize", 0) ? true : false;
	opt.bAvatarBorder = g_plugin.getByte("AvatarBorder", 0) ? true : false;
	opt.bLimitMsg = g_plugin.getByte("LimitMsg", 0) ? true : false;
	opt.iLimitCharCount = g_plugin.getByte("LimitCharCount", 64);
	opt.iSmileyAddFlags = g_plugin.getDword("SmileyAddFlags", SMILEYADD_ENABLE);

	DBVARIANT dbv;
	// Load the icons order
	for (i = 0; i < EXICONS_COUNT; i++) {
		opt.exIconsOrder[i] = i;
		opt.exIconsVis[i] = 1;
	}

	if (!db_get(0, MODULENAME, "IconOrder", &dbv)) {
		memcpy(opt.exIconsOrder, dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
	}

	if (!db_get(0, MODULENAME, "icons_vis", &dbv)) {
		memcpy(opt.exIconsVis, dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
	}

	for (i = 0; i < EXICONS_COUNT; i++) {
		exIcons[i].order = opt.exIconsOrder[i];
		exIcons[i].vis = opt.exIconsVis[i];
	}

	opt.iOpacity = g_plugin.getByte("Opacity", 75);
	opt.bBorder = g_plugin.getByte("Border", 1) ? true : false;
	opt.bDropShadow = g_plugin.getByte("DropShadow", 1) ? true : false;
	opt.bRound = g_plugin.getByte("RoundCorners", 1) ? true : false;
	opt.bAeroGlass = g_plugin.getByte("AeroGlass", 0) ? true : false;
	opt.showEffect = (PopupShowEffect)g_plugin.getByte("ShowEffect", (uint8_t)PSE_FADE);
	opt.iAnimateSpeed = g_plugin.getByte("ShowEffectSpeed", 12);

	if (opt.iAnimateSpeed < 1)
		opt.iAnimateSpeed = 1;
	else if (opt.iAnimateSpeed > 20)
		opt.iAnimateSpeed = 20;

	int iBgImg = g_plugin.getByte("SBgImage", 0);
	opt.skinMode = (SkinMode)g_plugin.getByte("SkinEngine", iBgImg ? SM_OBSOLOTE : SM_COLORFILL);
	opt.bLoadFonts = g_plugin.getByte("LoadFonts", 1) ? true : false;
	opt.bLoadProportions = g_plugin.getByte("LoadProportions", 1) ? true : false;
	opt.iEnableColoring = g_plugin.getDword("EnableColoring", 0);
	opt.szSkinName[0] = 0;

	if (opt.skinMode == SM_OBSOLOTE) {
		LoadObsoleteSkinSetting();
	}
	else if (opt.skinMode == SM_IMAGE) {
		if (!g_plugin.getWString("SkinName", &dbv)) {
			wcsncpy(opt.szSkinName, dbv.pwszVal, _countof(opt.szSkinName) - 1);
			db_free(&dbv);
		}
	}

	if (opt.iDsCount == 0 && opt.iDiCount == 0) {
		// set up some reasonable defaults
		CreateDefaultItems();
		SaveOptions();
		SaveItems();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Item editor dialog

struct
{
	DisplayItemType type;
	wchar_t *title;
}
static displayItemTypes[] = {
	{ DIT_ALL,      LPGENW("Show for all contact types") },
	{ DIT_CONTACTS, LPGENW("Show only for contacts") },
	{ DIT_CHATS,    LPGENW("Show only for chatrooms") },
};

INT_PTR CALLBACK DlgProcAddItem(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DISPLAYITEM *di = (DISPLAYITEM *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		di = (DISPLAYITEM *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)di);

		SetDlgItemText(hwndDlg, IDC_ED_LABEL, di->swzLabel);
		SetDlgItemText(hwndDlg, IDC_ED_VALUE, di->swzValue);

		for (auto &it : displayItemTypes) {
			int index = SendDlgItemMessage(hwndDlg, IDC_CMB_TYPE, CB_ADDSTRING, (WPARAM)-1, (LPARAM)TranslateW(it.title));
			SendDlgItemMessage(hwndDlg, IDC_CMB_TYPE, CB_SETITEMDATA, index, (LPARAM)it.type);
			if (it.type == di->type)
				SendDlgItemMessage(hwndDlg, IDC_CMB_TYPE, CB_SETCURSEL, index, 0);
		}

		CheckDlgButton(hwndDlg, IDC_CHK_LINEABOVE, di->bLineAbove ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_VALNEWLINE, di->bValueNewline ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_PARSETIPPERFIRST, di->bParseTipperVarsFirst ? BST_CHECKED : BST_UNCHECKED);

		for (auto &it : presetItems)
			SendDlgItemMessage(hwndDlg, IDC_CMB_PRESETITEMS, CB_ADDSTRING, 0, (LPARAM)TranslateW(it.swzName));

		variables_skin_helpbutton(hwndDlg, IDC_BTN_VARIABLE);

		SetFocus(GetDlgItem(hwndDlg, IDC_ED_LABEL));
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
			case IDOK:
				GetDlgItemText(hwndDlg, IDC_ED_LABEL, di->swzLabel, LABEL_LEN);
				GetDlgItemText(hwndDlg, IDC_ED_VALUE, di->swzValue, VALUE_LEN);
				{
					int sel = SendDlgItemMessage(hwndDlg, IDC_CMB_TYPE, CB_GETCURSEL, 0, 0);
					int type = SendDlgItemMessage(hwndDlg, IDC_CMB_TYPE, CB_GETITEMDATA, sel, 0);
					for (auto &it : displayItemTypes)
						if (it.type == type)
							di->type = it.type;

					di->bLineAbove = (IsDlgButtonChecked(hwndDlg, IDC_CHK_LINEABOVE) ? true : false);
					di->bValueNewline = (IsDlgButtonChecked(hwndDlg, IDC_CHK_VALNEWLINE) ? true : false);
					di->bParseTipperVarsFirst = (IsDlgButtonChecked(hwndDlg, IDC_CHK_PARSETIPPERFIRST) ? true : false);

					sel = SendDlgItemMessage(hwndDlg, IDC_CMB_PRESETITEMS, CB_GETCURSEL, 0, 0);
					if (sel != CB_ERR) {
						wchar_t buff[256];
						SendDlgItemMessage(hwndDlg, IDC_CMB_PRESETITEMS, CB_GETLBTEXT, sel, (LPARAM)buff);
						for (auto &it : presetItems)
							if (mir_wstrcmp(buff, TranslateW(it.swzName)) == 0) {
								if (it.szNeededSubst[0])
									EndDialog(hwndDlg, IDPRESETITEM + int(&it - presetItems));
								else
									EndDialog(hwndDlg, IDOK);
								break;
							}
					}
					else EndDialog(hwndDlg, IDOK);
				}
				return TRUE;

			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				return TRUE;

			case IDC_BTN_VARIABLE:
				if (GetFocus() == GetDlgItem(hwndDlg, IDC_ED_LABEL))
					variables_showhelp(hwndDlg, IDC_ED_LABEL, VHF_FULLDLG, nullptr, nullptr);
				else
					variables_showhelp(hwndDlg, IDC_ED_VALUE, VHF_FULLDLG, nullptr, nullptr);
				return TRUE;
			}
		}
		else if (HIWORD(wParam) == CBN_SELCHANGE) {
			if (LOWORD(wParam) == IDC_CMB_PRESETITEMS) {
				int sel = SendDlgItemMessage(hwndDlg, IDC_CMB_PRESETITEMS, CB_GETCURSEL, 0, 0);
				if (sel != CB_ERR) {
					wchar_t buff[256];
					SendDlgItemMessage(hwndDlg, IDC_CMB_PRESETITEMS, CB_GETLBTEXT, sel, (LPARAM)buff);
					for (auto &it : presetItems)
						if (mir_wstrcmp(buff, TranslateW(it.swzName)) == 0) {
							SetDlgItemText(hwndDlg, IDC_ED_LABEL, TranslateW(it.swzLabel));
							SetDlgItemText(hwndDlg, IDC_ED_VALUE, it.swzValue);
							break;
						}
				}
			}
		}
		break;
	}

	return 0;
}

INT_PTR CALLBACK DlgProcAddSubst(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DISPLAYSUBST *ds = (DISPLAYSUBST *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		ds = (DISPLAYSUBST *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)ds);

		SetDlgItemText(hwndDlg, IDC_ED_LABEL, ds->swzName);

		switch (ds->type) {
		case DVT_PROTODB:
			CheckDlgButton(hwndDlg, IDC_CHK_PROTOMOD, BST_CHECKED);
			SetDlgItemTextA(hwndDlg, IDC_ED_SETTING, ds->szSettingName);
			break;
		case DVT_DB:
			SetDlgItemTextA(hwndDlg, IDC_ED_MODULE, ds->szModuleName);
			SetDlgItemTextA(hwndDlg, IDC_ED_SETTING, ds->szSettingName);
			break;
		}

		for (int i = 0; i < iTransFuncsCount; i++) {
			int index = SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_ADDSTRING, (WPARAM)-1, (LPARAM)TranslateW(translations[i].swzName));
			SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_SETITEMDATA, index, (LPARAM)translations[i].id);
		}

		for (int i = 0; i < iTransFuncsCount; i++) {
			int id = SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_GETITEMDATA, i, 0);
			if (id == ds->iTranslateFuncId)
				SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_SETCURSEL, i, 0);
		}

		SendMessage(hwndDlg, WMU_ENABLE_MODULE_ENTRY, 0, 0);
		SetFocus(GetDlgItem(hwndDlg, IDC_ED_LABEL));
		return TRUE;

	case WMU_ENABLE_MODULE_ENTRY:
		{
			HWND hw = GetDlgItem(hwndDlg, IDC_CHK_PROTOMOD);
			EnableWindow(hw, TRUE);
			hw = GetDlgItem(hwndDlg, IDC_ED_MODULE);
			EnableWindow(hw, BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_PROTOMOD));
			hw = GetDlgItem(hwndDlg, IDC_ED_SETTING);
			EnableWindow(hw, TRUE);
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
			case IDC_CHK_PROTOMOD:
				SendMessage(hwndDlg, WMU_ENABLE_MODULE_ENTRY, 0, 0);
				break;

			case IDOK:
				GetDlgItemText(hwndDlg, IDC_ED_LABEL, ds->swzName, LABEL_LEN);
				if (ds->swzName[0] == 0) {
					MessageBox(hwndDlg, TranslateT("You must enter a label"), TranslateT("Invalid Substitution"), MB_OK | MB_ICONWARNING);
					return TRUE;
				}

				if (IsDlgButtonChecked(hwndDlg, IDC_CHK_PROTOMOD))
					ds->type = DVT_PROTODB;
				else {
					ds->type = DVT_DB;
					GetDlgItemTextA(hwndDlg, IDC_ED_MODULE, ds->szModuleName, _countof(ds->szModuleName));
				}

				GetDlgItemTextA(hwndDlg, IDC_ED_SETTING, ds->szSettingName, _countof(ds->szSettingName));

				ds->iTranslateFuncId = SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_GETCURSEL, 0, 0), 0);

				EndDialog(hwndDlg, IDOK);
				return TRUE;

			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				return TRUE;
			}
		}
		else if (HIWORD(wParam) == CBN_SELCHANGE)
			return TRUE;

		break;
	}

	return 0;
}

static void SetTreeItemText(DIListNode *node, wchar_t **pszText)
{
	if (node->di.swzLabel[0] == 0) {
		if (node->di.swzValue[0] == 0 && node->di.bLineAbove)
			*pszText = L"--------------------------------------";
		else
			*pszText = TranslateT("<No Label>");
	}
	else
		*pszText = node->di.swzLabel;
}

INT_PTR CALLBACK DlgProcOptsContent(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SendDlgItemMessage(hwndDlg, IDC_BTN_ADD, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_ADD, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Add item"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_BTN_ADD, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadIcon(SKINICON_OTHER_ADDCONTACT));

		SendDlgItemMessage(hwndDlg, IDC_BTN_SEPARATOR, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_SEPARATOR, BUTTONADDTOOLTIP, (WPARAM)Translate(iconList[4].szDescr), 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_SEPARATOR, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(iconList[4].hIcolib));

		SendDlgItemMessage(hwndDlg, IDC_BTN_EDIT, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_EDIT, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Edit"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_BTN_EDIT, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadIcon(SKINICON_OTHER_RENAME));

		SendDlgItemMessage(hwndDlg, IDC_BTN_REMOVE, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_REMOVE, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Remove"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_BTN_REMOVE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadIcon(SKINICON_OTHER_DELETE));

		SendDlgItemMessage(hwndDlg, IDC_BTN_UP, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_UP, BUTTONADDTOOLTIP, (WPARAM)Translate(iconList[2].szDescr), 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_UP, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(iconList[2].hIcolib));

		SendDlgItemMessage(hwndDlg, IDC_BTN_DOWN, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_DOWN, BUTTONADDTOOLTIP, (WPARAM)Translate(iconList[3].szDescr), 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_DOWN, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(iconList[3].hIcolib));

		SendDlgItemMessage(hwndDlg, IDC_BTN_ADD2, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_ADD2, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Add"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_BTN_ADD2, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadIcon(SKINICON_OTHER_ADDCONTACT));

		SendDlgItemMessage(hwndDlg, IDC_BTN_REMOVE2, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_REMOVE2, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Remove"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_BTN_REMOVE2, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadIcon(SKINICON_OTHER_DELETE));

		SendDlgItemMessage(hwndDlg, IDC_BTN_EDIT2, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_EDIT2, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Edit"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_BTN_EDIT2, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadIcon(SKINICON_OTHER_RENAME));

		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), GWL_STYLE) | TVS_CHECKBOXES);
		{
			TVINSERTSTRUCT tvi = {};
			tvi.hInsertAfter = TVI_LAST;
			tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
			tvi.item.stateMask = TVIS_STATEIMAGEMASK;

			DIListNode *di_node = opt.diList, *di_value;
			while (di_node) {
				di_value = (DIListNode *)mir_calloc(sizeof(DIListNode));
				*di_value = *di_node;
				tvi.item.lParam = (LPARAM)di_value;
				tvi.item.state = INDEXTOSTATEIMAGEMASK(di_value->di.bIsVisible ? 2 : 1);
				SetTreeItemText(di_value, &tvi.item.pszText);
				TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &tvi);
				di_node = di_node->next;
			}

			DSListNode *ds_node = opt.dsList, *ds_value;
			while (ds_node) {
				ds_value = (DSListNode *)mir_calloc(sizeof(DSListNode));
				*ds_value = *ds_node;
				int index = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)ds_value->ds.swzName);
				SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, index, (LPARAM)ds_value);
				ds_node = ds_node->next;
			}

			SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
			return FALSE;
		}

	case WMU_ENABLE_LIST_BUTTONS:
		{
			HTREEITEM hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
			if (hItem) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_REMOVE), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UP), TreeView_GetPrevSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hItem) ? TRUE : FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DOWN), TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hItem) ? TRUE : FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_EDIT), TRUE);
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_REMOVE), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UP), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DOWN), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_EDIT), FALSE);
			}

			int sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCURSEL, 0, 0);
			if (sel == -1) {
				HWND hw = GetDlgItem(hwndDlg, IDC_BTN_REMOVE2);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_EDIT2);
				EnableWindow(hw, FALSE);
			}
			else {
				HWND hw = GetDlgItem(hwndDlg, IDC_BTN_REMOVE2);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_EDIT2);
				EnableWindow(hw, TRUE);
			}
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == LBN_SELCHANGE && LOWORD(wParam) == IDC_LST_SUBST)
			SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
		else if (HIWORD(wParam) == CBN_SELCHANGE)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		else if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		else if (HIWORD(wParam) == LBN_DBLCLK && LOWORD(wParam) == IDC_LST_SUBST) {
			int sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCURSEL, 0, 0);
			if (sel != CB_ERR) {
				DSListNode *ds_value = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, sel, 0);
				if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SUBST), hwndDlg, DlgProcAddSubst, (LPARAM)&ds_value->ds) == IDOK) {
					SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_DELETESTRING, (WPARAM)sel, 0);

					sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)ds_value->ds.swzName);
					SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, sel, (LPARAM)ds_value);

					SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETCURSEL, sel, 0);
					SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}
		}
		else if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
			case IDC_BTN_ADD:
				{
					DIListNode *di_value = (DIListNode *)mir_calloc(sizeof(DIListNode));
					memset(di_value, 0, sizeof(DIListNode));
					di_value->di.bIsVisible = true;

					int result = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ITEM), hwndDlg, DlgProcAddItem, (LPARAM)&di_value->di);
					if (result == IDOK || (result >= IDPRESETITEM && result < (IDPRESETITEM + 100))) {
						TVINSERTSTRUCT tvi = {};
						tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
						tvi.item.stateMask = TVIS_STATEIMAGEMASK;
						tvi.item.lParam = (LPARAM)di_value;
						tvi.item.state = INDEXTOSTATEIMAGEMASK(2);
						SetTreeItemText(di_value, &tvi.item.pszText);

						HTREEITEM hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
						if (hItem)
							tvi.hInsertAfter = hItem;
						else
							tvi.hInsertAfter = TVI_LAST;
						TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &tvi);

						if (hItem) {
							HTREEITEM hNewItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hItem);
							if (hNewItem)
								TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hNewItem);
						}

						if (result >= IDPRESETITEM) {
							for (int i = 0; i < MAX_PRESET_SUBST_COUNT; i++) {

								PRESETSUBST *subst = GetPresetSubstByName(presetItems[result - IDPRESETITEM].szNeededSubst[i]);
								if (subst == nullptr)
									break;
								if (SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_FINDSTRING, -1, (LPARAM)subst->swzName) == LB_ERR) {
									DSListNode *ds_value = (DSListNode *)mir_calloc(sizeof(DSListNode));
									memset(ds_value, 0, sizeof(DSListNode));
									ds_value->next = nullptr;
									ds_value->ds.type = subst->szModuleName == nullptr ? DVT_PROTODB : DVT_DB;
									wcsncpy(ds_value->ds.swzName, subst->swzName, LABEL_LEN - 1);

									if (ds_value->ds.type == DVT_DB && subst->szModuleName)
										strncpy(ds_value->ds.szModuleName, subst->szModuleName, MODULE_NAME_LEN - 1);

									if (subst->szSettingName)
										strncpy(ds_value->ds.szSettingName, subst->szSettingName, SETTING_NAME_LEN - 1);

									ds_value->ds.iTranslateFuncId = subst->iTranslateFuncId;

									int index = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)ds_value->ds.swzName);
									SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, index, (LPARAM)ds_value);
									SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETCURSEL, index, 0);
								}
							}
						}

						SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
					else
						mir_free(di_value);

					return TRUE;
				}
			case IDC_BTN_SEPARATOR:
				{
					DIListNode *di_value = (DIListNode *)mir_calloc(sizeof(DIListNode));
					memset(di_value, 0, sizeof(DIListNode));
					di_value->di.bIsVisible = true;
					di_value->di.bLineAbove = true;

					TVINSERTSTRUCT tvi = {};
					tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
					tvi.item.stateMask = TVIS_STATEIMAGEMASK;
					tvi.item.lParam = (LPARAM)di_value;
					tvi.item.state = INDEXTOSTATEIMAGEMASK(2);
					tvi.item.pszText = L"---------------------------------";

					HTREEITEM hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
					if (hItem)
						tvi.hInsertAfter = hItem;
					else
						tvi.hInsertAfter = TVI_LAST;
					TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &tvi);

					SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					return TRUE;
				}
			case IDC_BTN_REMOVE:
				{
					TVITEM item = { 0 };
					item.mask = TVIF_PARAM;
					item.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
					if (item.hItem) {
						if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item)) {
							DIListNode *di_value = (DIListNode *)item.lParam;
							mir_free(di_value);
							TreeView_DeleteItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), item.hItem);
							SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}
					}
					return TRUE;
				}
			case IDC_BTN_UP:
			case IDC_BTN_DOWN:
				{
					HTREEITEM hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
					if (hItem) {
						HTREEITEM hNewItem;
						if (LOWORD(wParam) == IDC_BTN_UP)
							hNewItem = TreeView_GetPrevSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hItem);
						else
							hNewItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hItem);

						if (hNewItem) {
							wchar_t buff[512], buff2[512];
							LPARAM tmpParam;
							UINT tmpState;

							TVITEM item = { 0 };
							item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
							item.stateMask = TVIS_STATEIMAGEMASK;
							item.hItem = hItem;
							item.pszText = buff;
							item.cchTextMax = _countof(buff);
							if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item)) {
								tmpParam = item.lParam;
								tmpState = item.state;
								item.hItem = hNewItem;
								item.pszText = buff2;
								if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item)) {
									item.hItem = hItem;
									TreeView_SetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item);

									item.hItem = hNewItem;
									item.pszText = buff;
									item.lParam = tmpParam;
									item.state = tmpState;
									TreeView_SetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item);
									TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hNewItem);
								}
							}
						}
					}

					SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					return TRUE;
				}
			case IDC_BTN_EDIT:
				{
					TVITEM item = { 0 };
					item.mask = TVIF_PARAM;
					item.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
					if (item.hItem) {
						if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item)) {
							DIListNode *di_value = (DIListNode *)item.lParam;
							if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ITEM), hwndDlg, DlgProcAddItem, (LPARAM)&di_value->di) == IDOK) {
								item.mask = TVIF_TEXT;
								SetTreeItemText(di_value, &item.pszText);
								TreeView_SetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item);
								SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}
						}
					}

					return TRUE;
				}
			case IDC_BTN_ADD2:
				{
					DSListNode *ds_value = (DSListNode *)mir_calloc(sizeof(DSListNode));
					memset(ds_value, 0, sizeof(DSListNode));
					if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SUBST), hwndDlg, DlgProcAddSubst, (LPARAM)&ds_value->ds) == IDOK) {
						int index = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)ds_value->ds.swzName);
						SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, index, (LPARAM)ds_value);
						SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETCURSEL, index, 0);
						SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
					else
						mir_free(ds_value);

					return TRUE;
				}
			case IDC_BTN_REMOVE2:
				{
					int sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCURSEL, 0, 0);
					if (sel != LB_ERR) {
						DSListNode *ds_value = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, sel, 0);
						mir_free(ds_value);

						SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_DELETESTRING, (WPARAM)sel, 0);
						SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}

					return TRUE;
				}
			case IDC_BTN_EDIT2:
				{
					int sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCURSEL, 0, 0);
					if (sel != LB_ERR) {
						DSListNode *ds_value = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, sel, 0);
						if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SUBST), hwndDlg, DlgProcAddSubst, (LPARAM)&ds_value->ds) == IDOK) {
							SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_DELETESTRING, (WPARAM)sel, 0);

							sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)ds_value->ds.swzName);
							SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, sel, (LPARAM)ds_value);

							SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETCURSEL, sel, 0);
							SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}
					}

					return TRUE;
				}
			default:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				return TRUE;
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			{
				DIListNode *di_node;
				while (opt.diList) {
					di_node = opt.diList;
					opt.diList = opt.diList->next;
					mir_free(di_node);
				}

				DIListNode *di_value;
				opt.iDiCount = TreeView_GetCount(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));

				TVITEM item = { 0 };
				item.mask = TVIF_PARAM;
				item.hItem = TreeView_GetLastVisible(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
				while (item.hItem != nullptr) {
					if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item)) {
						di_node = (DIListNode *)item.lParam;
						di_value = (DIListNode *)mir_calloc(sizeof(DIListNode));
						*di_value = *di_node;
						di_value->next = opt.diList;
						opt.diList = di_value;
					}
					item.hItem = TreeView_GetPrevSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), item.hItem);
				}

				DSListNode *ds_node;
				while (opt.dsList) {
					ds_node = opt.dsList;
					opt.dsList = opt.dsList->next;
					mir_free(ds_node);
				}

				DSListNode *ds_value;
				opt.iDsCount = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCOUNT, 0, 0);
				for (int i = opt.iDsCount - 1; i >= 0; i--) {
					ds_node = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, i, 0);
					ds_value = (DSListNode *)mir_calloc(sizeof(DSListNode));
					*ds_value = *ds_node;
					ds_value->next = opt.dsList;
					opt.dsList = ds_value;
				}

				SaveItems();
				return TRUE;
			}
		case NM_DBLCLK:
			{
				TVITEM item = { 0 };
				item.mask = TVIF_PARAM;
				item.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
				if (item.hItem) {
					if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item)) {
						DIListNode *di_value = (DIListNode *)item.lParam;
						if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ITEM), hwndDlg, DlgProcAddItem, (LPARAM)&di_value->di) == IDOK) {
							item.mask = TVIF_TEXT;
							SetTreeItemText(di_value, &item.pszText);
							TreeView_SetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item);
							SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}
					}
				}
				break;
			}

		case NM_CLICK:
			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(GetMessagePos());
			hti.pt.y = (short)HIWORD(GetMessagePos());
			ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
			if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti)) {
				if (hti.flags & TVHT_ONITEMSTATEICON) {
					TVITEMA item = { 0 };
					item.hItem = hti.hItem;
					item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
					item.stateMask = TVIS_STATEIMAGEMASK;
					TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &item);

					if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 1)
						((DIListNode *)item.lParam)->di.bIsVisible = true;
					else
						((DIListNode *)item.lParam)->di.bIsVisible = false;

					TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &item);
					SendMessage((GetParent(hwndDlg)), PSM_CHANGED, (WPARAM)hwndDlg, 0);
				}
			}
			break;

		case TVN_SELCHANGED:
			SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
			break;
		}
		break;

	case WM_DESTROY:
		TVITEM tvi = { 0 };
		tvi.mask = TVIF_PARAM;

		for (HTREEITEM hItem = TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS)); hItem != nullptr;
			hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hItem)) {
			tvi.hItem = hItem;
			if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &tvi)) {
				DIListNode *di_value = (DIListNode *)tvi.lParam;
				mir_free(di_value);
			}
		}

		int count = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCOUNT, 0, 0);
		for (int i = 0; i < count; i++) {
			DSListNode *ds_value = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, i, 0);
			mir_free(ds_value);
		}

		break;
	}

	return 0;
}

INT_PTR CALLBACK DlgProcOptsAppearance(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_CHK_SHOWTITLE, opt.bShowTitle ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_NOFOCUS, opt.bShowNoFocus ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_SBAR, opt.bStatusBarTips ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("No icon"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("Icon on left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("Icon on right"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_SETCURSEL, (int)opt.titleIconLayout, 0);

		SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom right"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top right"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_SETCURSEL, (int)opt.pos, 0);

		SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Center"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom"));
		switch (opt.iLabelValign) {
		case DT_TOP: SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_SETCURSEL, 0, 0); break;
		case DT_VCENTER: SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_SETCURSEL, 1, 0); break;
		case DT_BOTTOM: SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_SETCURSEL, 2, 0); break;
		}

		SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Center"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom"));
		switch (opt.iValueValign) {
		case DT_TOP: SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_SETCURSEL, 0, 0); break;
		case DT_VCENTER: SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_SETCURSEL, 1, 0); break;
		case DT_BOTTOM: SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_SETCURSEL, 2, 0); break;
		}

		SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right"));
		switch (opt.iLabelHalign) {
		case DT_LEFT: SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_SETCURSEL, 0, 0); break;
		case DT_RIGHT: SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_SETCURSEL, 1, 0); break;
		}

		SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right"));
		switch (opt.iValueHalign) {
		case DT_LEFT: SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_SETCURSEL, 0, 0); break;
		case DT_RIGHT: SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_SETCURSEL, 1, 0); break;
		}

		SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("No avatar"));
		if (ServiceExists(MS_AV_DRAWAVATAR)) {
			SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left avatar"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right avatar"));
		}
		else {
			HWND hw = GetDlgItem(hwndDlg, IDC_CMB_AV);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_SPIN_AVSIZE);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_ED_AVSIZE);
			EnableWindow(hw, FALSE);
		}
		SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_SETCURSEL, (int)opt.avatarLayout, 0);

		SendDlgItemMessage(hwndDlg, IDC_SPIN_WIDTH, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_MINWIDTH, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_MAXHEIGHT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_MINHEIGHT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_AVSIZE, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 16));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_INDENT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(400, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_TITLEINDENT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(400, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_VALUEINDENT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(400, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_PADDING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(128, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_TEXTPADDING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(128, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_OUTAVPADDING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(128, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_INAVPADDING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(128, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_HOVER, UDM_SETRANGE, 0, (LPARAM)MAKELONG(5000, 5));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_SBWIDTH, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 0));

		SetDlgItemInt(hwndDlg, IDC_ED_WIDTH, opt.iWinWidth, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_MAXHEIGHT, opt.iWinMaxHeight, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_MINWIDTH, opt.iMinWidth, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_MINHEIGHT, opt.iMinHeight, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_AVSIZE, opt.iAvatarSize, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_INDENT, opt.iTextIndent, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_TITLEINDENT, opt.iTitleIndent, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_VALUEINDENT, opt.iValueIndent, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_PADDING, opt.iPadding, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_TEXTPADDING, opt.iTextPadding, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_OUTAVPADDING, opt.iOuterAvatarPadding, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_INAVPADDING, opt.iInnerAvatarPadding, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_HOVER, opt.iTimeIn, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_SBWIDTH, opt.iSidebarWidth, FALSE);

		CheckDlgButton(hwndDlg, IDC_CHK_ROUNDCORNERSAV, opt.bAvatarRound ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_AVBORDER, opt.bAvatarBorder ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_ORIGINALAVSIZE, opt.bOriginalAvatarSize ? BST_CHECKED : BST_UNCHECKED);

		if (opt.bOriginalAvatarSize)
			SetDlgItemText(hwndDlg, IDC_STATIC_AVATARSIZE, TranslateT("Max avatar size:"));
		return FALSE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_CHK_ORIGINALAVSIZE)
			SetDlgItemText(hwndDlg, IDC_STATIC_AVATARSIZE, IsDlgButtonChecked(hwndDlg, IDC_CHK_ORIGINALAVSIZE) ? TranslateT("Max avatar size:") : TranslateT("Avatar size:"));

		if (HIWORD(wParam) == CBN_SELCHANGE)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		else if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		else if (HIWORD(wParam) == BN_CLICKED)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY) {
			BOOL trans;
			int new_val = GetDlgItemInt(hwndDlg, IDC_ED_WIDTH, &trans, FALSE);
			if (trans) opt.iWinWidth = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_MINWIDTH, &trans, FALSE);
			if (trans) opt.iMinWidth = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_MAXHEIGHT, &trans, FALSE);
			if (trans) opt.iWinMaxHeight = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_MINHEIGHT, &trans, FALSE);
			if (trans) opt.iMinHeight = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_AVSIZE, &trans, FALSE);
			if (trans) opt.iAvatarSize = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_INDENT, &trans, FALSE);
			if (trans) opt.iTextIndent = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_TITLEINDENT, &trans, FALSE);
			if (trans) opt.iTitleIndent = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_VALUEINDENT, &trans, FALSE);
			if (trans) opt.iValueIndent = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_PADDING, &trans, FALSE);
			if (trans) opt.iPadding = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_TEXTPADDING, &trans, FALSE);
			if (trans) opt.iTextPadding = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_OUTAVPADDING, &trans, FALSE);
			if (trans) opt.iOuterAvatarPadding = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_INAVPADDING, &trans, FALSE);
			if (trans) opt.iInnerAvatarPadding = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_HOVER, &trans, FALSE);
			if (trans) opt.iTimeIn = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_SBWIDTH, &trans, FALSE);
			if (trans) opt.iSidebarWidth = new_val;

			opt.titleIconLayout = (PopupIconTitleLayout)SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_GETCURSEL, 0, 0);
			opt.avatarLayout = (PopupAvLayout)SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_GETCURSEL, 0, 0);
			opt.pos = (PopupPosition)SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_GETCURSEL, 0, 0);

			opt.bAvatarBorder = IsDlgButtonChecked(hwndDlg, IDC_CHK_AVBORDER) ? true : false;
			opt.bAvatarRound = IsDlgButtonChecked(hwndDlg, IDC_CHK_ROUNDCORNERSAV) && IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHK_ROUNDCORNERSAV)) ? true : false;
			opt.bOriginalAvatarSize = IsDlgButtonChecked(hwndDlg, IDC_CHK_ORIGINALAVSIZE) ? true : false;

			opt.bShowTitle = IsDlgButtonChecked(hwndDlg, IDC_CHK_SHOWTITLE) ? true : false;
			opt.bShowNoFocus = IsDlgButtonChecked(hwndDlg, IDC_CHK_NOFOCUS) ? true : false;
			opt.bStatusBarTips = IsDlgButtonChecked(hwndDlg, IDC_CHK_SBAR) ? true : false;

			switch (SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_GETCURSEL, 0, 0)) {
			case 0: opt.iLabelValign = DT_TOP; break;
			case 1: opt.iLabelValign = DT_VCENTER; break;
			case 2: opt.iLabelValign = DT_BOTTOM; break;
			}

			switch (SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_GETCURSEL, 0, 0)) {
			case 0: opt.iValueValign = DT_TOP; break;
			case 1: opt.iValueValign = DT_VCENTER; break;
			case 2: opt.iValueValign = DT_BOTTOM; break;
			}

			switch (SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_GETCURSEL, 0, 0)) {
			case 0: opt.iLabelHalign = DT_LEFT; break;
			case 1: opt.iLabelHalign = DT_RIGHT; break;
			}

			switch (SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_GETCURSEL, 0, 0)) {
			case 0: opt.iValueHalign = DT_LEFT; break;
			case 1: opt.iValueHalign = DT_RIGHT; break;
			}

			SaveOptions();
			return TRUE;
		}
		break;
	}

	return 0;
}

INT_PTR CALLBACK DlgProcOptsExtra(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	EXTRAICONDATA *dat = (EXTRAICONDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_CHK_WAITFORCONTENT, opt.bWaitForContent ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_GETSTATUSMSG, opt.bGetNewStatusMsg ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_RETRIEVEXSTATUS, opt.bRetrieveXstatus ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_LIMITMSG, opt.bLimitMsg ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_ENABLESMILEYS, opt.iSmileyAddFlags & SMILEYADD_ENABLE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_USEPROTOSMILEYS, opt.iSmileyAddFlags & SMILEYADD_USEPROTO ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_ONLYISOLATED, opt.iSmileyAddFlags & SMILEYADD_ONLYISOLATED ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_RESIZESMILEYS, opt.iSmileyAddFlags & SMILEYADD_RESIZE ? BST_CHECKED : BST_UNCHECKED);
		{
			BOOL bEnable = opt.iSmileyAddFlags & SMILEYADD_ENABLE;
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_USEPROTOSMILEYS), bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ONLYISOLATED), bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_RESIZESMILEYS), bEnable);
		}

		SendDlgItemMessage(hwndDlg, IDC_SPIN_CHARCOUNT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(1024, 16));
		SetDlgItemInt(hwndDlg, IDC_ED_CHARCOUNT, opt.iLimitCharCount, FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ED_CHARCOUNT), opt.bLimitMsg);

		for (int i = 0; i < EXICONS_COUNT; i++) {
			exIcons[i].order = opt.exIconsOrder[i];
			exIcons[i].vis = opt.exIconsVis[i];
		}

		dat = (EXTRAICONDATA *)mir_calloc(sizeof(EXTRAICONDATA));
		dat->bDragging = false;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		{
			TVINSERTSTRUCT tvi = {};
			tvi.hParent = nullptr;
			tvi.hInsertAfter = TVI_LAST;
			tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
			tvi.item.stateMask = TVIS_STATEIMAGEMASK;
			for (int i = 0; i < _countof(extraIconName); i++) {
				tvi.item.lParam = (LPARAM)&exIcons[i];
				tvi.item.pszText = TranslateW(extraIconName[exIcons[i].order]);
				tvi.item.state = INDEXTOSTATEIMAGEMASK(exIcons[i].vis ? 2 : 1);
				TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &tvi);
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHK_ENABLESMILEYS:
			{
				BOOL bEnable = IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLESMILEYS);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_RESIZESMILEYS), bEnable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_USEPROTOSMILEYS), bEnable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ONLYISOLATED), bEnable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_RESIZESMILEYS), bEnable);
			}
			break;

		case IDC_CHK_LIMITMSG:
			EnableWindow(GetDlgItem(hwndDlg, IDC_ED_CHARCOUNT), IsDlgButtonChecked(hwndDlg, IDC_CHK_LIMITMSG));
			break;
		}

		if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY) {
				TVITEM item = {};
				item.hItem = TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS));

				int i = 0;
				while (item.hItem != nullptr) {
					item.mask = TVIF_HANDLE | TVIF_PARAM;
					TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &item);
					opt.exIconsOrder[i] = ((ICONSTATE *)item.lParam)->order;
					opt.exIconsVis[i] = ((ICONSTATE *)item.lParam)->vis;
					item.hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), item.hItem);
					i++;
				}
				db_set_blob(0, MODULENAME, "IconOrder", opt.exIconsOrder, _countof(opt.exIconsOrder));
				db_set_blob(0, MODULENAME, "icons_vis", opt.exIconsVis, _countof(opt.exIconsVis));

				opt.iSmileyAddFlags = 0;
				opt.iSmileyAddFlags |= (IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLESMILEYS) ? SMILEYADD_ENABLE : 0)
					| (IsDlgButtonChecked(hwndDlg, IDC_CHK_USEPROTOSMILEYS) ? SMILEYADD_USEPROTO : 0)
					| (IsDlgButtonChecked(hwndDlg, IDC_CHK_ONLYISOLATED) ? SMILEYADD_ONLYISOLATED : 0)
					| (IsDlgButtonChecked(hwndDlg, IDC_CHK_RESIZESMILEYS) ? SMILEYADD_RESIZE : 0);

				opt.bWaitForContent = IsDlgButtonChecked(hwndDlg, IDC_CHK_WAITFORCONTENT) ? true : false;
				opt.bGetNewStatusMsg = IsDlgButtonChecked(hwndDlg, IDC_CHK_GETSTATUSMSG) ? true : false;
				opt.bRetrieveXstatus = IsDlgButtonChecked(hwndDlg, IDC_CHK_RETRIEVEXSTATUS) ? true : false;
				opt.bLimitMsg = IsDlgButtonChecked(hwndDlg, IDC_CHK_LIMITMSG) ? true : false;
				opt.iLimitCharCount = GetDlgItemInt(hwndDlg, IDC_ED_CHARCOUNT, nullptr, FALSE);

				g_plugin.setDword("SmileyAddFlags", opt.iSmileyAddFlags);
				g_plugin.setByte("WaitForContent", opt.bWaitForContent ? 1 : 0);
				g_plugin.setByte("GetNewStatusMsg", opt.bGetNewStatusMsg ? 1 : 0);
				g_plugin.setByte("RetrieveXStatus", opt.bRetrieveXstatus ? 1 : 0);
				g_plugin.setByte("LimitMsg", opt.bLimitMsg ? 1 : 0);
				g_plugin.setByte("LimitCharCount", opt.iLimitCharCount);

				return TRUE;
			}
			break;

		case IDC_TREE_EXTRAICONS:
			switch (((LPNMHDR)lParam)->code) {
			case TVN_BEGINDRAG:
				SetCapture(hwndDlg);
				dat->bDragging = true;
				dat->hDragItem = ((LPNMTREEVIEW)lParam)->itemNew.hItem;
				TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), dat->hDragItem);
				break;

			case NM_CLICK:
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti)) {
					if (hti.flags & TVHT_ONITEMSTATEICON) {
						TVITEMA item;
						item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
						item.stateMask = TVIS_STATEIMAGEMASK;
						item.hItem = hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &item);
						if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 1)
							((ICONSTATE *)item.lParam)->vis = 1;
						else
							((ICONSTATE *)item.lParam)->vis = 0;

						TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &item);
						SendMessage((GetParent(hwndDlg)), PSM_CHANGED, (WPARAM)hwndDlg, 0);
					}
				}
				break;
			}
		}
		break;

	case WM_MOUSEMOVE:
		if (dat->bDragging) {
			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(hwndDlg, &hti.pt);
			ScreenToClient(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &hti.pt);
			TreeView_HitTest(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &hti);
			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
				HTREEITEM hItem = hti.hItem;
				hti.pt.y -= TreeView_GetItemHeight(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS)) / 2;
				TreeView_HitTest(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &hti);
				if (!(hti.flags & TVHT_ABOVE))
					TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), hti.hItem, 1);
				else
					TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), hItem, 0);
			}
			else {
				if (hti.flags & TVHT_ABOVE)
					SendDlgItemMessage(hwndDlg, IDC_TREE_EXTRAICONS, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				if (hti.flags & TVHT_BELOW)
					SendDlgItemMessage(hwndDlg, IDC_TREE_EXTRAICONS, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);

				TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), 0, 0);
			}
		}
		break;

	case WM_LBUTTONUP:
		if (!dat->bDragging)
			break;

		TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), 0, 0);
		dat->bDragging = false;
		ReleaseCapture();

		TVHITTESTINFO hti;
		hti.pt.x = (short)LOWORD(lParam);
		hti.pt.y = (short)HIWORD(lParam);
		ClientToScreen(hwndDlg, &hti.pt);
		ScreenToClient(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &hti.pt);
		hti.pt.y -= TreeView_GetItemHeight(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS)) / 2;
		TreeView_HitTest(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &hti);
		if (dat->hDragItem == hti.hItem)
			break;

		if (hti.flags & TVHT_ABOVE)
			hti.hItem = TVI_FIRST;

		TVITEM item;
		item.mask = TVIF_HANDLE | TVIF_PARAM;
		item.hItem = dat->hDragItem;
		TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &item);
		if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT) || (hti.hItem == TVI_FIRST)) {
			TVINSERTSTRUCT tvis;
			wchar_t swzName[256];
			tvis.item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
			tvis.item.stateMask = TVIS_STATEIMAGEMASK;
			tvis.item.pszText = swzName;
			tvis.item.cchTextMax = _countof(swzName);
			tvis.item.hItem = dat->hDragItem;
			tvis.item.state = INDEXTOSTATEIMAGEMASK(((ICONSTATE *)item.lParam)->vis ? 2 : 1);
			TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &tvis.item);
			TreeView_DeleteItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), dat->hDragItem);
			tvis.hParent = nullptr;
			tvis.hInsertAfter = hti.hItem;
			TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &tvis));
			SendMessage((GetParent(hwndDlg)), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		}

		break;

	case WM_DESTROY:
		mir_free(dat);
		break;
	}
	return 0;
}

void EnableControls(HWND hwndDlg, bool bEnableSkin)
{
	ShowWindow(GetDlgItem(hwndDlg, IDC_ST_PREVIEW), (bEnableSkin && opt.szPreviewFile[0]) ? SW_HIDE : SW_SHOW);
	ShowWindow(GetDlgItem(hwndDlg, IDC_PIC_PREVIEW), (bEnableSkin && opt.szPreviewFile[0]) ? SW_SHOW : SW_HIDE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_SHADOW), !bEnableSkin);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_BORDER), !bEnableSkin);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ROUNDCORNERS), !bEnableSkin);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_LOADFONTS), bEnableSkin);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_LOADPROPORTIONS), bEnableSkin);

	if (!bEnableSkin) {
		CheckDlgButton(hwndDlg, IDC_CHK_ENABLECOLORING, BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ENABLECOLORING), FALSE);
	}
	else if (opt.iEnableColoring == -1) {
		CheckDlgButton(hwndDlg, IDC_CHK_ENABLECOLORING, BST_CHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ENABLECOLORING), FALSE);
	}
	else {
		CheckDlgButton(hwndDlg, IDC_CHK_ENABLECOLORING, opt.iEnableColoring ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ENABLECOLORING), TRUE);
	}
}

int iLastSel;
INT_PTR CALLBACK DlgProcOptsSkin(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		iLastSel = RefreshSkinList(hwndDlg);

		SendDlgItemMessage(hwndDlg, IDC_BTN_RELOADLIST, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_RELOADLIST, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Reload skin list"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_BTN_RELOADLIST, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(iconList[5].hIcolib));

		SendDlgItemMessage(hwndDlg, IDC_BTN_APPLYSKIN, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_APPLYSKIN, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Apply skin"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_BTN_APPLYSKIN, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(iconList[6].hIcolib));

		SendDlgItemMessage(hwndDlg, IDC_BTN_GETSKINS, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BTN_GETSKINS, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Get more skins"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_BTN_GETSKINS, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadIcon(SKINICON_EVENT_URL));


		SendDlgItemMessage(hwndDlg, IDC_SPIN_TRANS, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 0));
		SetDlgItemInt(hwndDlg, IDC_ED_TRANS, opt.iOpacity, FALSE);

		CheckDlgButton(hwndDlg, IDC_CHK_BORDER, opt.bBorder ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_ROUNDCORNERS, opt.bRound ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_SHADOW, opt.bDropShadow ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_AEROGLASS, opt.bAeroGlass ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_LOADFONTS, opt.bLoadFonts ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_LOADPROPORTIONS, opt.bLoadProportions ? BST_CHECKED : BST_UNCHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_AEROGLASS), MyDwmEnableBlurBehindWindow != nullptr);

		SendDlgItemMessage(hwndDlg, IDC_CMB_EFFECT, CB_ADDSTRING, 0, (LPARAM)TranslateT("None"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_EFFECT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Animation"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_EFFECT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Fade"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_EFFECT, CB_SETCURSEL, (int)opt.showEffect, 0);

		SendDlgItemMessage(hwndDlg, IDC_SPIN_SPEED, UDM_SETRANGE, 0, (LPARAM)MAKELONG(20, 1));
		SetDlgItemInt(hwndDlg, IDC_ED_SPEED, opt.iAnimateSpeed, FALSE);

		EnableControls(hwndDlg, opt.skinMode == SM_IMAGE);
		return TRUE;

	case WM_DRAWITEM:
		if (wParam == IDC_PIC_PREVIEW) {
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			HBRUSH hbr = GetSysColorBrush(COLOR_BTNFACE);
			FillRect(dis->hDC, &dis->rcItem, hbr);

			if (opt.szPreviewFile[0]) {
				HDC hdcMem = CreateCompatibleDC(dis->hDC);
				HBITMAP hbmpPreview = Bitmap_Load(opt.szPreviewFile);
				if (hbmpPreview) {
					int iWidth = dis->rcItem.right - dis->rcItem.left;
					int iHeight = dis->rcItem.bottom - dis->rcItem.top;

					HBITMAP hbmpRes = Image_Resize(hbmpPreview, RESIZEBITMAP_KEEP_PROPORTIONS, iWidth, iHeight);
					if (hbmpRes) {
						BITMAP bmp;
						GetObject(hbmpRes, sizeof(bmp), &bmp);
						SelectObject(hdcMem, hbmpRes);
						BitBlt(dis->hDC, (iWidth - bmp.bmWidth) / 2, (iHeight - bmp.bmHeight) / 2, iWidth, iHeight, hdcMem, 0, 0, SRCCOPY);
						if (hbmpPreview != hbmpRes)
							DeleteBitmap(hbmpRes);
					}

					DeleteBitmap(hbmpPreview);
				}

				DeleteDC(hdcMem);
			}
		}
		break;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case LBN_SELCHANGE:
			if (LOWORD(wParam) == IDC_LB_SKINS) {
				HWND hwndList = GetDlgItem(hwndDlg, IDC_LB_SKINS);
				int iSel = ListBox_GetCurSel(hwndList);
				if (iSel != iLastSel) {
					if (iSel == 0) {
						opt.szPreviewFile[0] = 0;
						EnableControls(hwndDlg, false);
					}
					else if (iSel != LB_ERR) {
						wchar_t swzSkinName[256];
						if (ListBox_GetText(hwndList, iSel, swzSkinName) > 0)
							ParseSkinFile(swzSkinName, false, true);
						EnableControls(hwndDlg, true);
					}

					InvalidateRect(GetDlgItem(hwndDlg, IDC_PIC_PREVIEW), nullptr, FALSE);
					iLastSel = iSel;
				}
			}
			else if (LOWORD(wParam) == IDC_CMB_EFFECT) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}

			break;

		case BN_CLICKED:
			if (LOWORD(wParam) == IDC_BTN_APPLYSKIN) {
				int iSel = ListBox_GetCurSel(GetDlgItem(hwndDlg, IDC_LB_SKINS));
				if (iSel == 0) {
					opt.skinMode = SM_COLORFILL;
					opt.szSkinName[0] = 0;
				}
				else if (iSel != LB_ERR) {
					if (ListBox_GetText(GetDlgItem(hwndDlg, IDC_LB_SKINS), iSel, opt.szSkinName) > 0) {
						opt.skinMode = SM_IMAGE;
						ParseSkinFile(opt.szSkinName, false, false);
						ReloadFont(0, 0);
						SaveOptions();
					}
				}

				g_plugin.setByte("SkinEngine", opt.skinMode);
				g_plugin.setWString("SkinName", opt.szSkinName);

				DestroySkinBitmap();
				SetDlgItemInt(hwndDlg, IDC_ED_TRANS, opt.iOpacity, FALSE);
			}
			else if (LOWORD(wParam) == IDC_BTN_RELOADLIST)
				iLastSel = RefreshSkinList(hwndDlg);
			else if (LOWORD(wParam) == IDC_CHK_LOADFONTS)
				opt.bLoadFonts = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOADFONTS) ? true : false;
			else if (LOWORD(wParam) == IDC_CHK_LOADPROPORTIONS)
				opt.bLoadProportions = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOADPROPORTIONS) ? true : false;
			else if (LOWORD(wParam) == IDC_BTN_GETSKINS)
				Utils_OpenUrl("https://miranda-ng.org/tags/tipper/");

			break;
		}

		if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY) {
			opt.iOpacity = GetDlgItemInt(hwndDlg, IDC_ED_TRANS, nullptr, 0);
			opt.bDropShadow = IsDlgButtonChecked(hwndDlg, IDC_CHK_SHADOW) ? true : false;
			opt.bBorder = IsDlgButtonChecked(hwndDlg, IDC_CHK_BORDER) ? true : false;
			opt.bRound = IsDlgButtonChecked(hwndDlg, IDC_CHK_ROUNDCORNERS) ? true : false;
			opt.bAeroGlass = IsDlgButtonChecked(hwndDlg, IDC_CHK_AEROGLASS) ? true : false;
			opt.showEffect = (PopupShowEffect)SendDlgItemMessage(hwndDlg, IDC_CMB_EFFECT, CB_GETCURSEL, 0, 0);
			opt.iAnimateSpeed = GetDlgItemInt(hwndDlg, IDC_ED_SPEED, nullptr, 0);
			opt.bLoadFonts = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOADFONTS) ? true : false;
			opt.bLoadProportions = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOADPROPORTIONS) ? true : false;

			if (opt.iEnableColoring != -1)
				opt.iEnableColoring = IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLECOLORING) ? true : false;

			SaveSkinOptions();
			return TRUE;
		}
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProcFavouriteContacts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, Clist::bUseGroups, 0);

		for (auto &hContact : Contacts()) {
			HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
			if (hItem && g_plugin.getByte(hContact, "FavouriteContact"))
				SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
		}

		CheckDlgButton(hwndDlg, IDC_CHK_HIDEOFFLINE, opt.iFavoriteContFlags & FAVCONT_HIDE_OFFLINE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_APPENDPROTO, opt.iFavoriteContFlags & FAVCONT_APPEND_PROTO ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BTN_OK:
			{
				uint8_t isChecked;
				int count = 0;

				for (auto &hContact : Contacts()) {
					HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
					if (hItem) {
						isChecked = (uint8_t)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0);
						g_plugin.setByte(hContact, "FavouriteContact", isChecked);
						if (isChecked)
							count++;
					}
				}
				g_plugin.setDword("FavouriteContactsCount", count);

				opt.iFavoriteContFlags = (IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDEOFFLINE) ? FAVCONT_HIDE_OFFLINE : 0)
					| (IsDlgButtonChecked(hwndDlg, IDC_CHK_APPENDPROTO) ? FAVCONT_APPEND_PROTO : 0);

				g_plugin.setDword("FavContFlags", opt.iFavoriteContFlags);
			}
			__fallthrough;

		case IDC_BTN_CANCEL:
			DestroyWindow(hwndDlg);
			break;
		}
		return TRUE;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProcOptsTraytip(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_CHK_ENABLETRAYTIP, opt.bTraytip ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_HANDLEBYTIPPER, opt.bHandleByTipper ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_EXPAND, opt.bExpandTraytip ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_HIDEOFFLINE, opt.bHideOffline ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_SPIN_EXPANDTIME, UDM_SETRANGE, 0, (LPARAM)MAKELONG(5000, 10));
		SetDlgItemInt(hwndDlg, IDC_ED_EXPANDTIME, opt.iExpandTime, FALSE);
		SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_CHK_ENABLETRAYTIP, 0), 0);

		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		{
			TVINSERTSTRUCT tvi = {};
			tvi.hParent = nullptr;
			tvi.hInsertAfter = TVI_LAST;
			tvi.item.mask = TVIF_TEXT | TVIF_STATE;

			for (auto &pa : Accounts()) {
				if (CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0) != 0) {
					tvi.item.pszText = pa->tszAccountName;
					tvi.item.stateMask = TVIS_STATEIMAGEMASK;
					tvi.item.state = INDEXTOSTATEIMAGEMASK(IsTrayProto(pa->tszAccountName, false) ? 2 : 1);
					TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS), &tvi);
					tvi.item.state = INDEXTOSTATEIMAGEMASK(IsTrayProto(pa->tszAccountName, true) ? 2 : 1);
					TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS), &tvi);
				}
			}

			for (int i = 0; i < _countof(trayTipItems); i++) {
				tvi.item.pszText = TranslateW(trayTipItems[i]);
				tvi.item.state = INDEXTOSTATEIMAGEMASK(opt.iFirstItems & (1 << i) ? 2 : 1);
				TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &tvi);
				tvi.item.state = INDEXTOSTATEIMAGEMASK(opt.iSecondItems & (1 << i) ? 2 : 1);
				TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS), &tvi);
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			UINT state;
		case IDC_CHK_ENABLETRAYTIP:
			state = IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLETRAYTIP);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_HANDLEBYTIPPER), state);
			__fallthrough;

		case IDC_CHK_HANDLEBYTIPPER:
			state = IsDlgButtonChecked(hwndDlg, IDC_CHK_HANDLEBYTIPPER) & IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLETRAYTIP);

			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_HIDEOFFLINE), state);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS), state);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), state);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_FAVCONTACTS), state);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_EXPAND), state);
			__fallthrough;

		case IDC_CHK_EXPAND:
			state = IsDlgButtonChecked(hwndDlg, IDC_CHK_HANDLEBYTIPPER)
				& IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLETRAYTIP)
				& IsDlgButtonChecked(hwndDlg, IDC_CHK_EXPAND);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS), state);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS), state);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ED_EXPANDTIME), state);
			break;

		case IDC_BTN_FAVCONTACTS:
			CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FAVCONTACTS), nullptr, DlgProcFavouriteContacts);
			break;
		}

		if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY) {
				wchar_t buff[256];
				wchar_t swzProtos[1024] = { 0 };

				TVITEM item;
				item.hItem = TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS));
				item.pszText = buff;
				item.cchTextMax = _countof(buff);
				item.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_STATE;
				item.stateMask = TVIS_STATEIMAGEMASK;
				while (item.hItem != nullptr) {
					TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS), &item);
					if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 2) {
						mir_wstrcat(swzProtos, buff);
						mir_wstrcat(swzProtos, L" ");
					}

					item.hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS), item.hItem);
				}

				g_plugin.setWString("TrayProtocols", swzProtos);

				swzProtos[0] = 0;
				item.hItem = TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS));
				item.pszText = buff;
				while (item.hItem != nullptr) {
					TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS), &item);
					if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 2) {
						mir_wstrcat(swzProtos, buff);
						mir_wstrcat(swzProtos, L" ");
					}

					item.hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS), item.hItem);
				}

				g_plugin.setWString("TrayProtocolsEx", swzProtos);

				int count = 0;
				opt.iFirstItems = 0;
				item.hItem = TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
				item.mask = TVIF_HANDLE | TVIF_STATE;
				item.stateMask = TVIS_STATEIMAGEMASK;
				while (item.hItem != nullptr) {
					TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item);
					if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 2)
						opt.iFirstItems |= (1 << count);

					item.hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), item.hItem);
					count++;
				}

				count = 0;
				opt.iSecondItems = 0;
				item.hItem = TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS));
				while (item.hItem != nullptr) {
					TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS), &item);
					if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 2)
						opt.iSecondItems |= (1 << count);

					item.hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS), item.hItem);
					count++;
				}

				opt.bTraytip = IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLETRAYTIP) ? true : false;
				opt.bHandleByTipper = IsDlgButtonChecked(hwndDlg, IDC_CHK_HANDLEBYTIPPER) ? true : false;
				opt.bExpandTraytip = IsDlgButtonChecked(hwndDlg, IDC_CHK_EXPAND) ? true : false;
				opt.bHideOffline = IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDEOFFLINE) ? true : false;
				opt.iExpandTime = max(min(GetDlgItemInt(hwndDlg, IDC_ED_EXPANDTIME, nullptr, FALSE), 5000), 10);

				g_plugin.setByte("TrayTip", (opt.bTraytip ? 1 : 0));
				g_plugin.setByte("ExtendedTrayTip", (opt.bHandleByTipper ? 1 : 0));
				g_plugin.setByte("ExpandTrayTip", (opt.bExpandTraytip ? 1 : 0));
				g_plugin.setByte("HideOffline", (opt.bHideOffline ? 1 : 0));
				g_plugin.setDword("ExpandTime", opt.iExpandTime);
				g_plugin.setDword("TrayTipItems", opt.iFirstItems);
				g_plugin.setDword("TrayTipItemsEx", opt.iSecondItems);
				return TRUE;
			}
			break;

		case IDC_TREE_FIRST_PROTOS:
		case IDC_TREE_SECOND_PROTOS:
		case IDC_TREE_FIRST_ITEMS:
		case IDC_TREE_SECOND_ITEMS:
			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
					if (hti.flags & TVHT_ONITEMSTATEICON)
						SendMessage((GetParent(hwndDlg)), PSM_CHANGED, (WPARAM)hwndDlg, 0);
			}
			break;
		}
		break;
	}

	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = -790000000;

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_APPEARANCE);
	odp.szTab.a = LPGEN("Appearance");
	odp.szTitle.a = LPGEN("Tooltips");
	odp.szGroup.a = LPGEN("Customize");
	odp.pfnDlgProc = DlgProcOptsAppearance;
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SKIN);
	odp.szTab.a = LPGEN("Tooltips");
	odp.szGroup.a = LPGEN("Skins");
	odp.pfnDlgProc = DlgProcOptsSkin;
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CONTENT);
	odp.szTab.a = LPGEN("Content");
	odp.szTitle.a = LPGEN("Tooltips");
	odp.szGroup.a = LPGEN("Customize");
	odp.pfnDlgProc = DlgProcOptsContent;
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TRAYTIP);
	odp.szTab.a = LPGEN("Tray tooltip");
	odp.szTitle.a = LPGEN("Tooltips");
	odp.szGroup.a = LPGEN("Customize");
	odp.pfnDlgProc = DlgProcOptsTraytip;
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_EXTRA);
	odp.szTab.a = LPGEN("Extra");
	odp.szTitle.a = LPGEN("Tooltips");
	odp.szGroup.a = LPGEN("Customize");
	odp.pfnDlgProc = DlgProcOptsExtra;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

void DeinitOptions()
{
	DIListNode *di_node = opt.diList;
	while (opt.diList) {
		di_node = opt.diList;
		opt.diList = opt.diList->next;
		mir_free(di_node);
	}

	DSListNode *ds_node = opt.dsList;
	while (opt.dsList) {
		ds_node = opt.dsList;
		opt.dsList = opt.dsList->next;
		mir_free(ds_node);
	}

	for (int i = 0; i < SKIN_ITEMS_COUNT; i++)
		if (opt.szImgFile[i])
			mir_free(opt.szImgFile[i]);
}
