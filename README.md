# WesnothInternalCodeCaveHack

CodeCave.dll injected to `wesnoth.exe` process creates code cave that changes `Gold` value.
It places hook opcodes right before calling `Terrain Description` menu.

Note: the hack is made for `Battle for Wesnoth` v.1.14.9 x86. Other version of the game are more likely
not supported.

## Build

Use Visual Studio standard build routine. Build the DLL for x86 platform.

## Usage

Use any DLL injector that supports x86 applications and inject compiled DLL file to the game process.

Take a look at [releases](https://github.com/elektrikArt/WesnothInternalCodeCaveHack/releases),
where you can find ready-to-use `Run.ps1` script: <br/>
<br/>
![WesnothInternalCodeCaveHack, Demonstration](https://github.com/user-attachments/assets/8f5d9e43-f6cc-4834-9c7c-68593b5c5fe8)



## Contributing

PRs are accepted.
