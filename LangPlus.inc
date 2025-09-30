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
    #define DefineLanguageReplacement SetStringReplacement
    #define InitLanguage LoadLanguage
    #define AddLanguageEntry AddLanguageEntry
    #define GetLanguageString GetLanguageString
    #define GetLanguageList GetLanguageList
    #define GetLanguageID GetLanguageId
    #define GetLanguageName GetLanguageName
    #define GetPlayerLanguage GetPlayerLanguage
    #define SetPlayerLanguage SetPlayerLanguage
    #define @L(%0,%1) ReturnLanguageString(GetPlayerLanguage(%0), %1)
#endif

static Map:s_LanguageMap[Language:MAX_LANGUAGES],
       Pool:s_LanguagePool,
       Map:s_StringReplacements,
       bool:s_LanguagePoolInitialized = false;

static Language:s_PlayerLanguage[MAX_PLAYERS];

public OnGameModeInit() {
    s_LanguagePool = pool_new(MAX_LANGUAGES);
    s_LanguagePoolInitialized = true;

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

static Language:_InitLanguage(const string:langName[]) {
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

static bool:IsValidLanguage(Language:id) {
    if (!s_LanguagePoolInitialized) {
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

    if (!map_valid(s_LanguageMap[id])) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "language hasn't been initialized ",
                   Logger_I("languageid", _:id));
        #endif

        return false;
    }

    return pool_has(s_LanguagePool, _:id);
}

AddLanguageEntry(const Language:languageid, const string:key[], const string:val[]) {
    if (!IsValidLanguage(languageid) || key[0] == EOS) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id or key",
                   Logger_I("languageid", _:languageid),
                   Logger_S("key", key));
        #endif
        return false;
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
Language:LoadLanguage(const string:langName[]) {
    if (langName[0] == EOS) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "language name is empty");
        #endif
        return INVALID_LANGUAGE_ID;
    }

    new Language:languageid = _InitLanguage(langName);
    if (languageid == INVALID_LANGUAGE_ID) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "unable to initialize language",
                   Logger_S("language", langName));
        #endif

        return INVALID_LANGUAGE_ID;
    }

    s_LanguageMap[languageid] = map_new();

    new fileName[MAX_FILE_NAME],
        File:fHandler,
        line[MAX_LANGUAGE_KEY_LEN + 1 + MAX_LANGUAGE_ENTRY_LENGTH],
        lineNumber,
        lineLength;

    format(fileName, sizeof(fileName), #DIRECTORY_LANGUAGES "%s.ini", langName);
    fHandler = fopen(fileName, io_read);

    if (!fHandler) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "unable to open file",
                   Logger_S("fileName", fileName));
        #endif

        map_delete(s_LanguageMap[languageid]);
        pool_remove(s_LanguagePool, _:languageid);
        return INVALID_LANGUAGE_ID;
    }

    new key[MAX_LANGUAGE_KEY_LEN],
        val[MAX_LANGUAGE_ENTRY_LENGTH],
        valFormatted[MAX_LANGUAGE_ENTRY_LENGTH];

    while (fread(fHandler, line)) {
        lineNumber++;
        lineLength = strlen(line);

        if (lineLength < 3) {
            // Minimum length of a line is 3 characters: "a=b"
            continue;
        }

        if (
            (line[0] < 'a' || line[0] > 'z')
            && (line[0] < 'A' || line[0] > 'Z')
            && (line[0] < '0' || line[0] > '9')
        ) {
            // Skip lines that don't start with a letter or digit (most likely comments)
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

            fclose(fHandler);
            return INVALID_LANGUAGE_ID;
        }

        strtrim(key);
        strtrim(val);

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

        if (!AddLanguageEntry(languageid, key, valFormatted)) {
            #if(defined _logger_included)
            Logger_Dbg("langplus", "failed to add language entry",
                       Logger_I("line", lineNumber),
                       Logger_S("file", fileName),
                       Logger_S("key", key));
            #endif

            fclose(fHandler);
            map_delete(s_LanguageMap[languageid]);
            pool_remove(s_LanguagePool, _:languageid);
            return INVALID_LANGUAGE_ID;
        }
    }

    fclose(fHandler);

    return languageid;
}

