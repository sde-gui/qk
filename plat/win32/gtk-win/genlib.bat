@echo off
setlocal

call "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\vcvars32.bat"

cd C:\Projects\gtk\release\target\lib

FOR %%d IN (*.def) DO (
  echo %%d
  lib /def:%%d /machine:x86
)
