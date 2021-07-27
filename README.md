# DS3Switcher

DS3 Switcher An application by echo778 on Nexus Mods

## Description

### THIS TOOL ONLY WORKS WITH MODS THAT USE dinput8.dll/Mod Engine.

This application mods and unmods Dark Souls 3 based on the Steam profile you are currently logged into. The program exports a Registry Key created by Steam on login, and extracts the ActiveUser dword to get the SteamID3. By comparing this ID to a list of registered IDs, it moves the dinput8.dll and modengine.ini files accordingly. List of registered IDs is kept in switcherIDs.txt. The backup dinput8.dll and modengine.ini files are kept in a folder named after the Steam ID they are tied to.

## Usage

Simply make a shortcut to the .exe, and launch that instead of your normal DARK SOULS III shortcut. If everything is set up correctly, a console window will flash on screen briefly, before starting Dark Souls 3.

Config will launch on first use of the DS3Switcher.exe. To edit configuration after first launch, DO NOT edit "switcher.config", instead launch config.bat. Alternatively, launch DS3Switcher.exe with the argument "config".

The program has 5 configuration options: DS3 Path, DS3 Executable, Arguments, Mod Toggle, and Save Configuration.
* DS3 Path is the path to your Dark Souls 3 Game folder.
* DS3 Executable is to the application you wish to be launched after using DS3Switcher.exe.
* Arguments is to set the run arguments on the application launched, which is set in DS3 Executable.
* Mod Toggle will toggle the status of the current ID between modded and unmodded.
* Save Configuration will backup the dinput8.dll and modengine.ini that is currently in your DS3 folder.

## Installation

1. Make sure that Dark Souls III is modded (there is a dinput8.dll and modengine.ini in the Dark Souls Game folder).
2. Launch DS3Switcher.exe in a dedicated directory/folder.
3. The program will create 3 files in the directory, DO NOT EDIT THEM.
4. Choose whether or not you'd like the current Steam account to be modded or not.
5. Config will launch, type an option and hit enter to set it.
6. You must set the path to your Dark Souls 3 game directory.
7. DO NOT edit DS3 Executable or Arguments unless you know what you're doing.
8. Save and Exit.
9. Launch DS3Switcher.exe again.
10. Finally, save the current configuration if you want the current mods to be tied to the current Steam ID.
11. Now, simply launch DS3Switcher.exe anytime you want to play Dark Souls 3.