stock bool:SetStringReplacement(const string:key[], const string:value[]) {
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

stock GetLanguageCount() {
    if (!s_LanguagePoolInitialized) {
        return 0;
    }

    return pool_size(s_LanguagePool);
}

stock bool:HasLanguage(const string:language[]) {
    if (!s_LanguagePoolInitialized || language[0] == EOS) {
        return false;
    }

    return pool_find_str(s_LanguagePool, language) != -1;
}

stock Language:GetLanguageId(const string:language[]) {
    if (!s_LanguagePoolInitialized) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "no languages have been initialized");
        #endif
        return INVALID_LANGUAGE_ID;
    }

    // Index of not existing languages is -1, so it equals INVALID_LANGUAGE_ID
    return Language:pool_find_str(s_LanguagePool, language);
}

stock GetLanguageList(string:languages[][], maxSize = sizeof(languages)) {
    if (!s_LanguagePoolInitialized) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "no languages have been initialized");
        #endif
        return 0;
    }

    new index,
        poolSize = pool_size(s_LanguagePool),
        limit = (poolSize < maxSize) ? poolSize : maxSize;

    for_pool(it : s_LanguagePool) {
        if (index >= limit) {
            break;
        }
        iter_get_value_str(it, languages[index], MAX_LANGUAGE_NAME);
        index++;
    }

    return index;
}

stock bool:GetLanguageName(Language:languageid, string:output[] = "", len = sizeof(output)) {
    if (!IsValidLanguage(languageid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id",
                   Logger_I("languageid", _:languageid));
        #endif

        return false;
    }

    pool_get_value_str(s_LanguagePool, _:languageid, output, len);

    return output[0] != '\0';
}

stock bool:GetLanguageString(Language:languageid, const string:key[], string:output[], len = sizeof(output)) {
    if (!IsValidLanguage(languageid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid language id",
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

        if (languageid != Language:0 && IsValidLanguage(Language:0)) {
            return GetLanguageString(Language:0, key, output, len);
        }

        #if(defined _logger_included)
        Logger_Log("language key does not exist in the default language",
                   Logger_I("languageid", _:languageid),
                   Logger_S("key", key));
        #endif

        strcopy(output, key, len);

        return false;
    }

    map_str_get_str(s_LanguageMap[languageid], key, output, len);

    return true;
}

stock ReturnLanguageString(Language:languageid, const string:key[]) {
    new string:result[MAX_LANGUAGE_ENTRY_LENGTH + 1];
    GetLanguageString(languageid, key, result);

    return result;
}

stock bool:SetPlayerLanguage(playerid, Language:languageid) {
    if (!IsValidLanguage(languageid)) {
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

stock Language:GetPlayerLanguage(playerid) {
    if (!IsPlayerConnected(playerid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid player id",
                   Logger_I("playerid", playerid));
        #endif

        return INVALID_LANGUAGE_ID;
    }

    return s_PlayerLanguage[playerid];
}

stock bool:SetPlayerLanguageByName(playerid, const string:language[]) {
    return SetPlayerLanguage(playerid, GetLanguageId(language));
}

stock Language:GetPlayerLanguageName(playerid, string:output[] = "", len = sizeof(output)) {
    if (!IsPlayerConnected(playerid)) {
        #if(defined _logger_included)
        Logger_Dbg("langplus", "invalid player id",
                   Logger_I("playerid", playerid));
        #endif

        return INVALID_LANGUAGE_ID;
    }

    GetLanguageName(s_PlayerLanguage[playerid], output);

    return s_PlayerLanguage[playerid];
}

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
