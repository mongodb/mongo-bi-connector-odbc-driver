#!/bin/bash
# Copyright (c) 2018-Present MongoDB Inc.
if [ "$PLATFORM" = "" ]; then
    PLATFORM=win64
    echo "WARNING: no value provided for \$PLATFORM: using default of '$PLATFORM'"
fi

case "$PLATFORM" in
win64)
    # We must use 64-bit powershell to test 64-bit odbc
    POWERSHELL='C:/Windows/System32/WindowsPowerShell/v1.0/powershell.exe' # 64-bit powershell
    PLATFORM_ARCH='64'
    PLATFORM_NAME='windows'
    ;;
win32)
    # We must use 32-bit powershell to test 32-bit odbc
    POWERSHELL='C:/Windows/SysWOW64/WindowsPowerShell/v1.0/powershell.exe' # 32-bit powershell
    PLATFORM_ARCH='32'
    PLATFORM_NAME='windows'
    ;;
*)
    echo "ERROR: invalid value for \$PLATFORM: '$PLATFORM'"
    echo "Allowed values: 'win64', 'win32'"
    exit 1
    ;;
esac
