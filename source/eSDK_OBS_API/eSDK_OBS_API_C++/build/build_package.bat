@echo off

::Usage: build.bat packageName release|debug
::
::obs的目录结构：
:: obs
:: ├─bin
:: ├─demo
:: ├─include
::安装包里的结构和部署结构一致。

SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

set CurBatPath=%~dp0
set AGENTPATH=%CurBatPath%

set "G_MSVC_VERSION=vc100"
set "G_sln_NAME=obs"

set "G_BUILD_DIR=%~dp0"
set "G_sln_DIR=%G_BUILD_DIR%..\sln\%G_MSVC_VERSION%\"
set "G_MSBUILD_DIR=C:\Windows\Microsoft.NET\Framework\v4.0.30319\"
set "G_OBS_ROOT=%G_BUILD_DIR%..\"
set "G_OBS_3rd_depend_DIR=%G_OBS_ROOT%..\..\..\"
set "G_TARGET_PLATFORM=WIN32"
set "G_SECUREC_DIR=%G_sln_DIR%\Release"
set "G_3rd_PLATFORM=windows\x86"

::获得当前时间，作为生成版本的目录名
for /F "tokens=1-4 delims=-/ " %%i in ('date /t') do (
   set Year=%%i
   set Month=%%j
   set Day=%%k
   set DayOfWeek=%%l
)
for /F "tokens=1-2 delims=: " %%i in ('time /t') do (
   set Hour=%%i
   set Minute=%%j
)

::设置各变量名
set WinRarRoot=C:\Program Files (x86)\WinRAR
set Version=2.1.10
set SDKTarget=eSDK_HWS_OBS_API_V%Version%_Windows_C.rar
set DateTime=%Year%-%Month%-%Day%-%Hour%-%Minute%
set DateOnly=%Year%-%Month%-%Day%
set VMPIP=10.162.145.59

:: -------------------- main --------------------------

echo compile eSDK_OBS_API_C++ for windows.
echo.

if not exist %G_MSBUILD_DIR% (
    echo "error: .NET Framework should be installed."
    
    ENDLOCAL&exit /b 1
)

set G_BUILD_OPT=release
set "G_BUILD_OUTPUT_DIR=%G_BUILD_DIR%%G_MSVC_VERSION%\%G_BUILD_OPT%\"

call :toLowerCase G_BUILD_OPT

set /a L_retValue=0
call :compileProvider %G_BUILD_OPT% L_retValue
if %L_retValue% NEQ 0 (
    echo error:build eSDK_OBS_API_C++ failed.
    
   ENDLOCAL&exit /b 1
)

set /a L_retValue=0
call :  %G_PKG_NAME% L_retValue
if %L_retValue% NEQ 0 (
    echo error:package failed.
    
    ENDLOCAL&exit /b 1
)

::echo xcopy /y %AGENTPATH%%SDKTarget% "\\%VMPIP%\Packages\%DateOnly%\HWS\"
::xcopy /y %AGENTPATH%%SDKTarget% "\\%VMPIP%\Packages\%DateOnly%\HWS\"
echo d | xcopy /y /s "%AGENTPATH%%SDKTarget%" "..\..\..\..\build\OBS\"

echo complete.

ENDLOCAL
:: ------------------ normal script execute flow end -----------------------
goto:EOF

:: -------------------- function ----------------------

:: ****************************************************************************
:: Function Name: compileProvider
:: Description: 
:: Parameter: %1 release|debug  %2 L_retValue 
:: Return: none
:: ****************************************************************************
:compileProvider
  
    set "L_sln_File=%G_sln_DIR%\%G_sln_NAME%.sln"
    if not exist %L_sln_File% (
        echo error: the sln is not exist.
        call :USAGE
        set /a %~2=1
        exit /b 1
    )
    
    if exist %G_BUILD_OUTPUT_DIR% (
        echo delete old output dir.
        echo.
        rmdir /s /q %G_BUILD_OUTPUT_DIR%
    )

    %G_MSBuild_DIR%MSBuild.exe %L_sln_File% /t:rebuild /p:Configuration=%1;Platform=%G_TARGET_PLATFORM%
    set /a %~2=%ERRORLEVEL%

goto:EOF

