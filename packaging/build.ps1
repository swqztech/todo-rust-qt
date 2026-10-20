$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Qt = if ($env:CMAKE_PREFIX_PATH) { $env:CMAKE_PREFIX_PATH } else { Join-Path $Root ".." ".qt" "6.8.2" "msvc2022_64" }
cmake -S $Root -B (Join-Path $Root "build") -DCMAKE_PREFIX_PATH=$Qt
cmake --build (Join-Path $Root "build") --config Release
