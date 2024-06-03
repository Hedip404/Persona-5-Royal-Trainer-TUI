#define _X86_ 1
#include <conio.h>
#include <stdio.h>
#include <handleapi.h>
#include <memoryapi.h>
#include <windows.h>
#include <TlHelp32.h>

// 0x2A0 between thieves
#define JOKER_HEALTH 0x29E8EAC 
#define SKULL_HEALTH 0x29E914C
#define MONA_HEALTH 0x29E93EC
#define PANTHER_HEALTH 0x29E968C
#define QUEEN_HEALTH 0x29E9BCC
#define CROW_HEALTH 0x29EA3AC

HANDLE OpenProcessByName(LPCTSTR name, DWORD dwAccess, DWORD *PID);
uintptr_t GetModuleBaseAddress(DWORD procId, const LPCTSTR modName);

typedef struct  {
    char *name;
    int healthValue;
    int cpointsValue;
    uintptr_t healthPtr;
    uintptr_t cpointsPtr;
} Thief;

int main() {

    DWORD pid;
    HANDLE hProcess = OpenProcessByName("P5R.exe", PROCESS_ALL_ACCESS, &pid);
    uintptr_t baseAddr = GetModuleBaseAddress(pid, "P5R.exe");

    if(hProcess == INVALID_HANDLE_VALUE) {
        printf("%s\n", "Couldn't find Persona 5 Royal process, exiting now.");
        return 1;
    }

    printf("Persona 5 R process found!\n");

    // 4 space between HEALTH and CP
    Thief thieves[6] = {
        { "Joker", 0, 0, baseAddr + JOKER_HEALTH, baseAddr + JOKER_HEALTH + 0x4 },
        { "Skull", 0, 0, baseAddr + SKULL_HEALTH, baseAddr + SKULL_HEALTH + 0x4 },
        { "Mona",  0, 0, baseAddr + MONA_HEALTH,  baseAddr + MONA_HEALTH  + 0x4 },
        { "Panther",  0, 0, baseAddr + PANTHER_HEALTH,  baseAddr + PANTHER_HEALTH  + 0x4 },
        { "Queen", 0, 0, baseAddr + QUEEN_HEALTH, baseAddr + QUEEN_HEALTH + 0x4 },
        { "Crow",  0, 0, baseAddr + CROW_HEALTH,  baseAddr + CROW_HEALTH  + 0x4 },
    };

    size_t numberOfBytesRead;
    int spaceBetween = 10;
    int leftPadding = 30;
    int hoveredThief = 0;
    char screenBuf[5096];
    printf("\e[?25l");
    printf("\e[H\e[0J");
    int tick = 0;
    while(1) {
        int lastStored = 0;

        for(int i = 0; i < 6; i++) {
            ReadProcessMemory(hProcess, (void*)thieves[i].healthPtr, &thieves[i].healthValue, sizeof(thieves[i].healthValue), &numberOfBytesRead);
            ReadProcessMemory(hProcess, (void*)thieves[i].cpointsPtr, &thieves[i].cpointsValue, sizeof(thieves[i].cpointsValue), &numberOfBytesRead);

            char *color;
            if(hoveredThief == i) {
                color = "\e[48;5;52m";
            } else {
                color = "\e[48;5;232m";
            }

            lastStored += snprintf((screenBuf + lastStored), sizeof(screenBuf), "\e[1;1H%d%s\e[1;%dH%-8s\e[2;%dH\e[38;5;40m%-8d\e[3;%dH\e[38;5;171m%-8d\e[0m", tick, color, (i*spaceBetween) + leftPadding, thieves[i].name, (i*spaceBetween) + leftPadding, thieves[i].healthValue, (i*spaceBetween) + leftPadding, thieves[i].cpointsValue);
            //printf("\e[45m");
            //printf("\e[1;%dH%s", (i*spaceBetween) + leftPadding, thieves[i].name);
            //printf("\e[2;%dH\e[38;5;40m%d\e[0m", (i*spaceBetween) + leftPadding, thieves[i].healthValue);
            //printf("\e[3;%dH\e[38;5;171m%d\e[0m", (i*spaceBetween) + leftPadding, thieves[i].cpointsValue);
            
        }

        
        if(_kbhit()) {
            int c = _getch();
            if(c == 224) {
                switch (_getch()) {
                    case 72:
                        break;
                    case 80:
                        break;
                    case 77:
                        hoveredThief = (hoveredThief + 1) % 6;
                        break;
                    case 75:
                        hoveredThief = hoveredThief - 1 + ( 6 * !hoveredThief);
                        break;

                }
            }
        }
    
        printf("%s", screenBuf);
        tick++;
    }

    return 0;
}


HANDLE OpenProcessByName(LPCTSTR name, DWORD dwAccess, DWORD *PID) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        ZeroMemory(&pe, sizeof(PROCESSENTRY32));
        pe.dwSize = sizeof(PROCESSENTRY32);
        Process32First(hSnap, &pe);
        do {
            if(!lstrcmpi(pe.szExeFile, name)) {
                *PID = pe.th32ProcessID;
                return OpenProcess(dwAccess, 0, pe.th32ProcessID);
            }

        } while (Process32Next(hSnap, &pe));
    }

    return INVALID_HANDLE_VALUE;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const LPCTSTR modName)
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                if (!lstrcmpi(modEntry.szModule, modName))
                {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}
