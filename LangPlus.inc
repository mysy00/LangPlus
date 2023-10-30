#if defined _inc_LangPlus
#undef _inc_LangPlus
#endif

#if defined _LangPlus_included
#endinput
#endif
#define _LangPlus_included

#if !defined PP_SYNTAX_FOR_POOL
    #define PP_SYNTAX_FOR_POOL
#endif

#if !defined PP_SYNTAX_FOR_MAP
    #define PP_SYNTAX_FOR_MAP
#endif

#include <open.mp>
#include <sscanf2>
#include <PawnPlus>
#include <strlib>
#tryinclude <YSI_Coding\y_va>
#tryinclude <YSI_Data\y_iterate>
#tryinclude <logger>

#if !defined MAX_LANGUAGES
    #define MAX_LANGUAGES 4
#endif

#if !defined DELIMITER_CHAR
    #define DELIMITER_CHAR "="
#endif

// the directory language data is stored in
#if !defined DIRECTORY_LANGUAGES
	#define DIRECTORY_LANGUAGES "languages/"
#endif

// the maximum length of a language key
#if !defined MAX_LANGUAGE_KEY_LEN
	#define MAX_LANGUAGE_KEY_LEN (32)
#endif

// the maximum length of a language text entry
#if !defined MAX_LANGUAGE_ENTRY_LENGTH
	#define MAX_LANGUAGE_ENTRY_LENGTH (768)
#endif

// name limit for a language
#if !defined MAX_LANGUAGE_NAME
	#define MAX_LANGUAGE_NAME (32)
#endif

// Max file name length for a language file
#if !defined MAX_FILE_NAME
    #define MAX_FILE_NAME (64)
#endif

#if !defined MAX_REPLACEMENT_KEY_LEN
    #define MAX_REPLACEMENT_KEY_LEN (16)
#endif

#if !defined MAX_REPLACEMENT_VALUE_LEN
    #define MAX_REPLACEMENT_VALUE_LEN (16)
#endif

#define INVALID_LANGUAGE_ID (Language:-1)

#if defined SS_LANG_COMPATIBILITY
    #define DefineLanguageReplacement LangPlus_SetStringReplacement
    #define InitLanguage LangPlus_LoadLanguage
    #define AddLanguageEntry LangPlus_AddLanguageEntry
    #define GetLanguageString LangPlus_GetLanguageString
    #define GetLanguageList LangPlus_GetLanguageList
    #define GetLanguageID LangPlus_GetLanguageId
    #define GetLanguageName LangPlus_GetLanguageName
    #define GetPlayerLanguage LangPlus_GetPlayerLanguage
    #define SetPlayerLanguage LangPlus_SetPlayerLanguage
    #define @L(%0,%1) GetLanguageString(GetPlayerLanguage(%0), %1)
#endif

static Map:s_LanguageMap[Language:MAX_LANGUAGES],
       Pool:s_LanguagePool,
       Map:s_StringReplacements;

static Language:s_PlayerLanguage[MAX_PLAYERS];

public OnGameModeInit() {
    s_LanguagePool = pool_new(MAX_LANGUAGES);

    #if defined LangPlus_OnGameModeInit
    return LangPlus_OnGameModeInit();
    #else
    return 1;
    #endif
}

public OnPlayerConnect(playerid) {
    s_PlayerLanguage[playerid] = Language:0;

    #if defined LangPlus_OnPlayerConnect
    return LangPlus_OnPlayerConnect(playerid);
    #else
    return 1;
    #endif
}

static stock Language:_InitLanguage(const string:langName[]) {
    if (pool_size(s_LanguagePool) == MAX_LANGUAGES) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "too many languages",
                   Logger_S("language", langName));
        #endif
        return INVALID_LANGUAGE_ID;
    }

    // pool_find_str returns the index, it's not a bool!
    if (pool_find_str(s_LanguagePool, langName) != -1) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "language already exists",
                   Logger_S("language", langName));
        #endif
        return INVALID_LANGUAGE_ID;
    }

    return Language:pool_add_str(s_LanguagePool, langName);
}

