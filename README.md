# LangPlus

[![sampctl](https://img.shields.io/badge/sampctl-LangPlus-2f2f2f.svg?style=for-the-badge)](https://github.com/mysy00/LangPlus)

LangPlus is a lightweight and powerful library for open.mp that allows you to easily create multilanguage servers. It uses simple INI files for language strings and provides a flexible API to manage languages for players.

## Installation

Simply install to your project:

```bash
sampctl package install mysy00/LangPlus
```

Include in your code and begin using the library:

```pawn
#include <LangPlus>
```

## Quick Start

Here is a minimal example to get you started.

**`gamemodes/main.pwn`**
```pawn
#include <LangPlus>

new Language:g_LangEnglish;
new Language:g_LangUkrainian;

public OnGameModeInit() {
    // Load languages from scriptfiles/languages/
    g_LangEnglish = LoadLanguage("English");
    g_LangUkrainian = LoadLanguage("Ukrainian");
    return 1;
}

public OnPlayerConnect(playerid) {
    // Set player's language to Ukrainian
    SetPlayerLanguage(playerid, g_LangUkrainian);

    // Send a localized message to the player
    new msg[128];
    GetLanguageString(GetPlayerLanguage(playerid), "WELCOME_MESSAGE", msg, sizeof(msg), playerid);
    SendClientMessage(playerid, 0xFFFFFFFF, msg);
    return 1;
}
```

**`scriptfiles/languages/English.ini`**
```ini
WELCOME_MESSAGE=Welcome to the server, player %d!
```

**`scriptfiles/languages/Ukrainian.ini`**
```ini
WELCOME_MESSAGE=Вітаємо на сервері, гравець %d!
```

## API Reference

### Core Functions

| Function | Description |
|---|---|
| `Language:LoadLanguage(const langName[])` | Loads a language from a `.ini` file. Returns the language ID. |
| `bool:SetStringReplacement(const key[], const value[])` | Defines a string replacement to be used when loading languages. Must be called **before** `LoadLanguage`. |
| `Language:GetLanguageId(const language[])` | Gets the ID of a loaded language by its name. |
| `GetLanguageList(string:languages[][])` | Fills a 2D array with the names of all loaded languages and returns the count. |
| `bool:GetLanguageName(Language:id, name[], len)` | Gets the name of a language by its ID. |

### String Functions

| Function | Description |
|---|---|
| `bool:GetLanguageString(Language:id, const key[], output[], len, ...)` | Gets a translated string for a given language ID and key. Supports format specifiers. |
| `ReturnLanguageString(Language:id, const key[], ...)` | Returns a translated string. **Note:** The returned string is temporary and should be used immediately. |

### Player Functions

| Function | Description |
|---|---|
| `bool:SetPlayerLanguage(playerid, Language:id)` | Sets the language for a player. |
| `Language:GetPlayerLanguage(playerid)` | Gets the current language ID for a player. |
| `bool:SetPlayerLanguageByName(playerid, const language[])` | Sets the language for a player by language name. |
| `Language:GetPlayerLanguageName(playerid, name[], len)` | Gets the name of the player's current language. |

## Configuration

Define these options **before** including the library to override the default values.

| Macro | Default | Description |
|---|---|---|
| `MAX_LANGUAGES` | `4` | The maximum number of languages that can be loaded. |
| `DELIMITER_CHAR` | `"="` | The character used to separate keys and values in the `.ini` files. |
| `DIRECTORY_LANGUAGES` | `"languages/"` | The directory where language files are stored. |
| `MAX_LANGUAGE_KEY_LEN` | `32` | The maximum length of a language key. |
| `MAX_LANGUAGE_ENTRY_LENGTH` | `768` | The maximum length of a language string. |
| `MAX_LANGUAGE_NAME` | `32` | The maximum length of a language name. |
| `MAX_FILE_NAME` | `64` | The maximum length of a language file name. |
| `MAX_REPLACEMENT_KEY_LEN` | `16` | The maximum length of a replacement key. |
| `MAX_REPLACEMENT_VALUE_LEN` | `16` | The maximum length of a replacement value. |

## Integrations

### tdialogs

Here is an example of how to use LangPlus with [tdialogs](https://github.com/TommyB123/tdialogs) to create a language selection dialog:

**Code:**
```pawn
#define PP_SYNTAX_AWAIT
#define PP_SYNTAX_YIELD
#include <LangPlus>
#include <tdialogs>

ShowPlayerLanguageDialog(playerid) {
    yield 1;

    new dialog_string[256];
    new header_string[64];
    new button_string[32];

    GetLanguageString(GetPlayerLanguage(playerid), "DIALOG_LANG_TITLE", header_string, sizeof(header_string));
    GetLanguageString(GetPlayerLanguage(playerid), "DIALOG_LANG_SELECT", button_string, sizeof(button_string));

    // Build the list of languages
    new langList[MAX_LANGUAGES][MAX_LANGUAGE_NAME];
    new langCount = GetLanguageList(langList);
    for (new i = 0; i < langCount; i++) {
        strcat(dialog_string, langList[i]);
        strcat(dialog_string, "\n");
    }

    // Show async dialog and await response
    new listitem = await ShowAsyncListitemIndexDialog(playerid, header_string, dialog_string, button_string, "Cancel");

    if (listitem != -1) {
        SetPlayerLanguageByName(playerid, langList[listitem]);
        SendClientMessage(playerid, -1, ReturnLanguageString(GetPlayerLanguage(playerid), "LANGUAGE_CHANGED"));
    }
}
```

**Language File (`English.ini`):**
```ini
DIALOG_LANG_TITLE=Select Language
DIALOG_LANG_SELECT=Select
LANGUAGE_CHANGED=Language changed successfully.
```

## Testing

To test, simply run the package:

```bash
sampctl run
```

## Credits

- [ScavengeSurvive/language](https://github.com/ScavengeSurvive/language)
