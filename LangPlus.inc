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

#include <open.mp>
#include <sscanf2>
#include <PawnPlus>
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

#define INVALID_LANGUAGE_ID (Language:-1)

// A macro for easier migration from SS/language
#define @L(%0,%1) LangPlus_ReturnLanguageString(LangPlus_GetPlayerLanguage(%0), %1)

static Map:s_LanguageMap[Language:MAX_LANGUAGES],
       Pool:s_LanguagePool;

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

static Language:_InitLanguage(const langName[]) {
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

static _LangPlus_AddLanguageEntry(const Language:languageid, const key[], const val[]) {
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

stock Language:LangPlus_LoadLanguage(const langName[]) {
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
        val[MAX_LANGUAGE_ENTRY_LENGTH];

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

        _LangPlus_AddLanguageEntry(languageid, key, val);

        lineNumber++;
    }

    fclose(fHandler);

    return languageid;
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

stock bool:LangPlus_GetLanguageName(Language:languageid, output[] = "", len = sizeof(output)) {
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

stock bool:LangPlus_GetLanguageString(Language:languageid, const string:key[], output[], len = sizeof(output)) {
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

        return false;
    }

    if (!map_has_str_key(s_LanguageMap[languageid], key)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "language key does not exist",
                   Logger_I("languageid", _:languageid),
                   Logger_S("key", key));
        #endif

        if (_:languageid == 0) {
            return false;
        }

        languageid = Language:0;
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

stock Language:LangPlus_GetPlayerLanguageName(playerid, output[] = "", len = sizeof(output)) {
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

stock bool:LangPlus_SendClientMessage(playerid, color, const string:msg[], OPEN_MP_TAGS:...) {
    return SendClientMessage(
               playerid,
               color,
               LangPlus_ReturnLanguageString(LangPlus_GetPlayerLanguage(playerid), msg),
               getarg(3)
           );
}

stock bool:LangPlus_SendClientMessageToAll(color, const string:msg[], OPEN_MP_TAGS:...) {
    #if defined _Y_ITERATE_LOCAL_VERSION || defined _FOREACH_LOCAL_VERSION
    foreach (new i : Player) {
    #else
    for (new i = 0; i != MAX_PLAYERS; i++) {
        if (!IsPlayerConnected(i)) {
            continue;
        }
    #endif
        SendClientMessage(
            i,
            color,
            LangPlus_ReturnLanguageString(LangPlus_GetPlayerLanguage(i), msg),
            getarg(2)
        );
    }

    return true;
}

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