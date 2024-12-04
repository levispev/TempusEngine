#!/bin/bash

rm -rf bin
rm -rf bin-int

rm -rf Tempus.xcworkspace
rm -rf "Sandbox/Sandbox.xcodeproj"
rm -rf "Tempus/Tempus.xcodeproj"

rm -rf Makefile
rm -rf "Sandbox/Makefile"
rm -rf "Tempus/Makefile"

vendor/bin/premake/premake5 gmake2

read -p "Press any key to continue..."