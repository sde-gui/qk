@echo off
setlocal

cd C:\Projects\debian\gtk-win-build\release\target\lib

FOR %%d IN (*.def) DO (
  echo %%d
  lib /def:%%d /machine:x86
)

dir

