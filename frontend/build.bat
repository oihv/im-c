@echo off
del build\debug\im_c.exe
cmake --build build
build\debug\im_c.exe
