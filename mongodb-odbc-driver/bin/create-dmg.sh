#!/bin/bash
# Copyright (c) 2018-Present MongoDB Inc.

# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

(
    if [ "$PLATFORM_NAME" != macos ]; then
        exit 0
    fi

    export PREFIX=mongodb-connector-odbc-"$MDBODBC_VER"-macos-x86-64

    # clear DMG_BUILD_DIR
    rm -rf "$DMG_BUILD_DIR"
    mkdir -p "$DMG_BUILD_DIR"

    cd "$DMG_BUILD_DIR"

    # copy dmg sources to current directory
    cp -R "$PROJECT_ROOT/mongodb-odbc-driver/installer/dmg/"* ./

    # copy driver libraries to appropriate location
    cp "$DRIVERS_DIR"/*.so ./
    # copy the openssl libs to appropriate location
    cp "$OPENSSL_PATH/libssl.1.0.0.dylib"  ./
    cp "$OPENSSL_PATH/libcrypto.1.0.0.dylib"  ./

    install_name_tool -change "$BREW_OPENSSL_PATH/libcrypto.1.0.0.dylib" "@loader_path/libcrypto.1.0.0.dylib" ./libssl.1.0.0.dylib

    sh ./build-dmg.sh

    # clear PKG_DIR
    rm -rf "$PKG_DIR"
    mkdir -p "$PKG_DIR"

    # copy the dmg to PKG_DIR
    cp ./*.dmg "$PKG_DIR"/
) > $LOG_FILE 2>&1

print_exit_msg

