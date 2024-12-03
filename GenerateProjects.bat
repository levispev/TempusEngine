@echo off

rmdir /s /q bin
rmdir /s /q bin-int
rmdir /s /q .vs

del Tempus.sln
del "Sandbox\Sandbox.vcxproj"
del "Sandbox\Sandbox.vcxproj.user"
del "Tempus\Tempus.vcxproj"
del "Tempus\Tempus.vcxproj.filters"

call vendor\bin\premake\premake5.exe vs2022
PAUSE