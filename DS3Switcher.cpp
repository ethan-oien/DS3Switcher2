#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <list>
#include <stdio.h>
#include <direct.h>
#include <conio.h>

/***********************************
====================================
  AUTHOR: ECHO778
  PROGRAM: DS3SWITCHER
====================================
***********************************/

//====================
//  GLOBAL VARIABLES
//====================

//gets current working directory; lifted from stack overflow
std::wstring ExePath() {
    wchar_t buffer[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    buffer;
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos + 1);
}

std::wstring path = ExePath(); //GET CURRENT DIRECTORY
std::wstring steamID64;
std::wstring idPath;

//====================
//  CONFIG VARIABLES
//====================

std::wstring conFile = path + L"switcher.config";
BOOL config = 0;
std::wstring ds3Path = L" ";
std::wstring ds3EXE = L" ";
std::wstring arguments = L" ";

//=======================
//  FUNCTION PROTOTYPES
//=======================

//MAIN FUNCTIONS
int main(int argc, char* argv[]);
void StartDS3();
void ReadGlobals();
void Pause();

//IDLIST VARIABLES
std::list<std::wstring> ids;
std::list<wchar_t> modded;
std::list<std::wstring> idList;
std::wstring file = path + L"switcherIDs.txt";
//IDLIST FUNCTIONS
bool UserInList();
void NewUser();
void ParseID();
void InitList();
bool IDInList(std::wstring id);
bool IDIsModded(std::wstring id);
void UnmodID(std::wstring id);
void ModID(std::wstring id);

//DLL VARIABLES
std::wstring dll = L"dinput8.dll";
std::wstring ini = L"modengine.ini";
//DLL FUNCTIONS
bool ConfigExists(std::wstring id);
int SaveCurConfig();
int LoadCurConfig();
int RemoveFiles();
bool INIInDS3();
bool DLLInDS3();

//STEAM FUNCTIONS
long long SteamID3toSteamID64(long steamid3);
std::wstring FetchSteamID64();
long FetchSteamID3();

//CONFIG FUNCTIONS
void Menu();
void RewriteGlobals();
void FolderChange();
void EXEChange();
void ArgumentChange();
void ModdedToggle(std::wstring id);

//==================
//  MAIN FUNCTIONS
//==================

//main
int main(int argc, char* argv[])
{
    ReadGlobals();
    steamID64 = FetchSteamID64();
    idPath = path + steamID64 + L"\\";
    if (steamID64 == std::to_wstring(76561197960265728))
    {
        std::cout << "Not signed into Steam! Please sign in.\n";
        Pause();
        exit(1);
    }
        
    InitList();

    if (argc > 2)
    {
        std::cout << "Only 1 argument accepted. Use \"config\" to enter config mode.\n";
        Pause();
        exit(1);
    }
    else if (argc == 2)
    {
        if (std::string(argv[1]) == "config")
            Menu();
    }

    if (!config)
        Menu();

    if (IDIsModded(steamID64))
    {
        if (!ConfigExists(steamID64))
        {
            std::cout << "No configuration saved for the current ID!" << std::endl;
            if (SaveCurConfig() == 1)
            {
                std::cout << "Please make sure the correct mods for the current ID are installed." << std::endl;
                std::cout << "Additionally, make sure modengine.ini is set up as desired." << std::endl;
                Pause();
                exit(0);
            }
            Pause();
        }

        if (LoadCurConfig() == 0)
        {
            StartDS3();
            return 0;
        }
        else
        {
            std::cout << "Unable to insert DLL into Dark Souls 3 folder, game not started.\n";
            Pause();
            exit(1);
        }
    }
    else
    {
        if (RemoveFiles() == 0)
        {
            StartDS3();
            return 0;
        }
        else
        {
            std::cout << "Unable to remove DLL from Dark Souls 3 folder, game not started.\n";
            Pause();
            exit(1);
        }

    }

    return 0;
}

//starts ds3 with specified arguments
void StartDS3()
{
    int e;

    LPCWSTR ProcessName = (ds3EXE).c_str();
    LPWSTR Arguments = const_cast<wchar_t*>(arguments.c_str());
    STARTUPINFO si = { sizeof(STARTUPINFO), 0 };
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = { 0 };

    if (arguments == L" ")
        e = CreateProcessW(
            ProcessName,
            NULL,
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_PROCESS_GROUP,
            NULL,
            NULL,
            &si,
            &pi
        );
    else
        e = CreateProcessW(
            ProcessName,
            Arguments,
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_PROCESS_GROUP,
            NULL,
            NULL,
            &si,
            &pi
        );
    if (pi.hThread)
        CloseHandle(pi.hThread);
    if (pi.hProcess)
        CloseHandle(pi.hProcess);
    if (e == 0)
    {
        std::cout << GetLastError() << std::endl;
        Pause();
        exit(1);
    }
}

