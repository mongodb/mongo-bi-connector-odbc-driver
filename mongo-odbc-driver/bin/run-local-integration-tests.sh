#!/bin/bash

# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

test_set=''
if [ "$TEST_SET" = 'SSL' ]; then
    test_set='LocalSSL'
elif [ "$TEST_SET" = '' ]; then
    test_set='Local'
fi

if [ "Windows_NT" = "$OS" ]; then

    if [ "$PLATFORM_ARCH" = "64" ]; then
        platform="64-bit"
    else
        platform="32-bit"
    fi

    cd "$PKG_DIR"
    echo 'running local connection integration tests...'
    "$POWERSHELL" \
        -ExecutionPolicy ByPass \
        -NoProfile \
        -NoLogo \
        -NonInteractive \
        -File "$SCRIPT_DIR/run-integration-tests.ps1" \
        -"$test_set" \
        -Platform "$platform" \
        -Server '127.0.0.1' \
        -Port '3307' \
        -User 'user_not_used' \
        -Password 'password_not_used' \
        -DB 'information_schema'
    echo 'integration tests passed'

else
    echo 'integration tests not yet implemented for non-windows platforms'
    exit 1
fi