static bool:LangPlus_IsValidLanguage(Language:id) {
    if (!pool_valid(s_LanguagePool)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "no languages have been initialized");
        #endif

        return false;
    }

    if (_:id < 0 || _:id >= pool_size(s_LanguagePool)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id",
                   Logger_I("languageid", _:id));
        #endif

        return false;
    }

    return pool_has(s_LanguagePool, _:id);
}

LangPlus_AddLanguageEntry(const Language:languageid, const string:key[], const string:val[]) {
    if (!LangPlus_IsValidLanguage(languageid) || key[0] == EOS || val[0] == EOS) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id or key/value",
                   Logger_I("languageid", _:languageid),
                   Logger_S("key", key),
                   Logger_S("val", val));
        #endif
        return false;
    }

    if (!pool_has(s_LanguagePool, _:languageid))  {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id",
                   Logger_I("languageid", _:languageid));
        #endif
        return false;
    }

    if (!map_valid(s_LanguageMap[languageid])) {
        s_LanguageMap[languageid] = map_new();
    }

    if (map_has_str_key(s_LanguageMap[languageid], key)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "language key already exists",
                   Logger_S("key", key));
        #endif
        return false;
    }

    map_str_add_str(s_LanguageMap[languageid], key, val);

    return true;
}

// This one shouldn't be a stock.
// Importing the library and not using it is pointless
Language:LangPlus_LoadLanguage(const string:langName[]) {
    new Language:languageid = _InitLanguage(langName);
    if (languageid == INVALID_LANGUAGE_ID) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "unable to initialize language",
                   Logger_S("language", langName));
        #endif

        return INVALID_LANGUAGE_ID;
    }

    new fileName[MAX_FILE_NAME],
        File:fHandler,
        line[MAX_LANGUAGE_KEY_LEN + 1 + MAX_LANGUAGE_ENTRY_LENGTH],
        lineNumber = 1,
        lineLength;

    format(fileName, sizeof(fileName), #DIRECTORY_LANGUAGES "%s.ini", langName);
    fHandler = fopen(fileName, io_read);

    if (!fHandler) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "unable to open file",
                   Logger_S("fileName", fileName));
        #endif

        return INVALID_LANGUAGE_ID;
    }

    new key[MAX_LANGUAGE_KEY_LEN],
        val[MAX_LANGUAGE_ENTRY_LENGTH],
        valFormatted[MAX_LANGUAGE_ENTRY_LENGTH];

    while (fread(fHandler, line)) {
        lineLength = strlen(line);

        if (lineLength < 3) {
            // Minimum length of a line is 3 characters: "a=b"
            continue;
        }

        if (
            (line[0] < 97 || line[0] > 122)
            && (line[0] < 65 || line[0] > 90)
        ) {
            // Skip lines that don't start with a letter (most likely comments)
            // We also skip sections. These are only for users and translation software
            continue;
        }

        if (sscanf(line, "p<"#DELIMITER_CHAR">s["#MAX_LANGUAGE_KEY_LEN"]s["#MAX_LANGUAGE_ENTRY_LENGTH"]", key, val)) {
            #if(defined _logger_included)
            Logger_Dbg("langplus", "invalid line format",
                       Logger_I("line", lineNumber),
                       Logger_S("file", fileName),
                       Logger_S("line", line));
            #endif

            return INVALID_LANGUAGE_ID;
        }

        strfromliteral(valFormatted, val);

        if (map_valid(s_StringReplacements)) {
            for_map(it : s_StringReplacements) {
                new replacementKey[MAX_REPLACEMENT_KEY_LEN + 1],
                    replacementValue[MAX_REPLACEMENT_VALUE_LEN + 1];

                iter_get_key_str(it, replacementKey);
                iter_get_value_str(it, replacementValue);

                strreplace(valFormatted, replacementKey, replacementValue);
            }
        }

        LangPlus_AddLanguageEntry(languageid, key, valFormatted);

        lineNumber++;
    }

    fclose(fHandler);

    return languageid;
}

