#!/bin/bash
# Copyright (c) 2018-Present MongoDB Inc., licensed under
# GNU GENERAL PUBLIC LICENSE Version 2.

# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

# clear the BUILD_DIR
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# clear DRIVERS_DIR
rm -rf "$DRIVERS_DIR"
mkdir -p "$DRIVERS_DIR"

# clear BUILD_SRC_DIR
rm -rf "$BUILD_SRC_DIR"
mkdir -p "$BUILD_SRC_DIR"

# copy odbc source into BUILD_SRC_DIR
cp -r "$PROJECT_ROOT"/* "$BUILD_SRC_DIR"/ || true
rm -rf "$BUILD_SRC_DIR"/mongo-odbc-driver

# copy mongosql-auth-c cmake utils into odbc source
cp "$MONGOSQL_AUTH_PROJECT_DIR"/cmake/*.cmake "$BUILD_SRC_DIR"/cmake/

# run CMake in the BUILD_DIR
cd "$BUILD_DIR"
cmake "$BUILD_SRC_DIR" -G "$CMAKE_GENERATOR" -DMYSQLCLIENT_STATIC_LINKING:BOOL=TRUE

# build the ODBC driver
devenv.com MySQL_Connector_ODBC.sln /build Release

# copy artifacts into DRIVERS_DIR
cp "$BUILD_DIR"/lib/Release/mdbodbc{a,S,w}.{dll,lib} "$DRIVERS_DIR"
cp "$BUILD_DIR"/bin/Release/myodbc-installer.exe "$DRIVERS_DIR"/installer.exe