:: ****************************************************************************
:: Function Name: package
:: Description: 
:: Parameter: $1 packageName $2 retValue
:: Return: none
:: ****************************************************************************
:package
   set "L_TMP_PACKAGE_DIR=%G_BUILD_DIR%obs\"
   if exist %L_TMP_PACKAGE_DIR% (
       rmdir /q /s %L_TMP_PACKAGE_DIR%
   )
   
   mkdir %L_TMP_PACKAGE_DIR%
   mkdir %L_TMP_PACKAGE_DIR%demo
   mkdir %L_TMP_PACKAGE_DIR%include
   mkdir %L_TMP_PACKAGE_DIR%bin
   mkdir %L_TMP_PACKAGE_DIR%demo\securec
   mkdir %L_TMP_PACKAGE_DIR%demo\securec\lib
   mkdir %L_TMP_PACKAGE_DIR%demo\securec\include

   if %ERRORLEVEL% NEQ 0 (
      echo make dir %L_TMP_PACKAGE_DIR% failed.
      set /a %~2=1
      exit /b 1
   )
   
   echo copy 3rd files...
   set "L_TMP_3rd_LIB_PATH=%G_OBS_3rd_depend_DIR%open_src\"   
   if not exist "%L_TMP_3rd_LIB_PATH%" (
       echo %L_TMP_3rd_LIB_PATH% not exist.
       set /a %~2=1
       exit /b 1
   )
   
   pushd %L_TMP_3rd_LIB_PATH%
     
   xcopy /c /Y curl-7.64.0\bin\%G_3rd_PLATFORM%\*.dll %L_TMP_PACKAGE_DIR%bin
   xcopy /c /Y curl-7.64.0\bin\%G_3rd_PLATFORM%\*.exe %L_TMP_PACKAGE_DIR%bin
   xcopy /c /Y curl-7.64.0\bin\%G_3rd_PLATFORM%\*.manifest %L_TMP_PACKAGE_DIR%bin

   
   xcopy /c /Y openssl-1.0.2k\bin\%G_3rd_PLATFORM%\*.dll %L_TMP_PACKAGE_DIR%bin
   xcopy /c /Y openssl-1.0.2k\bin\%G_3rd_PLATFORM%\*.exe %L_TMP_PACKAGE_DIR%bin
   xcopy /c /Y openssl-1.0.2k\bin\%G_3rd_PLATFORM%\*.manifest %L_TMP_PACKAGE_DIR%bin 

   
   xcopy /c /Y pcre-8.39\bin\%G_3rd_PLATFORM%\*.dll %L_TMP_PACKAGE_DIR%bin
   xcopy /c /Y pcre-8.39\bin\%G_3rd_PLATFORM%\*.exe %L_TMP_PACKAGE_DIR%bin
   xcopy /c /Y pcre-8.39\bin\%G_3rd_PLATFORM%\*.manifest %L_TMP_PACKAGE_DIR%bin
   
   popd
   
   
   xcopy /c /Y %G_OBS_3rd_depend_DIR%platform\eSDK_LogAPI_V2.1.10\C\release\*.dll %L_TMP_PACKAGE_DIR%bin
   
   
   echo "copy vc crt..."    
   set "L_TMP_VC_CRT_DIR=%G_OBS_3rd_depend_DIR%third_party\smis\win32_x86_msvc\Microsoft.VC100.CRT\"
   echo %L_TMP_VC_CRT_DIR%
   xcopy /c /Y %L_TMP_VC_CRT_DIR%*.dll %L_TMP_PACKAGE_DIR%bin
   
   
   echo copy files...
   for /f %%i in ('dir /b %G_BUILD_OUTPUT_DIR%\*.dll') do (
      xcopy /c /Y %G_BUILD_OUTPUT_DIR%%%i %L_TMP_PACKAGE_DIR%bin
   )
   
   for /f %%i in ('dir /b %G_BUILD_OUTPUT_DIR%\*.lib') do (
      xcopy /c /Y %G_BUILD_OUTPUT_DIR%%%i %L_TMP_PACKAGE_DIR%bin
   )
   :: 打包安全库文件
   xcopy /c /Y %G_OBS_3rd_depend_DIR%platform\securec\include\*.h   %L_TMP_PACKAGE_DIR%demo\securec\include
   xcopy /c /Y %G_BUILD_OUTPUT_DIR%securec.lib %L_TMP_PACKAGE_DIR%demo\securec\lib
   
   xcopy /c /Y %G_BUILD_DIR%..\inc\eSDKOBSS3.h %L_TMP_PACKAGE_DIR%include
   xcopy /c /Y %G_BUILD_DIR%..\build\OBS.ini %L_TMP_PACKAGE_DIR%bin
   
   xcopy /c /Y %G_BUILD_OUTPUT_DIR%libeSDKOBS.lib %L_TMP_PACKAGE_DIR%demo
   xcopy /c /Y %G_BUILD_DIR%..\inc\eSDKOBS.h %L_TMP_PACKAGE_DIR%demo
   xcopy /c /Y %G_BUILD_DIR%..\cert\client.crt %L_TMP_PACKAGE_DIR%demo
   xcopy /c /Y %G_BUILD_DIR%..\cert\server.jks %L_TMP_PACKAGE_DIR%demo
   xcopy /c /Y %G_OBS_3rd_depend_DIR%test\demo\eSDK_OBS_API_C++_Demo\README.txt %L_TMP_PACKAGE_DIR%demo
   xcopy /c /Y %G_OBS_3rd_depend_DIR%test\demo\eSDK_OBS_API_C++_Demo\set.bat %L_TMP_PACKAGE_DIR%demo
   xcopy /c /Y %G_OBS_3rd_depend_DIR%test\demo\eSDK_OBS_API_C++_Demo\s3.c %L_TMP_PACKAGE_DIR%demo
   xcopy /c /Y %G_OBS_3rd_depend_DIR%test\demo\eSDK_OBS_API_C++_Demo\sln\*.sln %L_TMP_PACKAGE_DIR%demo\sln\
   xcopy /c /Y %G_OBS_3rd_depend_DIR%test\demo\eSDK_OBS_API_C++_Demo\sln\*.vcxproj %L_TMP_PACKAGE_DIR%demo\sln\
   xcopy /c /Y %G_OBS_3rd_depend_DIR%test\demo\eSDK_OBS_API_C++_Demo\sln\*.filters %L_TMP_PACKAGE_DIR%demo\sln\
   xcopy /c /Y %G_BUILD_DIR%..\readme.txt %L_TMP_PACKAGE_DIR%

   
   cd %G_BUILD_DIR%
   
   :: package
   echo "%WinRarRoot%\WinRAR.exe" a -ep1 -r %AGENTPATH%%SDKTarget% %L_TMP_PACKAGE_DIR%
   "%WinRarRoot%\WinRAR.exe" a -ep1 -r %AGENTPATH%%SDKTarget% %L_TMP_PACKAGE_DIR%
        if !ERRORLEVEL! NEQ 0 (
            echo error:package faild.
            set /a %~2=0
            exit /b 1
        )
   )