stock bool:LangPlus_SetStringReplacement(const string:key[], const string:value[]) {
    if (key[0] == EOS || value[0] == EOS) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid key/value",
                   Logger_S("key", key),
                   Logger_S("value", value));
        #endif

        return false;
    }

    if (strlen(key) > MAX_REPLACEMENT_KEY_LEN) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "key is too long",
                   Logger_S("key", key));
        #endif

        return false;
    }

    if (strlen(value) > MAX_REPLACEMENT_VALUE_LEN) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "value is too long",
                   Logger_S("value", value));
        #endif

        return false;
    }

    if (!map_valid(s_StringReplacements)) {
        s_StringReplacements = map_new();
    }

    if (map_has_str_key(s_StringReplacements, key)) {
        #if(defined _logger_included)
        Logger_Log("replacement key already exists",
                   Logger_S("key", key));
        #endif

        return false;
    }

    map_str_add_str(s_StringReplacements, key, value);

    return true;
}

stock Language:LangPlus_GetLanguageId(const string:language[]) {
    if (pool_size(s_LanguagePool) == 0) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "no languages have been initialized");
        #endif
        return INVALID_LANGUAGE_ID;
    }

    // Index of not existing languages is -1, so it equals INVALID_LANGUAGE_ID
    return Language:pool_find_str(s_LanguagePool, language);
}

stock LangPlus_GetLanguageList(string:languages[][]) {
    if (pool_size(s_LanguagePool) == 0) {
        Logger_Dbg("langplus", "no languages have been initialized");
        return 0;
    }

    new langName[MAX_LANGUAGE_NAME + 1],
        index;

    for_pool(it : s_LanguagePool) {
        iter_get_value_str(it, langName);
        strcopy(languages[index], langName, MAX_LANGUAGE_NAME);
        index++;
    }

    return pool_size(s_LanguagePool);
}

stock bool:LangPlus_GetLanguageName(Language:languageid, string:output[] = "", len = sizeof(output)) {
    if (!LangPlus_IsValidLanguage(languageid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id",
                   Logger_I("languageid", _:languageid));
        #endif

        return false;
    }

    pool_get_value_str(s_LanguagePool, _:languageid, output, len);

    return output[0] != '\0';
}

stock bool:LangPlus_GetLanguageString(Language:languageid, const string:key[], string:output[], len = sizeof(output)) {
    if (!LangPlus_IsValidLanguage(languageid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id",
                   Logger_I("languageid", _:languageid));
        #endif

        return false;
    }

    if (!map_valid(s_LanguageMap[languageid])) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "language map is invalid",
                   Logger_I("languageid", _:languageid));
        #endif

        if (languageid != Language:0) {
            return LangPlus_GetLanguageString(Language:0, key, output, len);
        }

        #if(defined _logger_included)
        Logger_Log("language key does not exist in the default language",
                   Logger_I("languageid", _:languageid),
                   Logger_S("key", key));
        #endif

        return false;
    }

    if (!map_has_str_key(s_LanguageMap[languageid], key)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "language key does not exist",
                   Logger_I("languageid", _:languageid),
                   Logger_S("key", key));
        #endif

        if (languageid != Language:0) {
            return LangPlus_GetLanguageString(Language:0, key, output, len);
        }

        #if(defined _logger_included)
        Logger_Log("language key does not exist in the default language",
                   Logger_I("languageid", _:languageid),
                   Logger_S("key", key));
        #endif

        return false;
    }

    map_str_get_str(s_LanguageMap[languageid], key, output, len);

    return true;
}

stock LangPlus_ReturnLanguageString(Language:languageid, const string:key[]) {
    new string:result[MAX_LANGUAGE_ENTRY_LENGTH + 1];
    LangPlus_GetLanguageString(languageid, key, result);

    return result;
}

