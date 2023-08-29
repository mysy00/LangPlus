#pragma dynamic 64000
// #pragma option -E

#define VOID_TAGS
#define STRONG_TAGS
#include <open.mp>
#include "LangPlus.inc"

main() {}

new Language:g_LangEnglish;
new Language:g_LangUkrainian;

public OnGameModeInit() {
    g_LangEnglish = LangPlus_LoadLanguage("English");
    g_LangUkrainian = LangPlus_LoadLanguage("Ukrainian");

    new langList[MAX_LANGUAGES][MAX_LANGUAGES * MAX_LANGUAGE_NAME];
    new langCount = LangPlus_GetLanguageList(langList);

    printf("Available languages: %d", langCount);
    for (new i = 0; i < langCount; i++) {
        printf("%s", langList[i]);
    }

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

    return 1;
}