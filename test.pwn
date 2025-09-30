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

    SetStringReplacement("{CE_WHITE}", "{FFFFFF}");

    g_LangEnglish = LoadLanguage("en", "English");
    g_LangUkrainian = LoadLanguage("uk", "Ukrainian");

    print("#########################################");
    print("## TEST: GetLanguageCount & HasLanguage ##");
    print("#########################################");

    printf("Total languages loaded: %d", GetLanguageCount());
    printf("Has 'en': %d", HasLanguage("en"));
    printf("Has 'uk': %d", HasLanguage("uk"));
    printf("Has 'fr': %d", HasLanguage("fr"));

    print("#########################################");
    print("## TEST: GetLanguageList ##");
    print("#########################################");

    new langList[MAX_LANGUAGES][MAX_LANGUAGE_NAME];
    new langCount = GetLanguageList(langList);

    printf("Available languages: %d", langCount);
    for (new i = 0; i < langCount; i++) {
        printf("%s", langList[i]);
    }

    print("#########################################");
    print("## TEST: Replacements ##");
    print("#########################################");

    printf(ReturnLanguageString(g_LangEnglish, "TEST_REPLACEMENT"));

    print("#########################################");
    print("## TEST: Escape sequences ##");
    print("#########################################");
    new escapedString[MAX_LANGUAGE_ENTRY_LENGTH + 1];
    GetLanguageString(g_LangEnglish, "DIALOGMENU", escapedString);
    print("Option 1\nOption 2\nOption 3");
    print(escapedString);

    print("#########################################");
    print("## TEST: ReturnLanguageString ##");
    print("#########################################");

    print("[TEST] English language set");
    print(ReturnLanguageString(g_LangEnglish, "TEST_LANG"));
    printf(ReturnLanguageString(g_LangEnglish, "TEST_SCM"), 2137, 69, 420);
    print("[TEST] Ukrainian language set");
    print(ReturnLanguageString(g_LangUkrainian, "TEST_LANG"));
    printf(ReturnLanguageString(g_LangUkrainian, "TEST_SCM"), 2137, 69, 420);
    printf(ReturnLanguageString(g_LangUkrainian, "KEYONLYENGLISH"));

    print("########################################");
    print("## TEST: GetLanguageString ##");
    print("########################################");
    new referenceTest[MAX_LANGUAGE_ENTRY_LENGTH + 1];
    GetLanguageString(g_LangEnglish, "TEST_LANG", referenceTest);
    print(referenceTest);

    return 1;
}

public OnPlayerConnect(playerid) {
    print("#########################################");
    print("## TEST: SendLanguageMessage ##");
    print("#########################################");

    // Test with English
    SetPlayerLanguage(playerid, g_LangEnglish);
    SendLanguageMessage(playerid, -1, "TEST_LANG");
    SendLanguageMessage(playerid, 0xFF0000FF, "TEST_SCM", 2137, 69, 420);

    // Test with Ukrainian
    SetPlayerLanguage(playerid, g_LangUkrainian);
    SendLanguageMessage(playerid, -1, "TEST_LANG");
    SendLanguageMessage(playerid, 0x00FF00FF, "TEST_SCM", 2137, 69, 420);

    // Test fallback (key only in English)
    SendLanguageMessage(playerid, 0xFFFF00FF, "KEYONLYENGLISH");

    print("#########################################");
    print("## TEST: SendLanguageMessageToAll ##");
    print("#########################################");

    // Send to all - each player will receive message in their language
    SendLanguageMessageToAll(-1, "TEST_LANG");
    SendLanguageMessageToAll(0xFFFFFFFF, "TEST_SCM", 2137, 69, 420);

    return 1;
}