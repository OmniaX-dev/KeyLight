#!/bin/bash

cd ..
./build release

printf "\n\n\033[0;32mBuilding Linux release...\n\033[0m"

mkdir bin/KeyLight_linux64

cp -r extra/* bin/KeyLight_linux64/
cp bin/KeyLight bin/KeyLight_linux64


cp -r licences bin/KeyLight_linux64
cp LICENSE bin/KeyLight_linux64/licences/KeyLight-LICENCE.txt

cp -r bin/locale bin/KeyLight_linux64

printf "\n\033[0;32mLinux release ready in bin/KeyLight_linux64!\n\033[0m"
