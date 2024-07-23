/*+===================================================================
  File:      CodeCave.dll

  Summary:   CodeCave.dll injected to `wesnoth.exe` process creates
             code cave that changes `Gold` value. It places hook
             opcodes right before calling `Terrain Description` menu.
             

  Classes:   None

  Functions: void CodeCave()

  Origin:    It is my implementation to the lesson
             (https://gamehacking.academy/pages/3/04/)

===================================================================+*/

#include <windows.h>
#include <easylogging++.h>

INITIALIZE_EASYLOGGINGPP

DWORD ret_address = 0xCCAF90;

DWORD* pdwPlayerBase = NULL;
DWORD* pdwGameBase = NULL;
DWORD* pdwGold = NULL;

void ConfigureLoggers()
{
    el::Configurations loggersConfigs;
    loggersConfigs.setToDefault();
    loggersConfigs.setGlobally(el::ConfigurationType::Filename, "C:\\Users\\musli\\AppData\\Local\\Temp\\CodeCaveDll.log");
    el::Loggers::reconfigureAllLoggers(loggersConfigs);
}

__declspec(naked) void CodeCave() {
    //  __declspec() - is keyword that tells compiler how to assemble function.
    //
    //      naked - is atributes that tells compiler not to create standard function
    //      stack skeleton
    //
    //          function:
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
    *pdwGold = 888;

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

DWORD GetLocationForJmp(DWORD* lpDestinationLocation, DWORD* lpOpcodeLocation) {
    return (DWORD)lpDestinationLocation - (DWORD)lpOpcodeLocation + 5;
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
            &flOldProtect) == 0)
        {
            LOG(ERROR) << "VirtualProtect() fail";
        }

        lpHookOpcode[0] = 0xE9;   //  `jmp` opcode
        *(DWORD*)(lpHookOpcode + 1) = GetLocationForJmp((DWORD*)&CodeCave, (DWORD*)lpHookOpcode);   //  writes 4 bytes
        lpHookOpcode[5] = 0x90;   //  `nop` opcode
    }

    return true;
}

