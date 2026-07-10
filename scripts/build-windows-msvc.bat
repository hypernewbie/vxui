@echo off
REM Mirror .github/workflows/ci.yml for the Windows job: configure with the
REM default Visual Studio generator, build Release, run ctest.

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DGLFW_BUILD_WAYLAND=OFF
if errorlevel 1 exit /b 1
cmake --build build --config Release --parallel
if errorlevel 1 exit /b 1
ctest --test-dir build -C Release --output-on-failure