stock bool:LangPlus_SetPlayerLanguage(playerid, Language:languageid) {
    if (!LangPlus_IsValidLanguage(languageid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id",
                   Logger_I("languageid", _:languageid));
        #endif

        return false;
    }

    if (!IsPlayerConnected(playerid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid player id",
                   Logger_I("playerid", playerid));
        #endif

        return false;
    }

    s_PlayerLanguage[playerid] = languageid;

    return true;
}

stock Language:LangPlus_GetPlayerLanguage(playerid) {
    if (!IsPlayerConnected(playerid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid player id",
                   Logger_I("playerid", playerid));
        #endif

        return INVALID_LANGUAGE_ID;
    }

    return s_PlayerLanguage[playerid];
}

stock bool:LangPlus_SetPlayerLanguageByName(playerid, const string:language[]) {
    return LangPlus_SetPlayerLanguage(playerid, LangPlus_GetLanguageId(language));
}

stock Language:LangPlus_GetPlayerLanguageName(playerid, string:output[] = "", len = sizeof(output)) {
    if (!IsPlayerConnected(playerid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid player id",
                   Logger_I("playerid", playerid));
        #endif

        return INVALID_LANGUAGE_ID;
    }

    LangPlus_GetLanguageName(s_PlayerLanguage[playerid], output);

    return s_PlayerLanguage[playerid];
}

#if defined _INC_y_va
#if defined LANGPLUS_REPLACE_NATIVES
LangPlus_SendClientMessage(playerid, colour, const format[], OPEN_MP_TAGS:...) {
    new Language:playerLanguage;
    playerLanguage = LangPlus_GetPlayerLanguage(playerid);

    if (playerLanguage == INVALID_LANGUAGE_ID) {
        return SendClientMessage(playerid, colour, format, ___(3));
    }

    new langMessage[144];

    new bool:ret;
    ret = LangPlus_GetLanguageString(playerLanguage, format, langMessage);

    if (ret) {
        SendClientMessage(playerid, colour, langMessage, ___(3));
        return true;
    }

    SendClientMessage(playerid, colour, format, ___(3));
    return true;
}

bool:LangPlus_SendClientMessageToAll(color, const string:msg[], OPEN_MP_TAGS:...) {
    foreach (new i : Player) {
        SendClientMessage(
            i,
            color,
            msg,
            ___(2)
        );
    }

    return true;
}
#else
stock bool:LangPlus_SendClientMessage(playerid, color, const string:msg[], OPEN_MP_TAGS:...) {
    return SendClientMessage(
               playerid,
               color,
               LangPlus_ReturnLanguageString(LangPlus_GetPlayerLanguage(playerid), msg),
               ___(3)
           );
}

stock bool:LangPlus_SendClientMessageToAll(color, const string:msg[], OPEN_MP_TAGS:...) {
    foreach (new i : Player) {
        SendClientMessage(
            i,
            color,
            LangPlus_ReturnLanguageString(LangPlus_GetPlayerLanguage(i), msg),
            ___(2)
        );
    }

    return true;
}
#endif
#endif

// ALS

#if defined _ALS_OnGameModeInit
    #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif
#define OnGameModeInit LangPlus_OnGameModeInit
#if defined LangPlus_OnGameModeInit
forward LangPlus_OnGameModeInit();
#endif

#if defined _ALS_OnPlayerConnect
    #undef OnPlayerConnect
#else
    #define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect LangPlus_OnPlayerConnect
#if defined LangPlus_OnPlayerConnect
forward LangPlus_OnPlayerConnect(playerid);
#endif

#if defined LANGPLUS_REPLACE_NATIVES
    #if defined _ALS_SendClientMessage
        #undef SendClientMessage
    #else
        #define _ALS_SendClientMessage
    #endif
    #define SendClientMessage LangPlus_SendClientMessage

    #define SendClientMessageToAll LangPlus_SendClientMessageToAll
#endif
