del *.vcxproj
del *.vcxproj.filters
del *.vcxproj.user
del *.sln

del ogl\*.vcxproj
del ogl\*.vcxproj.filters
del ogl\*.vcxproj.user
del ogl\*.sln

del CMakeCache.txt
del cmake_install.cmake
rmdir /s /q %cd%\CMakeFiles

del ogl\CMakeCache.txt
del ogl\cmake_install.cmake
rmdir /s /q %cd%\ogl\CMakeFiles

rmdir /s /q %cd%\x64
rmdir /s /q %cd%\Debug
rmdir /s /q %cd%\.vs
rmdir /s /q %cd%\Oniups_Game_Engine.dir

rmdir /s /q %cd%\ogl\Debug
rmdir /s /q %cd%\ogl\ogl.dir

pause