//checks if directory exists; lifted from StackOverflow
BOOL DirectoryExists(std::wstring tempPath)
{
    LPCWSTR szPath = tempPath.c_str();

    DWORD dwAttrib = GetFileAttributesW(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int FindNLast(std::wstring buffer, std::wstring str, int N)
{
    int index = -1;

    for (int i = 0; i < N; i++)
    {
        if (buffer.find(str) != -1)
        {
            index = buffer.find_last_of(str);
            buffer = buffer.erase(index, 1);
        }
        else
            return -1;
    }

    return index;
}

//creates a directory tree to current folder
void MakeDirectory(std::wstring path)
{
    std::wstring bfpath = path.substr(0, FindNLast(path, (L"\\"), 2)+1);
    if (DirectoryExists(bfpath))
    {
        _wmkdir(path.c_str());
        return;
    }
    
    MakeDirectory(bfpath);

    return;
}

//called to read global variables from config file
void ReadGlobals()
{
    //std::list<std::wstring> lines;

    FILE* gfile;

    if(_wfopen_s(&gfile, conFile.c_str(), L"r") != 0)
    {
        _wfopen_s(&gfile, conFile.c_str(), L"a");
        _wfreopen_s(&gfile, conFile.c_str(), L"r", gfile);
    }

    if (!gfile)
    {
        std::cout << "Can't access config file!\n";
        Pause();
        exit(1);
    }

    wchar_t line[MAX_PATH];
    std::wstring field;
    std::wstring value;

    while (fgetws(line, MAX_PATH, gfile))
    {
        if (line[0] != '\n')
        {
            if (std::wstring(line).find('=') != -1)
            {
                field = std::wstring(line).substr(0, std::wstring(line).find('='));
                value = std::wstring(line).substr(std::wstring(line).find('=') + 1, std::wstring(line).length() - 1);
                if (value.find('\n') != -1)
                    value.erase(value.find('\n'), 1);

                if (field == L"config")
                {
                    if (value == L"true")
                        value = L"1";
                    else if (value == L"false")
                        value = L"0";

                    config = std::stoi(value);
                }
                    
                else if (field == L"ds3Path")
                    ds3Path = value;
                else if (field == L"ds3EXE")
                    ds3EXE = value;
                else if (field == L"arguments")
                    arguments = value;
            }
            else
            {
                std::cout << "switcher.config is in wrong format! Was it edited externally?\n";
                Pause();
                exit(1);
            }
        }
    }

    fclose(gfile);
}

void Pause()
{
    std::cout << "Press any key to continue . . . ";
    _getch();
    std::cout << std::endl;
}

//====================
//  IDLIST FUNCTIONS
//====================

//writes new user to text file
bool UserInList()
{
    if (!ids.empty())
    {
        for(std::list<std::wstring>::iterator it=ids.begin(); it != ids.end(); it++)
        {
            if (*it == steamID64)
                return true;
        }
    }
    return false;
}

//creates a new user in the text file with their choice of modded or unmodded DS3
void NewUser()
{
    wchar_t modChoice;
    wchar_t sure;
    std::wstring newRecord;

    do
    {
        std::wcout << "Steam User \"" << steamID64 << "\" is not registered.\nIs this account modded or unmodded?" << std::endl;
        do
        {
            std::cout << "Please make your selection using the m(odded) or u(nmodded) key." << std::endl;
            modChoice = getchar();
            while (getchar() != '\n');
        } while (modChoice != 'm' && modChoice != 'u');

        if (modChoice == 'm')
        {
            std::cout << "You have chosen MODDED. Are you sure?" << std::endl;
        }
        else if (modChoice == 'u')
        {
            std::cout << "You have chosen UNMODDED. Are you sure? " << std::endl;
        }
        do
        {
            std::cout << "Please respond with the y(es) or n(o) key." << std::endl;
            sure = getchar();
            while (getchar() != '\n');
        } while (sure != 'y' && sure != 'n');
    } while (sure == 'n');

    newRecord = modChoice + steamID64;

    if (modChoice == 'm')
    {
        std::wcout << "User " << steamID64 << " added as a MODDED user." << std::endl;
    }
    else if (modChoice == 'u')
    {
        std::wcout << "User " << steamID64 << " added as a UNMODDED user." << std::endl;
    }

    idList.push_back(newRecord);

    FILE* fp;
    _wfopen_s(&fp, file.c_str(), L"a");

    fwprintf(fp, L"%s\n", newRecord.c_str());

    fclose(fp);

    ParseID();
}

//reads ids text file into modded and ids lists
void ParseID()
{
    FILE* fp;
    _wfopen_s(&fp, file.c_str(), L"r");
    wchar_t line[19];
    std::wstring sFile = path + L"switcher.config";

    if (!fp)
    {
        _wfopen_s(&fp, sFile.c_str(), L"a");
        fclose(fp);
        _wfopen_s(&fp, sFile.c_str(), L"r");
    }

    if (!fp)
    {
        std::cout << "Can't access config file!\n";
        Pause();
        exit(1);
    }

    idList.clear();

    while (fgetws(line, 19, fp))
    {
        if(line[0] != '\n')
            idList.push_back(std::wstring(line));
    }

    modded.clear();
    ids.clear();

    for (std::list<std::wstring>::iterator it = idList.begin(); it != idList.end(); it++)
    {
        modded.push_back((*it)[0]);
        ids.push_back(it->substr(1, 17)); //stol converts wstring to long
    }

    fclose(fp);
}

//initializes the list, called once in main
void InitList()
{
    ParseID();
    if (!UserInList())
    {
        NewUser();
    }
}

//returns -1 if id doesn't exist, otherwise returns id index
bool IDInList(std::wstring id)
{
    for (std::list<std::wstring>::iterator it = ids.begin(); it != ids.end(); it++)
    {
        if (*it == id)
            return true;
    }

    return false;
}

//returns true or false, depending on if the id is marked as modded or not
bool IDIsModded(std::wstring id)
{
    if (!IDInList(id))
    {
        std::cout << "ID does not exist in list.\n";
        Pause();
        exit(1);
    }

    std::list<std::wstring>::iterator iit = ids.begin();
    for (std::list<wchar_t>::iterator it = modded.begin(); it != modded.end(); it++)
    {
        if (*iit == id)
        {
            if (*it == 'm')
                return true;
        }
        iit++;
    }
    return false;
}

//changes passed ID to unmodded
void UnmodID(std::wstring id)
{
    std::list<wchar_t>::iterator mit = modded.begin();
    std::list<std::wstring>::iterator lit = idList.begin();
    for (std::list<std::wstring>::iterator iit = ids.begin(); iit != ids.end(); iit++)
    {
        if (*iit == id)
        {
            *mit = 'u';
            *lit = *mit + *iit;
        }
        mit++;
        lit++;
    }

    FILE* fp;
    _wfopen_s(&fp, file.c_str(), L"w");
    for (std::list<std::wstring>::iterator lit = idList.begin(); lit != idList.end(); lit++)
    {
        fwprintf(fp, L"%s\n", lit->c_str());
    }

    fclose(fp);

    ParseID();
}

//changes passed ID to modded
void ModID(std::wstring id)
{
    std::list<wchar_t>::iterator mit = modded.begin();
    std::list<std::wstring>::iterator lit = idList.begin();
    for (std::list<std::wstring>::iterator iit = ids.begin(); iit != ids.end(); iit++)
    {
        if (*iit == id)
        {
            *mit = 'm';
            *lit = *mit + *iit;
        }
        mit++;
        lit++;
    }

    FILE* fp;
    _wfopen_s(&fp, file.c_str(), L"w");
    for (std::list<std::wstring>::iterator lit = idList.begin(); lit != idList.end(); lit++)
    {
        fwprintf(fp, L"%s\n", lit->c_str());
    }

    fclose(fp);

    ParseID();
}

//=================
//  DLL FUNCTIONS
//=================

//checks for an existing configuration in the id's folder
bool ConfigExists(std::wstring id)
{
    if(!DirectoryExists(idPath))
        MakeDirectory(idPath);

    FILE* fd;
    _wfopen_s(&fd, (idPath + dll).c_str(), L"r");
    FILE* fi;
    _wfopen_s(&fi, (idPath + ini).c_str(), L"r");

    if (!fd || !fi)
    {
        if (fd)
            fclose(fd);
        else if (fi)
            fclose(fi);
        return false;
    }
    else
    {
        fclose(fd);
        fclose(fi);
        return true;
    }
}

//saves the current configuration in the dark souls folder
int SaveCurConfig()
{
    wchar_t sure;

    std::cout << "Would you like to save the current configuration to this ID?" << std::endl;
    std::cout << "(The dinput8.dll and modengine.ini in your Dark Souls 3 directory will be saved)" << std::endl;
    do
    {
        std::cout << "Please indicate your response with the y(es) or n(o) key." << std::endl;
        sure = getchar();
        while (getchar() != '\n');
    } while (sure != 'y' && sure != 'n');

    if (sure == 'n')
    {
        std::cout << "Current configuration not saved." << std::endl;
        return 1;
    }

    try
    {
        _wremove((idPath + dll).c_str());
        _wremove((idPath + ini).c_str());

        std::ifstream src(ds3Path + dll, std::ios::binary);
        std::ofstream dst(idPath + dll, std::ios::binary);
        dst << src.rdbuf();

        std::ifstream  src2(ds3Path + ini, std::ios::binary);
        std::ofstream  dst2(idPath + ini, std::ios::binary);
        dst2 << src2.rdbuf();
    }
    catch (const std::exception&)
    {
        std::cout << "Unable to save config! Are there any mods installed?\n";
        Pause();
        exit(1);
    }

    std::wcout << "\nCurrent configuration saved to id " << steamID64 << std::endl;
    std::cout << "(Start DS3Switcher in config mode to overwrite current configuration)" << std::endl;
    return 0;
}

//loads the current configuration saved in the id's folder
int LoadCurConfig()
{
    if (RemoveFiles() == 0)
    {
        try
        {
            std::ifstream  src((idPath + dll), std::ios::binary);
            std::ofstream  dst((ds3Path + dll), std::ios::binary);
            dst << src.rdbuf();
        }
        catch (const std::exception&)
        {
            std::cout << "dinput8.dll not found! Are there any mods installed?\n";
            Pause();
            exit(1);
        }

        try
        {
            std::ifstream  src((idPath + ini), std::ios::binary);
            std::ofstream  dst((ds3Path + ini), std::ios::binary);
            dst << src.rdbuf();
        }
        catch (const std::exception&)
        {
            std::cout << "modengine.ini not found! Are there any mods installed?\n";
            Pause();
            exit(1);
        }

        if (DLLInDS3() && INIInDS3())
            return 0;
        else
            return 1;
    }
    else
        return 1;
}

//removes both files from the dark souls folder
int RemoveFiles()
{
    if (DLLInDS3())
        _wremove((ds3Path + dll).c_str());

    if (INIInDS3())
        _wremove((ds3Path + ini).c_str());

    if (!DLLInDS3() && !INIInDS3())
        return 0;
    else
        return 1;
}

//checks for modengine.ini in specified dark souls folder
bool INIInDS3()
{
    FILE* fp;
    _wfopen_s(&fp, (ds3Path + ini).c_str(), L"r");

    if (!fp)
    {
        return false;
    }
    else
    {
        fclose(fp);
        return true;
    }
}

//checks for dinput8.dll in specified dark souls folder
bool DLLInDS3()
{
    FILE* fp;
    _wfopen_s(&fp, (ds3Path + dll).c_str(), L"r");

    if (!fp)
    {
        return false;
    }
    else
    {
        fclose(fp);
        return true;
    }
}

//===================
//  STEAM FUNCTIONS
//===================

//converts steamid3 to steamid64
long long SteamID3toSteamID64(long steamid3)
{
    std::wstring id = L"7656119" + std::to_wstring(steamid3 + 7960265728);
    return std::stoll(id); //stol converts wstring to long
}

//fetches steamid64
std::wstring FetchSteamID64()
{
    return std::to_wstring(SteamID3toSteamID64(FetchSteamID3()));
}

//fetches steamid3
long FetchSteamID3()
{
    std::wstring regFile = path + L"steamsession.txt";
    std::wstring regExpCmd = L"\"reg.exe export \"HKCU\\Software\\Valve\\Steam\\ActiveProcess\" \"" + regFile + L"\"\"";
    std::list<std::wstring> temp;
    long steamID3 = -1;
    wchar_t line[500];

    _wremove(regFile.c_str());

    _wsystem((regExpCmd.c_str()));
    std::cout << std::endl;

    try
    {
        FILE* fp;
        _wfopen_s(&fp, (regFile).c_str(), L"r, ccs=UNICODE");
        while (fgetws(line, 500, fp))
        {
            temp.push_back(std::wstring(line));
        }

        fclose(fp);
    }
    catch (const std::exception&)
    {
        std::cout << "Registry export failed!\n";
        Pause();
        exit(1);
    }

    for (std::list<std::wstring>::iterator it = temp.begin(); it != temp.end(); it++)
    {
        if (it->find(L"ActiveUser") != -1)
        {
            std::wstring i = *it;
            i = i.substr(i.find(':') + 1, 8);
            steamID3 = std::wcstol(i.c_str(), NULL, 16); //stol = wstring to long
        }
    }

    _wremove(regFile.c_str());

    return steamID3;
}

//====================
//  CONFIG FUNCTIONS
//====================

//main call for config
void Menu()
{
    int option = -1;
    wchar_t curPath[MAX_PATH]; GetCurrentDirectoryW(MAX_PATH, curPath);

    std::wcout << "Program started in " << curPath << std::endl;
    std::cout << "Launching config..." << std::endl;

    while (true)
    {
        std::cout << "\nCONFIG" << std::endl;
        std::wcout << "Current User: " << steamID64 << std::endl;
        if (IDIsModded(steamID64))
            std::cout << "Status: MODDED\n" << std::endl;
        else
            std::cout << "Status: UNMODDED\n" << std::endl;
        std::cout << "1) Change path to DS3 Folder";
        if (ds3Path == L" ")
            std::cout << " (NEEDS TO BE SET)\n";
        else
            std::cout << "\n";
        std::cout << "2) Change path to DS3 Executable" << std::endl;
        std::cout << "3) Change arguments for launching DS3" << std::endl;
        if (IDIsModded(steamID64))
            std::cout << "4) Switch current account to UNMODDED" << std::endl;
        else
            std::cout << "4) Switch current account to MODDED" << std::endl;
        std::cout << "5) Save Current Mod Configuration to ID" << std::endl;
        std::cout << "6) Exit and Save\n" << std::endl;

        std::cout << "Select an option: " << std::endl;

        do
        {
            option = (int)(getchar() - 48);
            while (getchar() != '\n');
            if (option < 1 || option > 6)
            {
                option = -1;
                std::cout << "Option outside of range." << std::endl;
                std::cout << "Please enter one numbered option and hit enter." << std::endl;
            }
        } while (option == -1);

        switch (option)
        {
        case 1:
            FolderChange();
            break;
        case 2:
            EXEChange();
            break;
        case 3:
            ArgumentChange();
            break;
        case 4:
            ModdedToggle(steamID64);
            break;
        case 5:
            if (ds3Path != L" ")
            {
                ConfigExists(steamID64);
                SaveCurConfig();
            }
            else
                std::cout << "Please set the path to your DS3 Folder." << std::endl;
            break;
        case 6:
            if (ds3Path != L" ")
                config = true;
            RewriteGlobals();
            exit(0);
            break;
        }
    }
}

//called at the end of program to rewrite globals into config file
void RewriteGlobals()
{
    std::list<std::wstring> newLines;

    newLines.push_back(L"config=" + std::to_wstring(config));
    newLines.push_back(L"ds3Path=" + ds3Path);
    newLines.push_back(L"ds3EXE=" + ds3EXE);
    newLines.push_back(L"arguments=" + arguments);

    FILE* fp;
    _wfopen_s(&fp, conFile.c_str(), L"w");
    for (std::list<std::wstring>::iterator it = newLines.begin(); it != newLines.end(); it++)
    {
        fwprintf(fp, L"%s\n", (it->c_str()));
    }

    fclose(fp);
}

void scrubWString(std::wstring& str, BOOL trailingBS)
{
    if (str.empty())
        return;

    while (str.find(L"\"") != -1) //quotations
    {
        str = str.erase(str.find(L"\""), 1);
        if (str.empty())
            return;
    }
    while (str[0] == ' ') //leading spaces
    {
        str = str.erase(str.find(' '), 1);
        if (str.empty())
            return;
    }
    while (str[str.length() - 1] == ' ') //trailing spaces
    {
        str = str.erase(str.length() - 1, 1);
        if (str.empty())
            return;
    }
    if (!trailingBS)
    {
        while (str[str.length() - 1] == '\\') //trailing backslash
        {
            str = str.erase(str.length() - 1, 1);
            if (str.empty())
                return;
        }
    }
    else
    {
        if (str[str.length() - 1] != '\\')
        {
            str = str + L"\\";
        }
    }
}

//called when ds3 folder change option is selected
void FolderChange()
{
    wchar_t sure;
    std::wstring newFolder;

    std::wcout << "Current Dark Souls 3 Folder path: " << ds3Path << std::endl;
    std::cout << "Would you like to change this setting?" << std::endl;
    do
    {
        std::cout << "Please indicate your response with the y(es) or n(o) key." << std::endl;
        sure = getchar();
        while (getchar() != '\n');
    } while (sure != 'y' && sure != 'n');

    if (sure == 'n')
        return;

    std::cout << "Please write the path to your Dark Souls 3 Game Folder:" << std::endl;
    std::cout << "ex. \"C:\\Program Files (x86)\\Steam\\steamapps\\common\\DARK SOULS III\\Game\\\"\n";
    std::getline(std::wcin, newFolder);

    scrubWString(newFolder, TRUE);

    if (!newFolder.empty())
    {
        ds3Path = newFolder;
        if (ds3EXE == L" ")
            ds3EXE = ds3Path + L"DarkSoulsIII.exe";
        std::wcout << L"New Dark Souls 3 Game Folder Location: " << newFolder << std::endl;
    }
    else
    {
        std::wcout << L"No new folder set.\n";
    }
}

//called when ds3 exe change option is selected
void EXEChange()
{
    wchar_t sure;
    std::wstring newPath;

    std::wcout << L"Current Dark Souls 3 Executable path: " << ds3EXE << std::endl;
    std::cout << "Would you like to change this setting?" << std::endl;
    do
    {
        std::cout << "Please indicate your response with the y(es) or n(o) key." << std::endl;
        sure = getchar();
        while (getchar() != '\n');
    } while (sure != 'y' && sure != 'n');

    if (sure == 'n')
        return;

    std::cout << "Please write the path to your Dark Souls 3 Executable:" << std::endl;
    std::cout << "ex. \"C:\\Program Files (x86)\\Steam\\steamapps\\common\\DARK SOULS III\\Game\\DarkSoulsIII.exe\"\n";
    std::getline(std::wcin, newPath);

    scrubWString(newPath, FALSE);

    if (!newPath.empty())
    {
        ds3EXE = newPath;
        std::wcout << L"New Dark Souls 3 Executable Location: " << newPath << std::endl;
    }
    else
    {
        std::wcout << L"No new .exe path set.\n";
    }
}

//called when ds3 folder change option is selected
void ArgumentChange()
{
    wchar_t sure;
    std::wstring newArgs;

    std::wcout << L"Current arguments to running Dark Souls 3: " << arguments << std::endl;
    std::cout << "Would you like to change this setting?" << std::endl;
    do
    {
        std::cout << "Please indicate your response with the y(es) or n(o) key." << std::endl;
        sure = getchar();
        while (getchar() != '\n');
    } while (sure != 'y' && sure != 'n');

    if (sure == 'n')
        return;

    std::cout << "Please write new run arguments (separated by spaces):" << std::endl;
    std::cout << "ex. \"arg1 arg2\"" << std::endl;
    std::getline(std::wcin, newArgs);
    
    scrubWString(newArgs, FALSE);

    if (!newArgs.empty())
    {
        arguments = newArgs;
        std::wcout << L"New Dark Souls 3 Run Arguments: " << newArgs << std::endl;
    }
    else
    {
        std::wcout << L"No new folder set.\n";
    }
}

//called when modded toggle is selected
void ModdedToggle(std::wstring id)
{
    wchar_t sure;
    std::wstring toggle;

    if (IDIsModded(id))
        toggle = L"UNMODDED";
    else
        toggle = L"MODDED";

    std::wcout << L"Are you sure you would like to change the current user to " << toggle << L"?" << std::endl;
    do
    {
        std::cout << "Please indicate your response with the y(es) or n(o) key." << std::endl;
        sure = getchar();
        while (getchar() != '\n');
    } while (sure != 'y' && sure != 'n');

    if (sure == 'n')
        return;

    if (IDIsModded(id))
        UnmodID(id);
    else
        ModID(id);

    std::wcout << L"Steam User \"" << steamID64 << L"\" changed to " << toggle << L" user." << std::endl;
}