# LangPlus

[![sampctl](https://img.shields.io/badge/sampctl-LangPlus-2f2f2f.svg?style=for-the-badge)](https://github.com/mysy00/LangPlus)

Create multilanguage open.mp servers easily.

## Installation

Simply install to your project:

```bash
sampctl install mysy00/LangPlus
```

Include in your code and begin using the library:

```pawn
#include <LangPlus>
```

## Usage

main.pwn:
```pawn
new Language:g_LangEnglish;
new Language:g_LangUkrainian;

public OnGameModeInit() {
    g_LangEnglish = LangPlus_LoadLanguage("English");
    g_LangUkrainian = LangPlus_LoadLanguage("Ukrainian");

    return 1;
}

public OnPlayerConnect(playerid) {
  // Send a message with key "TEST_SCM"
  // 2137 is an integer formatted as %d to show parameters working
  LangPlus_SetPlayerLanguage(playerid, g_LangUkrainian);
  LangPlus_SendClientMessage(playerid, 0xFFFFFFFF, "TEST_SCM", 2137);

  return 1;
}
```

scriptfiles/languages/Ukrainian.ini:
```ini
TEST_SCM=Привіт Люба LangPlus_SendClientMessage: %d
```

## Configuration
Define your options before including the library.

```pawn
// max number of languages
#if !defined MAX_LANGUAGES
    #define MAX_LANGUAGES 4
#endif

// character used as delimiter
#if !defined DELIMITER_CHAR
    #define DELIMITER_CHAR "="
#endif

// directory languages are stored in
#if !defined DIRECTORY_LANGUAGES
	#define DIRECTORY_LANGUAGES "languages/"
#endif

// maximum length of a language key
#if !defined MAX_LANGUAGE_KEY_LEN
	#define MAX_LANGUAGE_KEY_LEN (32)
#endif

// maximum length of a language text entry
#if !defined MAX_LANGUAGE_ENTRY_LENGTH
	#define MAX_LANGUAGE_ENTRY_LENGTH (768)
#endif

// name limit for a language
#if !defined MAX_LANGUAGE_NAME
	#define MAX_LANGUAGE_NAME (32)
#endif

// max file name length for a language file
#if !defined MAX_FILE_NAME
    #define MAX_FILE_NAME (64)
#endif
```

## API
```pawn
// Define a replacement for a string found in the file
bool:LangPlus_SetStringReplacement(const string:key[], const string:value[])
// Example:
LangPlus_SetStringReplacement("{CE_WHITE}", "{FFFFFF}");
// NOTE: You have to use it before you load a language

// Load a language
Language:LangPlus_LoadLanguage(const string:langName[])
// Example:
new Language:g_LangEnglish = LangPlus_LoadLanguage("English");

// Get id of a langauge
Language:LangPlus_GetLanguageId(const string:language[])

// Returns the number of added languages and by using pass-by-reference you're getting an array with them
LangPlus_GetLanguageList(string:languages[][])
// Example:
new langList[MAX_LANGUAGES][MAX_LANGUAGES * MAX_LANGUAGE_NAME];
new langCount = LangPlus_GetLanguageList(langList);

printf("Available languages: %d", langCount);
for (new i = 0; i < langCount; i++) {
  printf("%s", langList[i]);
}

// Get name of a language by its ID
bool:LangPlus_GetLanguageName(Language:languageid, string:output[] = "", len = sizeof(output))

// Get translated string from a language
bool:LangPlus_GetLanguageString(Language:languageid, const string:key[], string:output[], len = sizeof(output))

// Same as above but return it
LangPlus_ReturnLanguageString(Language:languageid, const string:key[])

// Set player's language
bool:LangPlus_SetPlayerLanguage(playerid, Language:languageid)

// Get player's language (its ID)
bool:Language:LangPlus_GetPlayerLanguage(playerid)

// Returns player's language and gets the name of the language
Language:LangPlus_GetPlayerLanguageName(playerid, string:output[] = "", len = sizeof(output))

// Send a message to everyone in their language
bool:LangPlus_SendClientMessage(playerid, color, const string:msg[], OPEN_MP_TAGS:...)
```

## Testing

To test, simply run the package:

```bash
sampctl run
```

## Credits
- [ScavengeSurvive/language](https://github.com/ScavengeSurvive/language)