goto:EOF

:: ****************************************************************************
:: Function Name: toUpperCase
:: Description: 
:: Parameter: $1 string in ASCII
:: Return: none
:: ****************************************************************************
:toUpperCase
    for %%a in ("a=A" "b=B" "c=C" "d=D" "e=E" "f=F" "g=G" "h=H" "i=I" "j=J" "k=K" "l=L" "m=M" "n=N" "o=O" "p=P" "q=Q" "r=R" "s=S" "t=T" "u=U" "v=V" "w=W" "x=X" "y=Y" "z=Z") do ( 
        call set "%1=%%%1:%%~a%%"
    )
goto:EOF

:: ****************************************************************************
:: Function Name: toLowerCase
:: Description: 
:: Parameter: $1 string in ASCII
:: Return: none
:: ****************************************************************************
:toLowerCase
    for %%a in ("A=a" "B=b" "C=c" "D=d" "E=e" "F=f" "G=g" "H=h" "I=i" "J=j" "K=k" "L=l" "M=m" "N=n" "O=o" "P=p" "Q=q" "R=r" "S=s" "T=t" "U=u" "V=v" "W=w" "X=x" "Y=y" "Z=z") do ( 
        call set "%1=%%%1:%%~a%%"
    )
goto:EOF

:USAGE
    echo "Usage: %~nx0 pkgName debug|release"
goto:EOF

@echo on
