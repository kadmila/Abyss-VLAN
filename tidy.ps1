Get-ChildItem .\src -Recurse -Filter *.c | ForEach-Object {
    clang-tidy $_.FullName -p build --fix
}