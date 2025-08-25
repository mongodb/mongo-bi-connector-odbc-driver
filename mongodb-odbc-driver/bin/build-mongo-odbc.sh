#!/bin/bash
# Copyright (c) 2018-Present MongoDB Inc.

# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

(
    set -o errexit
    # clear the BUILD_DIR
    echo "clear $BUILD_DIR"
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    MONGOSQL_AUTH_LIB_NAME="mongosql_auth"
    if [ "$PLATFORM_NAME" = "macos" ]; then
      MONGOSQL_AUTH_LIB_NAME=lib$MONGOSQL_AUTH_LIB_NAME.a
      export MYSQL_EXTRA_LIBRARIES="$MYSQL_PROJECT_DIR/bld/build/archive_output_directory/$MONGOSQL_AUTH_LIB_NAME"
    elif [ "$PLATFORM_NAME" = "windows" ]; then
      MONGOSQL_AUTH_LIB_NAME=$MONGOSQL_AUTH_LIB_NAME.lib
      export MYSQL_EXTRA_LIBRARIES="$MYSQL_PROJECT_DIR/bld/build/archive_output_directory/Release/$MONGOSQL_AUTH_LIB_NAME"
      echo "-----Copy libcrypto and libssl locally because cmake cannot handle spaces in paths"
      mkdir "$PROJECT_DIR/openssl"
      cp "$OPENSSL_PATH/lib/VC/x64/MD/libcrypto.lib" "$PROJECT_DIR/openssl/"
      cp "$OPENSSL_PATH/lib/VC/x64/MD/libssl.lib" "$PROJECT_DIR/openssl/"
      export MYSQL_EXTRA_LIBRARIES="$MYSQL_EXTRA_LIBRARIES;$PROJECT_DIR/openssl/libcrypto.lib;$PROJECT_DIR/openssl/libssl.lib"
    elif [ "$PLATFORM_NAME" = "linux" ]; then
      MONGOSQL_AUTH_LIB_NAME=lib$MONGOSQL_AUTH_LIB_NAME.a
      export MYSQL_EXTRA_LIBRARIES="$MYSQL_PROJECT_DIR/bld/build/archive_output_directory/$MONGOSQL_AUTH_LIB_NAME"
    fi

    #if [ "$PLATFORM" = macos ]; then
    #    cd "$BUILD_DIR"
    #    # unfortunately, we unzip this in two places (libmongosql and here)
    #    curl -O https://mongo-bic-odbc-driver-resources.s3.amazonaws.com/macos/openssl-1.0.2n.zip
    #    unzip openssl-1.0.2n.zip
    #    OPENSSL_PATH="$BUILD_DIR/1.0.2n"
    #    CMAKE_ARGS="$CMAKE_ARGS -DWITH_SSL=$OPENSSL_PATH -DCMAKE_VERBOSE_MAKEFILE=ON"
    #fi

    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_VERBOSE_MAKEFILE=ON"
    if [ "$PLATFORM_NAME" = "macos" ]; then
      CMAKE_ARGS="$CMAKE_ARGS -DWITH_SSL=$OPENSSL_PATH"
    fi


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
    if [ "$PLATFORM_NAME" = "macos" ]; then
        iODBC_dir=libiodbc-3.52.16
        echo "downloading iODBC https://github.com/openlink/iODBC/releases/download/v3.52.16/$iODBC_dir.tar.gz"

        curl -LO "https://github.com/openlink/iODBC/releases/download/v3.52.16/$iODBC_dir.tar.gz" \
             --silent \
             --fail \
             --max-time 60 \
             --retry 5 \
             --retry-delay 0
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
    CMAKE_ARGS="$CMAKE_ARGS $CMAKE_ICU_ARGS -DDEBUG_OUTPUT='C:\\mysql_debug.log'"

    # run CMake in the BUILD_DIR
    echo "Running cmake \"$BUILD_SRC_DIR\" -G \"$CMAKE_GENERATOR\" -DMYSQLCLIENT_STATIC_LINKING:BOOL=TRUE -DCMAKE_MODULE_PATH="$CMAKE_MODULE_PATH" $UNIX_LIB $CMAKE_ARGS"
    cmake "$BUILD_SRC_DIR" -G "$CMAKE_GENERATOR" -DMYSQLCLIENT_STATIC_LINKING:BOOL=TRUE -DCMAKE_MODULE_PATH="$CMAKE_MODULE_PATH" $UNIX_LIB $CMAKE_ARGS

    echo 'building...'
    # build the ODBC driver
    if [ "$PLATFORM_NAME" = "windows" ]; then
        BUILD='devenv.com MySQL_Connector_ODBC.sln /build Release || true'
        eval $BUILD
        #BUILD='devenv.com MySQL.sln /Build Release /Project mysqlclient'
    else
        make mdbodbca mdbodbcw
    fi

    echo 'copy artifacts'
    # copy artifacts into DRIVERS_DIR
    if [ "$PLATFORM_NAME" = "windows" ]; then
        cp "$BUILD_DIR"/lib/Release/mdbodbc{a,S,w}.{dll,lib} "$DRIVERS_DIR"
        cp "$BUILD_DIR"/bin/Release/myodbc-installer.exe "$DRIVERS_DIR"/installer.exe
        #also copy openssl
        cp "$OPENSSL_PATH"/*.dll "$DRIVERS_DIR"
    else
        cp "$BUILD_DIR"/lib/libmdbodbc{a,w}.so "$DRIVERS_DIR"
        # if this is mac, we need to update the openssl library rpaths
        # This is the important thing, this allows it to link against openssl in the same directory as
        # the driver library ('@loader_path' refers to the location of the driver library, the current
        # directory for the loader is where the library is called from, so using ./libssl.1.0.0.dylib would not
        # work).
        if [ "$PLATFORM_NAME" = "macos" ]; then
            install_name_tool -change /usr/local/opt/openssl/lib/libssl.1.0.0.dylib "@loader_path/libssl.1.0.0.dylib" "$DRIVERS_DIR"/libmdbodbca.so
            install_name_tool -change /usr/local/opt/openssl/lib/libssl.1.0.0.dylib "@loader_path/libssl.1.0.0.dylib" "$DRIVERS_DIR"/libmdbodbcw.so
            install_name_tool -change /usr/local/opt/openssl/lib/libcrypto.1.0.0.dylib "@loader_path/libcrypto.1.0.0.dylib" "$DRIVERS_DIR"/libmdbodbca.so
            install_name_tool -change /usr/local/opt/openssl/lib/libcrypto.1.0.0.dylib "@loader_path/libcrypto.1.0.0.dylib" "$DRIVERS_DIR"/libmdbodbcw.so
        fi
    fi
) > $LOG_FILE 2>&1

print_exit_msg
