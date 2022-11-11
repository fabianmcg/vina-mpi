# autodock-vina
Fork of autodock vina, from http://vina.scripps.edu/.

## Binaries 
Binaries (Windows, Linux) can be downloaded from [repository releases](https://github.com/skodapetr/autodock-vina/releases). 

## Building from source

### Dependencies
*  [Boost 1.67](https://www.boost.org/doc/libs/1_67_0/) 

### Linux
 * Get boost. Either compile from sources or use packaging manager.  
 It may work fine  with older versions of boost (1.5) as well.
 * Use makefiles in `build/linux`.

### Windows (Visual Studio 2017)
 * Download and unpack Boost into `dependencies/boost_1_67_0`.
 * Start `x64 Native Tools Command Prompt for VS 2017`.
 * Navigate into `dependencies/boost_1_67_0` directory.
 * Run `bootstrap.bat`.
 * Run `b2 -a -j8 address-model=64 architecture=x86`.
 * open Visual Studio solution (`autodock-vina.sln`) in `build/windows/`.
 * Build `vina` or `vina-split` project. Do not build the solution as that would run build of both project at once causing them to collide.

## Change log
 * Update sources to work with Boost 1.67
 * Fix issue with hardcoded limit on generated models


