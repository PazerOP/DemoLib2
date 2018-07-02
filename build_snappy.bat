@echo off
setlocal

pushd snappy

mkdir x86
mkdir x64

cd x86
mkdir Debug
mkdir Release

cd Debug
cmake -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 15 2017" ../../
cmake --build . --config Debug

cd ../Release/
cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 15 2017" ../../
cmake --build . --config Release

cd ../../x64
mkdir Debug
mkdir Release

cd Debug
cmake -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 15 2017 Win64" ../../
cmake --build . --config Debug

cd ../Release
cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 15 2017 Win64" ../../
cmake --build . --config Release