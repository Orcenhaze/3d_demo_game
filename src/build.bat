@echo off

REM USED COMPILER OPTIONS:
REM -Od      : Disables optimization.
REM -MTd     : Links to the static version of the CRT (debug version).
REM -fp:fast : Specifies how the compiler treats floating-point expressions.
REM -GM-     : Enables minimal rebuild.
REM -GR-     : Disables RTTI (C++ feature).
REM -EHa-    : Disables C++ stack unwinding for catch(...).
REM -Oi      : Generates intrinsic functions.
REM -WX      : Treats all warnings as errors.
REM -W4      : Sets warning level 4 to output.
REM -wd####  : Disables the specified warning.
REM -FC      : Displays full path of source code files passed to cl.exe in diagnostic text.
REM -Z7      : Generates C 7.0-compatible debugging information.
REM -Zo      : Generates enhanced debugging information for optimized code (release build).
REM -Fm      : Creates a map file.
REM -opt:ref : Eliminates functions and data that are never referenced.
REM -LD      : Creates a .dll.

REM USED PREPROCESSOR DEFINITIONS:
REM INTERNAL_BUILD : For code that should only run on _your_ machine. Obviously for debugging.
REM DEBUG_BUILD    : For debugging in general, on _any_ developer's machine.



set COMPILER_FLAGS=-Od -nologo -MTd -fp:fast -Gm- -GR- -EHa- -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4456 -FC -Z7 -Zo
set COMPILER_FLAGS=-DDEBUG_BUILD=0 -DINTERNAL_BUILD=1 %COMPILER_FLAGS%
set LINKER_FLAGS=-incremental:no -opt:ref user32.lib gdi32.lib winmm.lib opengl32.lib

REM -fsanitize=address: Enables AddressSanitizer.
set NOLIBCMT=
REM set COMPILER_FLAGS=-fsanitize=address %COMPILER_FLAGS% & set NOLIBCMT=-nodefaultlib:libcmt



REM pushd takes you to a directory you specify and popd takes you back to where you were.
REM "%~dp0" is the drive letter and path combined, of the batch file being executed.
pushd %~dp0
IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM We want to compile some files separately and link them dynamically to the main .exe
REM so we can make changes to the code, recompile and then see the results live.
REM However, when we unload the .dll to reload it, VS2019 will lock the .pdb files instead of
REM automatically unloading them. Therefore, we will explicity delete all .pdb files in our
REM build directory before compiling and create a .pdb with a new name to work around the
REM issue.
REM When deleting the .pdb files the stdout and stderr streams will display unwanted messages
REM to the console, so we will redirect them to NUL to basically get rid of them.

del *.pdb > NUL 2> NUL

REM The compiler writes the .exe/.dll first then the .pdb second, so sometimes when we hot load
REM the .exe/.dll, the compiler still hasn't finished writing the .pdb. 
REM We will create a dummy file during the .dll compilation and use it to determine when
REM to do the reload. We'll call the file lock.tmp.
 
echo WAITING FOR PDB > lock.tmp 
cl %COMPILER_FLAGS% ..\src\game.cpp -LD /link -incremental:no -opt:ref -PDB:game_%random%.pdb -EXPORT:game_init -EXPORT:game_update -EXPORT:game_render  %NOLIBCMT%
set LAST_ERROR=%ERRORLEVEL%
del lock.tmp
cl %COMPILER_FLAGS% ..\src\win32_entry.cpp /link /entry:WinMainCRTStartup /subsystem:console %LINKER_FLAGS% 
popd
popd
