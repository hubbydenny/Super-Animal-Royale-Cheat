@echo off
set MSBUILD="C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
%MSBUILD% SuperAnimalHack.sln /p:Configuration=Release /p:Platform=x64 /v:minimal
pause
