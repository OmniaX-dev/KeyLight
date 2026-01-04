#!/bin/bash

MSYS2_ROOT=c:/msys64

cd ..
./build release

printf "\n\n\033[0;32mBuilding Windows release...\n\033[0m"

mkdir bin/KeyLight_w64

cp -r extra/* bin/KeyLight_w64/
cp other/appIcon.ico bin/KeyLight_w64/themes/ui

cp bin/KeyLight.exe bin/KeyLight_w64

for dll in libgcc_s_seh-1.dll libstdc++-6.dll libostd.dll libwinpthread-1.dll \
           tgui.dll sfml-audio-3.dll sfml-graphics-3.dll sfml-system-3.dll \
           sfml-window-3.dll libboost_filesystem-mt.dll libintl-b.dll libiconv-2.dll; do
    src="$MSYS2_ROOT/ucrt64/bin/$dll"
    [[ -f "$src" ]] || { echo "Missing: $dll"; exit 1; }
    cp "$src" bin/KeyLight_w64/
done

cp -r licences bin/KeyLight_w64
cp LICENSE bin/KeyLight_w64/licences/KeyLight-LICENCE.txt

cp -r bin/locale bin/KeyLight_w64

printf "\n\033[0;32mWindows release ready in bin/KeyLight_w64!\n\033[0m"
