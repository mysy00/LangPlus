#pragma dynamic 64000
// #pragma option -E

#define VOID_TAGS
#define STRONG_TAGS

// #define LANGPLUS_REPLACE_NATIVES

#include <open.mp>
#include "LangPlus.inc"

main() {}

new Language:g_LangEnglish;
new Language:g_LangUkrainian;

public OnGameModeInit() {
    Logger_ToggleDebug("langplus", true);

    LangPlus_SetStringReplacement("{CE_WHITE}", "{FFFFFF}");

    g_LangEnglish = LangPlus_LoadLanguage("English");
    g_LangUkrainian = LangPlus_LoadLanguage("Ukrainian");

    new langList[MAX_LANGUAGES][MAX_LANGUAGES * MAX_LANGUAGE_NAME];
    new langCount = LangPlus_GetLanguageList(langList);

    printf("Available languages: %d", langCount);
    for (new i = 0; i < langCount; i++) {
        printf("%s", langList[i]);
    }

    print("#########################################");
    print("## TEST: Replacements ##");
    print("#########################################");

    printf(LangPlus_ReturnLanguageString(g_LangEnglish, "TEST_REPLACEMENT"));

    print("#########################################");
    print("## TEST: Escape sequences ##");
    print("#########################################");
    new escapedString[MAX_LANGUAGE_ENTRY_LENGTH + 1];
    LangPlus_GetLanguageString(g_LangEnglish, "DIALOGMENU", escapedString);
    print("Option 1\nOption 2\nOption 3");
    print(escapedString);

    print("#########################################");
    print("## TEST: LangPlus_ReturnLanguageString ##");
    print("#########################################");

    print("[TEST] English language set");
    print(LangPlus_ReturnLanguageString(g_LangEnglish, "TEST_LANG"));
    printf(LangPlus_ReturnLanguageString(g_LangEnglish, "TEST_SCM"), 2137);
    print("[TEST] Ukrainian language set");
    print(LangPlus_ReturnLanguageString(g_LangUkrainian, "TEST_LANG"));
    printf(LangPlus_ReturnLanguageString(g_LangUkrainian, "TEST_SCM"), 2137);
    printf(LangPlus_ReturnLanguageString(g_LangUkrainian, "KEYONLYENGLISH"));

    print("########################################");
    print("## TEST: LangPlus_GetLanguageString ##");
    print("########################################");
    new referenceTest[MAX_LANGUAGE_ENTRY_LENGTH + 1];
    LangPlus_GetLanguageString(g_LangEnglish, "TEST_LANG", referenceTest);
    print(referenceTest);

    return 1;
}

public OnPlayerConnect(playerid) {
    SendClientMessage(playerid, -1, "[TEST] No language set");
    LangPlus_SendClientMessage(playerid, -1, "TEST_SCM", 2137);
    SendClientMessage(playerid, -1, "[TEST] Ukrainian language set");
    LangPlus_SetPlayerLanguage(playerid, g_LangUkrainian);
    LangPlus_SendClientMessage(playerid, -1, "TEST_SCM", 2137);

    LangPlus_SetPlayerLanguage(playerid, g_LangEnglish);
    LangPlus_SendClientMessage(playerid, -1, "TEST_SCM", 2137);
    LangPlus_SetPlayerLanguageByName(playerid, "Ukrainian");
    LangPlus_SendClientMessage(playerid, -1, "TEST_SCM", 2137);

    LangPlus_SendClientMessageToAll(-1, "TEST_SCM", 2137);

    // Hooked natives
    #if defined LANGPLUS_REPLACE_NATIVES
    LangPlus_SetPlayerLanguage(playerid, g_LangEnglish);
    SendClientMessage(playerid, -1, "[TEST] Hooked natives in English - SendClientMessage");
    SendClientMessage(playerid, -1, "regular message");
    SendClientMessage(playerid, -1, "TEST_SCM", 2137);
    SendClientMessage(playerid, -1, "TEST_SCM", 2137);
    SendClientMessage(playerid, -1, "[TEST] Hooked natives in English - SendClientMessageToAll");
    SendClientMessageToAll(-1, "regular message", 2137);
    SendClientMessageToAll(-1, "TEST_SCM", 2137);
    SendClientMessageToAll(-1, "TEST_SCM", 2137);

    LangPlus_SetPlayerLanguage(playerid, g_LangUkrainian);
    SendClientMessage(playerid, -1, "[TEST] Hooked natives in Ukrainian - SendClientMessage");
    SendClientMessage(playerid, -1, "regular message");
    SendClientMessage(playerid, -1, "TEST_SCM", 2137);
    SendClientMessage(playerid, -1, "TEST_SCM", 2137);
    SendClientMessage(playerid, -1, "[TEST] Hooked natives in Ukrainian - SendClientMessageToAll");
    SendClientMessageToAll(-1, "regular message", 2137);
    SendClientMessageToAll(-1, "TEST_SCM", 2137);
    SendClientMessageToAll(-1, "TEST_SCM", 2137);
    #endif

    return 1;
}