cmake .

del ALL_BUILD.vcxproj
del ALL_BUILD.vcxproj.filters

del INSTALL.vcxproj
del INSTALL.vcxproj.filters

del ZERO_CHECK.vcxproj
del ZERO_CHECK.vcxproj.filters

del ogl\INSTALL.vcxproj
del ogl\INSTALL.vcxproj.filters

rmdir /s /q %cd%\CMakeFiles
rmdir /s /q %cd%\ogl\CMakeFiles

del cmake_install.cmake
del ogl\cmake_install.cmake
del CMakeCache.text
del ogl\CMakeCache.text

pause