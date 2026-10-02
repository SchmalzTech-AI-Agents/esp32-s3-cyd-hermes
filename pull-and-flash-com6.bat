@echo off
setlocal EnableExtensions
REM Run from an ESP-IDF 5.5.3 Command Prompt / PowerShell with idf.py available.
cd /d "%~dp0"
where idf.py >nul 2>&1
if errorlevel 1 (
  echo [ERROR] ESP-IDF 5.5.3 environment is not active. PlatformIO alone cannot build this project.
  echo Open the ESP-IDF 5.5.3 shell and retry this batch file.
  goto :failed
)
where git >nul 2>&1
if errorlevel 1 (
  echo [ERROR] Git for Windows was not found on PATH.
  goto :failed
)
echo [1/3] Fast-forwarding source...
git pull --ff-only origin main
if errorlevel 1 (
  echo [ERROR] Git update failed; preserve/resolve local changes before retrying.
  goto :failed
)
echo [2/3] Selecting ESP32-S3 and building firmware plus Jarvis model...
idf.py set-target esp32s3
if errorlevel 1 goto :failed
idf.py build
if errorlevel 1 goto :failed
echo [3/3] Flashing all partitions to COM6 (including model)...
idf.py -p COM6 flash
if errorlevel 1 (
  echo [ERROR] Flash failed. Check COM port and BOOT/RESET mode.
  goto :failed
)
echo [OK] Full firmware/model flashed. For serial output run: idf.py -p COM6 monitor
pause
exit /b 0
:failed
pause
exit /b 1
