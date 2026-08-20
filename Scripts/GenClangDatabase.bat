@echo off
REM Regenerate compile_commands.json for clangd.
REM Run this after adding .cpp/.h files or editing Build.cs,
REM otherwise clangd will not recognize the new files.
setlocal

set "ENGINE_DIR=D:\Epic gaems\installer\UE_5.3"
set "PROJECT_DIR=%~dp0.."
set "UBT=%ENGINE_DIR%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"

"%UBT%" -Mode=GenerateClangDatabase -Project="%PROJECT_DIR%\Aura.uproject" -Game -Engine -Target="AuraEditor Win64 Development"
if errorlevel 1 goto :fail

REM UBT always writes the database to the engine root; copy it back so clangd can find it.
copy /Y "%ENGINE_DIR%\compile_commands.json" "%PROJECT_DIR%\compile_commands.json" >nul
if errorlevel 1 goto :fail

echo [OK] compile_commands.json updated
exit /b 0

:fail
echo [FAILED] see output above
exit /b 1
