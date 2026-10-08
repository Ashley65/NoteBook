@echo off
echo ===================================================
echo Building NoteBook / Taskhelper C++ Code to WebAssembly
echo ===================================================

if "%QT_WASM_PATH%"=="" (
    set "QT_WASM_PATH=C:/Qt/6.11.0/wasm_multithread"
)

if "%QT_HOST_PATH%"=="" (
    set "QT_HOST_PATH=C:/Qt/6.11.0/mingw_64"
)

set "PATH=C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\mingw1310_64\bin;%PATH%"

if exist "C:\emsdk\emsdk_env.bat" (
    call C:\emsdk\emsdk_env.bat
)

set "EMSDK=C:/emsdk"

echo Using Qt WebAssembly path: %QT_WASM_PATH%
echo Using Qt Host path: %QT_HOST_PATH%

if exist "build-wasm" (
    echo Cleaning previous build-wasm directory...
    rmdir /s /q build-wasm
)

echo Configuring CMake for Qt WebAssembly...
call emcmake cmake -B build-wasm -DCMAKE_TOOLCHAIN_FILE="%QT_WASM_PATH%/lib/cmake/Qt6/qt.toolchain.cmake" -DQT_HOST_PATH="%QT_HOST_PATH%" -DQT_CHAINLOAD_TOOLCHAIN_FILE="C:/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"

echo Building WebAssembly binary (.wasm / .html)...
call cmake --build build-wasm --config Release --target Flow

echo ===================================================
echo Build Complete! Output generated in build-wasm\taskHelper.html
echo Run "python -m http.server 8000 --directory build-wasm" to serve in browser.
echo ===================================================
