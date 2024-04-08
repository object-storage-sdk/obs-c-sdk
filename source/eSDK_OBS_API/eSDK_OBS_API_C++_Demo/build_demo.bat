@echo off
xCOPY %cd%\..\bin\*  %~dp0 /y

xCOPY %cd%\..\include\* %~dp0 /y

xCOPY %cd%\securec\include\* %~dp0 /y

xCOPY %cd%\securec\lib\* %~dp0 /y

cl demo_windows.cpp /link libeSDKOBS.lib securec.lib

@echo on