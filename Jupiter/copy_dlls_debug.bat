copy /y %QTDIR%\bin\Qt5Cored.dll Build\Jupiter_Win32_Debug\
copy /y %QTDIR%\bin\Qt5Guid.dll Build\Jupiter_Win32_Debug\
copy /y %QTDIR%\bin\Qt5Widgetsd.dll Build\Jupiter_Win32_Debug\
copy /y %QTDIR%\bin\libGLESv2d.dll Build\Jupiter_Win32_Debug\
copy /y %QTDIR%\bin\libEGLd.dll Build\Jupiter_Win32_Debug\
copy /y %QTDIR%\bin\icuin52.dll Build\Jupiter_Win32_Debug\
copy /y %QTDIR%\bin\icuuc52.dll Build\Jupiter_Win32_Debug\
copy /y %QTDIR%\bin\icudt52.dll Build\Jupiter_Win32_Debug\
mkdir Build\Jupiter_Win32_Debug\platforms
copy /y %QTDIR%\plugins\platforms\qwindowsd.dll Build\Jupiter_Win32_Debug\platforms\