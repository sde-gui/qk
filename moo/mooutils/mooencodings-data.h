enum {
    ENCODING_GROUP_WEST_EUROPEAN,
    ENCODING_GROUP_EAST_EUROPEAN,
    ENCODING_GROUP_EAST_ASIAN,
    ENCODING_GROUP_SE_SW_ASIAN,
    ENCODING_GROUP_MIDDLE_EASTERN,
    ENCODING_GROUP_UNICODE,
    N_ENCODING_GROUPS
};

static const char * const moo_encoding_groups_names[] = {
    N_("West European"),
    N_("East European"),
    N_("East Asian"),
    N_("SE & SW Asian"),
    N_("Middle Eastern"),
    N_("Unicode")
};

static const struct {
    const char *name;
    const char *alias;
} const moo_encoding_aliases[] = {
    {"UTF-8", "UTF8"}
};

/* The stuff below if from profterm:
 *
 * Copyright (C) 2002 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 */

static const struct {
    const char *name;
    const char *display_subgroup;
    const char *short_display_name;
    guint group : 3;
} const moo_encodings_data[] =
{
    { "ISO-8859-1",     N_("Western"),             "ISO-8859-1",       ENCODING_GROUP_WEST_EUROPEAN },
    { "ISO-8859-2",     N_("Central European"),    "ISO-8859-2",       ENCODING_GROUP_EAST_EUROPEAN },
    { "ISO-8859-3",     N_("South European"),      "ISO-8859-3",       ENCODING_GROUP_WEST_EUROPEAN },
    { "ISO-8859-4",     N_("Baltic"),              "ISO-8859-4",       ENCODING_GROUP_EAST_EUROPEAN },
    { "ISO-8859-5",     N_("Cyrillic"),            "ISO-8859-5",       ENCODING_GROUP_EAST_EUROPEAN },
    { "ISO-8859-6",     N_("Arabic"),              "ISO-8859-6",       ENCODING_GROUP_MIDDLE_EASTERN },
    { "ISO-8859-7",     N_("Greek"),               "ISO-8859-7",       ENCODING_GROUP_WEST_EUROPEAN },
    { "ISO-8859-8",     N_("Hebrew Visual"),       "ISO-8859-8",       ENCODING_GROUP_MIDDLE_EASTERN },
    { "ISO-8859-8-I",   N_("Hebrew"),              "ISO-8859-8-I",     ENCODING_GROUP_MIDDLE_EASTERN },
    { "ISO-8859-9",     N_("Turkish"),             "ISO-8859-9",       ENCODING_GROUP_SE_SW_ASIAN },
    { "ISO-8859-10",    N_("Nordic"),              "ISO-8859-10",      ENCODING_GROUP_WEST_EUROPEAN },
    { "ISO-8859-13",    N_("Baltic"),              "ISO-8859-13",      ENCODING_GROUP_EAST_EUROPEAN },
    { "ISO-8859-14",    N_("Celtic"),              "ISO-8859-14",      ENCODING_GROUP_WEST_EUROPEAN },
    { "ISO-8859-15",    N_("Western"),             "ISO-8859-15",      ENCODING_GROUP_WEST_EUROPEAN },
    { "ISO-8859-16",    N_("Romanian"),            "ISO-8859-16",      ENCODING_GROUP_EAST_EUROPEAN },

    { "UTF-7",          N_("Unicode"),             "UTF-7",            ENCODING_GROUP_UNICODE },
    { "UTF-8",          N_("Unicode"),             "UTF-8",            ENCODING_GROUP_UNICODE },
    { "UTF-16",         N_("Unicode"),             "UTF-16",           ENCODING_GROUP_UNICODE },
    { "UCS-2",          N_("Unicode"),             "UCS-2",            ENCODING_GROUP_UNICODE },
    { "UCS-4",          N_("Unicode"),             "UCS-4",            ENCODING_GROUP_UNICODE },

    { "ARMSCII-8",      N_("Armenian"),            "ARMSCII-8",        ENCODING_GROUP_SE_SW_ASIAN },
    { "BIG5",           N_("Chinese Traditional"), "Big5",             ENCODING_GROUP_EAST_ASIAN },
    { "BIG5-HKSCS",     N_("Chinese Traditional"), "Big5-HKSCS",       ENCODING_GROUP_EAST_ASIAN },
    { "CP866",          N_("Cyrillic/Russian"),    "CP866",            ENCODING_GROUP_EAST_EUROPEAN },

    { "EUC-JP",         N_("Japanese"),            "EUC-JP",           ENCODING_GROUP_EAST_ASIAN },
    { "EUC-KR",         N_("Korean"),              "EUC-KR",           ENCODING_GROUP_EAST_ASIAN },
    { "EUC-TW",         N_("Chinese Traditional"), "EUC-TW",           ENCODING_GROUP_EAST_ASIAN },

    { "GB18030",        N_("Chinese Simplified"),  "GB18030",          ENCODING_GROUP_EAST_ASIAN },
    { "GB2312",         N_("Chinese Simplified"),  "GB2312",           ENCODING_GROUP_EAST_ASIAN },
    { "GBK",            N_("Chinese Simplified"),  "GBK",              ENCODING_GROUP_EAST_ASIAN },
    { "GEORGIAN-PS",    N_("Georgian"),            "GEORGIAN-PS",      ENCODING_GROUP_SE_SW_ASIAN },
    { "HZ",             N_("Chinese Simplified"),  "HZ",               ENCODING_GROUP_EAST_ASIAN },

    { "IBM850",         N_("Western"),             "IBM850",           ENCODING_GROUP_WEST_EUROPEAN },
    { "IBM852",         N_("Central European"),    "IBM852",           ENCODING_GROUP_EAST_EUROPEAN },
    { "IBM855",         N_("Cyrillic"),            "IBM855",           ENCODING_GROUP_EAST_EUROPEAN },
    { "IBM857",         N_("Turkish"),             "IBM857",           ENCODING_GROUP_SE_SW_ASIAN },
    { "IBM862",         N_("Hebrew"),              "IBM862",           ENCODING_GROUP_MIDDLE_EASTERN },
    { "IBM864",         N_("Arabic"),              "IBM864",           ENCODING_GROUP_MIDDLE_EASTERN },

    { "ISO2022JP",      N_("Japanese"),            "ISO2022JP",        ENCODING_GROUP_EAST_ASIAN },
    { "ISO2022KR",      N_("Korean"),              "ISO2022KR",        ENCODING_GROUP_EAST_ASIAN },
    { "ISO-IR-111",     N_("Cyrillic"),            "ISO-IR-111",       ENCODING_GROUP_EAST_EUROPEAN },
    { "JOHAB",          N_("Korean"),              "JOHAB",            ENCODING_GROUP_EAST_ASIAN },
    { "KOI8-R",         N_("Cyrillic"),            "KOI8-R",           ENCODING_GROUP_EAST_EUROPEAN },
    { "KOI8-U",         N_("Cyrillic/Ukrainian"),  "KOI8-U",           ENCODING_GROUP_EAST_EUROPEAN },

    { "MAC_ARABIC",     N_("Arabic"),              "MacArabic",        ENCODING_GROUP_MIDDLE_EASTERN },
    { "MAC_CE",         N_("Central European"),    "MacCE",            ENCODING_GROUP_EAST_EUROPEAN },
    { "MAC_CROATIAN",   N_("Croatian"),            "MacCroatian",      ENCODING_GROUP_EAST_EUROPEAN },
    { "MAC-CYRILLIC",   N_("Cyrillic"),            "MacCyrillic",      ENCODING_GROUP_EAST_EUROPEAN },
    { "MAC_DEVANAGARI", N_("Hindi"),               "MacDevanagari",    ENCODING_GROUP_SE_SW_ASIAN },
    { "MAC_FARSI",      N_("Persian"),             "MacFarsi",         ENCODING_GROUP_MIDDLE_EASTERN },
    { "MAC_GREEK",      N_("Greek"),               "MacGreek",         ENCODING_GROUP_WEST_EUROPEAN },
    { "MAC_GUJARATI",   N_("Gujarati"),            "MacGujarati",      ENCODING_GROUP_SE_SW_ASIAN },
    { "MAC_GURMUKHI",   N_("Gurmukhi"),            "MacGurmukhi",      ENCODING_GROUP_SE_SW_ASIAN },
    { "MAC_HEBREW",     N_("Hebrew"),              "MacHebrew",        ENCODING_GROUP_MIDDLE_EASTERN },
    { "MAC_ICELANDIC",  N_("Icelandic"),           "MacIcelandic",     ENCODING_GROUP_WEST_EUROPEAN },
    { "MAC_ROMAN",      N_("Western"),             "MacRoman",         ENCODING_GROUP_WEST_EUROPEAN },
    { "MAC_ROMANIAN",   N_("Romanian"),            "MacRomanian",      ENCODING_GROUP_EAST_EUROPEAN },
    { "MAC_TURKISH",    N_("Turkish"),             "MacTurkish",       ENCODING_GROUP_SE_SW_ASIAN },
    { "MAC_UKRAINIAN",  N_("Cyrillic/Ukrainian"),  "MacUkrainian",     ENCODING_GROUP_EAST_EUROPEAN },

    { "SHIFT-JIS",      N_("Japanese"),            "Shift_JIS",        ENCODING_GROUP_EAST_ASIAN },
    { "TCVN",           N_("Vietnamese"),          "TCVN",             ENCODING_GROUP_EAST_ASIAN },
    { "TIS-620",        N_("Thai"),                "TIS-620",          ENCODING_GROUP_SE_SW_ASIAN },
    { "UHC",            N_("Korean"),              "UHC",              ENCODING_GROUP_EAST_ASIAN },
    { "VISCII",         N_("Vietnamese"),          "VISCII",           ENCODING_GROUP_EAST_ASIAN },

    { "WINDOWS-1250",   N_("Central European"),    "Windows-1250",     ENCODING_GROUP_EAST_EUROPEAN },
    { "WINDOWS-1251",   N_("Cyrillic"),            "Windows-1251",     ENCODING_GROUP_EAST_EUROPEAN },
    { "WINDOWS-1252",   N_("Western"),             "Windows-1252",     ENCODING_GROUP_WEST_EUROPEAN },
    { "WINDOWS-1253",   N_("Greek"),               "Windows-1253",     ENCODING_GROUP_WEST_EUROPEAN },
    { "WINDOWS-1254",   N_("Turkish"),             "Windows-1254",     ENCODING_GROUP_SE_SW_ASIAN },
    { "WINDOWS-1255",   N_("Hebrew"),              "Windows-1255",     ENCODING_GROUP_MIDDLE_EASTERN },
    { "WINDOWS-1256",   N_("Arabic"),              "Windows-1256",     ENCODING_GROUP_MIDDLE_EASTERN },
    { "WINDOWS-1257",   N_("Baltic"),              "Windows-1257",     ENCODING_GROUP_EAST_EUROPEAN },
    { "WINDOWS-1258",   N_("Vietnamese"),          "Windows-1258",     ENCODING_GROUP_EAST_ASIAN }
};
