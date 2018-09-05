#!/bin/bash
# Copyright (c) 2018-Present MongoDB Inc.

# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

(
    # clear the BUILD_DIR
    echo "clear $BUILD_DIR"
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"

    # clear DRIVERS_DIR
    echo "clear $DRIVERS_DIR"
    rm -rf "$DRIVERS_DIR"
    mkdir -p "$DRIVERS_DIR"

    # clear BUILD_SRC_DIR
    echo "clear $BUILD_SRC_DIR"
    rm -rf "$BUILD_SRC_DIR"
    mkdir -p "$BUILD_SRC_DIR"

    # copy odbc source into BUILD_SRC_DIR
    echo "copying odbc source to $BUILD_SRC_DIR"
    for x in "$PROJECT_ROOT"/*; do
        if [ ! "$(basename "$x")" = mongodb-odbc-driver ]; then
            echo "...copying $x to $BUILD_SRC_DIR"
        cp -r "$x" "$BUILD_SRC_DIR"/
        fi
    done

    # copy mongosql-auth-c cmake utils into odbc source
    echo "copying mongosql-auth-c cmake files $MONGOSQL_AUTH_PROJECT_DIR to $BUILD_SRC_DIR"
    cp "$MONGOSQL_AUTH_PROJECT_DIR"/cmake/*.cmake "$BUILD_SRC_DIR"/cmake/

    cd "$BUILD_DIR"
    # on OS X we use iODBC
    if [ "$PLATFORM" = macos ]; then
        iODBC_dir=iODBC-3.52.12
        echo "downloading iODBC"
        curl -O "http://noexpire.s3.amazonaws.com/sqlproxy/binary/linux/$iODBC_dir.tar.gz"
        tar xf "$iODBC_dir.tar.gz"
        UNIX_LIB="-DODBC_INCLUDES=$BUILD_DIR/$iODBC_dir/include -DODBC_LIB_DIR=/usr/lib"
    fi
    # on Linux we use unixODBC
    if [ "$PLATFORM_NAME" = linux ]; then
        "$SCRIPT_DIR"/build-unixodbc.sh
        UNIX_LIB="-DODBC_INCLUDES=$BUILD_DIR/install/include -DODBC_LIB_DIR=$BUILD_DIR/install/lib -DWITH_UNIXODBC=1"
    fi

    # Set locations for ICU
    ICU_DIR="$MYSQL_PROJECT_DIR/bld/artifacts/icu"
    ICU_SRC_DIR="$ICU_DIR/icu/source"
    ICU_BUILD_DIR="$ICU_DIR/build"
    ENABLE_ICU='ON'
    CMAKE_ICU_ARGS="-DENABLE_ICU=$ENABLE_ICU -DICU_ROOT=$ICU_BUILD_DIR -DICU_INCLUDE_DIR=$ICU_SRC_DIR/common"
    CMAKE_ARGS="$CMAKE_ARGS $CMAKE_ICU_ARGS"

    # run CMake in the BUILD_DIR
    cmake "$BUILD_SRC_DIR" -G "$CMAKE_GENERATOR" -DMYSQLCLIENT_STATIC_LINKING:BOOL=TRUE -DCMAKE_MODULE_PATH="$CMAKE_MODULE_PATH" $UNIX_LIB $CMAKE_ARGS

    echo 'building'
    # build the ODBC driver
    if [ "$PLATFORM_NAME" = "windows" ]; then
        devenv.com MySQL_Connector_ODBC.sln /build Release || true
    else
        make mdbodbca mdbodbcw
    fi

    echo 'copy artifacts'
    # copy artifacts into DRIVERS_DIR
    if [ "$PLATFORM_NAME" = "windows" ]; then
        cp "$BUILD_DIR"/lib/Release/mdbodbc{a,S,w}.{dll,lib} "$DRIVERS_DIR"
        cp "$BUILD_DIR"/bin/Release/myodbc-installer.exe "$DRIVERS_DIR"/installer.exe
    else
        cp "$BUILD_DIR"/lib/libmdbodbc{a,w}.so "$DRIVERS_DIR"
    fi
) > $LOG_FILE 2>&1

print_exit_msg
