copy /y %QTDIR%\bin\Qt5Core.dll Build\Jupiter_Win32_Release\
copy /y %QTDIR%\bin\Qt5Gui.dll Build\Jupiter_Win32_Release\
copy /y %QTDIR%\bin\Qt5Widgets.dll Build\Jupiter_Win32_Release\
copy /y %QTDIR%\bin\libGLESv2.dll Build\Jupiter_Win32_Release\
copy /y %QTDIR%\bin\libEGL.dll Build\Jupiter_Win32_Release\
copy /y %QTDIR%\bin\icuin52.dll Build\Jupiter_Win32_Release\
copy /y %QTDIR%\bin\icuuc52.dll Build\Jupiter_Win32_Release\
copy /y %QTDIR%\bin\icudt52.dll Build\Jupiter_Win32_Release\
mkdir Build\Jupiter_Win32_Release\platforms
copy /y %QTDIR%\plugins\platforms\qwindows.dll Build\Jupiter_Win32_Release\platforms\