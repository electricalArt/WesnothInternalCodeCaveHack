/*+===================================================================
  File:      CodeCave.dll

  Summary:   CodeCave.dll injected to `wesnoth.exe` process creates
             code cave that changes `Gold` value. It places hook
             opcodes right before calling `Terrain Description` menu.

             Note: the hack is made for x86 version of the game, and
             it should be build as x86 application.
             

  Classes:   None

  Origin:    It is my implementation to the lesson
             (https://gamehacking.academy/pages/3/04/)

===================================================================+*/

#include <windows.h>
#include <easylogging++.h>
#include <filesystem>

INITIALIZE_EASYLOGGINGPP

DWORD ret_address = 0xCCAF90;
// It contains address that should be executed after code cave instructions

DWORD* pdwPlayerBase = NULL;
DWORD* pdwGameBase = NULL;
DWORD* pdwGold = NULL;

void ConfigureLoggers()
{
    el::Configurations loggersConfigs;
    loggersConfigs.setToDefault();
    loggersConfigs.setGlobally(el::ConfigurationType::Filename, std::filesystem::temp_directory_path().string() + "\\CodeCaveDll.log");
    el::Loggers::reconfigureAllLoggers(loggersConfigs);
}

__declspec(naked) void CodeCave() {
    //  __declspec() - is keyword that tells compiler how to assemble function.
    //
    //      naked - is atributes that tells compiler not to create standard function
    //      stack skeleton.
    //
    //          Standard function skeleton is:
    //              push ebp
    //              mov ebp, esp
    //              ...
    //              mov esp, ebp
    //              pop ebp
    //              ret
    //

    //  When creating code cave first thing to do is to save and restore the
    //  registers and then restore the overwriteen instructions. It should be
    //  done to origin code run as nothing is happend.
    //
    //      Here it is done using two `__asm {}` blocks.
    //

    //  Saving the registers.
    __asm {
        pushad
    }

    //  Hacking. Rewriting the gold.
    pdwPlayerBase = (DWORD*)(0x017EED18);
    pdwGameBase = (DWORD*)(*pdwPlayerBase + 0xA90);
    pdwGold = (DWORD*)(*pdwGameBase + 0x4);
    *pdwGold = 9999;

    //  Restoring the registers. And performing those opcodes that were
    //  overwritten (with opcodes jumping to our code cave).
    __asm {
        popad
        mov eax, dword ptr ds:[ecx]
        lea esi, dword ptr ds:[esi]
        jmp ret_address
        //  ret_address - `jmp` opcode should be used with variable, not with
        //  direct address `0xCCAF90`. This is because `jmp` opcode have many
        //  types (????one for 32-bit address, otherone for 16-bit address),
        //  and with direct address C++ compiler cannot distinguish wich to use.
    }
}

//  Calculates jump address for specified address
DWORD GetLocationBytesForJmp(DWORD* lpDestinationLocation, DWORD* lpOpcodeLocation) {
    DWORD dwLocationBytesForJmp = (DWORD)lpDestinationLocation - (DWORD)lpOpcodeLocation - 5;
    LOG(INFO) << "dwLocationBytesForJmp: " << std::hex << dwLocationBytesForJmp;
    return dwLocationBytesForJmp;
}

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        ConfigureLoggers();
        LOG(INFO) << "Start";

        unsigned char* lpHookOpcode = (unsigned char*)0x00CCAF8A;
        //  `lpHookOpcode` contains address to opcode right before opcode that
        //   calls `Terrain Description` menu. 

        DWORD flOldProtect = { 0 };
        if (VirtualProtect(
                lpHookOpcode,
                6,
                PAGE_EXECUTE_READWRITE,
                &flOldProtect) == 0) {
            LOG(ERROR) << "VirtualProtect() fail";
        }
        if (VirtualProtect(
                (LPVOID)0x017EED18,
                4,
                PAGE_READWRITE,
                &flOldProtect) == 0) {
            LOG(ERROR) << "VirtualProtect() fail";
        }

        LOG(INFO) << "lpHookOpcode: " << std::hex << lpHookOpcode;
        LOG(INFO) << "CodeCave() actual location: " << std::hex << CodeCave;
        lpHookOpcode[0] = 0xE9;   //  `jmp` opcode
        *(DWORD*)(lpHookOpcode + 1) = GetLocationBytesForJmp((DWORD*)&CodeCave, (DWORD*)lpHookOpcode);   //  writes 4 bytes
        lpHookOpcode[5] = 0x90;   //  `nop` opcode
        //  GetLocationBytesForJmp() should be used because you can't just write to `jmp` opcode
        //  some address as it is. The `jmp` opcode should contains calculated address.

        LOG(INFO) << "End";
    }

    return true;
}

