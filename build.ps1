cmake -G Ninja -S . -B build `
    -D CMAKE_C_COMPILER=gcc `
    -D CMAKE_CXX_COMPILER=g++ `
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build build

if ($env:PROCESSOR_ARCHITECTURE -eq "AMD64") {
    copy-item .\third_party\wintun\bin\amd64\wintun.dll .\build\wintun.dll
} 
elseif ($env:PROCESSOR_ARCHITECTURE -eq "ARM64") {
    copy-item .\third_party\wintun\bin\arm64\wintun.dll .\build\wintun.dll
}