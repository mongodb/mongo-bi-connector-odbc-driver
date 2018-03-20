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

# run CMake in the BUILD_DIR
cd "$BUILD_DIR"
cmake "$PROJECT_ROOT" -G "$CMAKE_GENERATOR" -DMYSQLCLIENT_STATIC_LINKING:BOOL=TRUE

# build the ODBC driver
devenv.com MySQL_Connector_ODBC.sln /build Release

# copy artifacts into DRIVERS_DIR
cp "$BUILD_DIR"/lib/Release/mdbodbc{a,S,w}.{dll,lib} "$DRIVERS_DIR"
cp "$BUILD_DIR"/bin/Release/myodbc-installer.exe "$DRIVERS_DIR"/installer.exe
