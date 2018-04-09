#!/bin/bash
# Copyright (c) 2018-Present MongoDB Inc.
# This file should be sourced by all scripts in mongo-odbc-driver/bin

# we start by sourcing platforms.sh. this will set environment variables that
# differ depending on which platform we are building on
# shellcheck source=platforms.sh
. "$(dirname "$0")/platforms.sh"

# create variables for a number of useful directories
SCRIPT_DIR=$(dirname $(readlink -f $0))
if [ "$OS" = "Windows_NT" ]; then
    SCRIPT_DIR="$(cygpath -m "$SCRIPT_DIR")"
fi
PROJECT_ROOT="$SCRIPT_DIR/../.."
BUILD_DIR="$PROJECT_ROOT/mongo-odbc-driver/build"
BUILD_SRC_DIR="$PROJECT_ROOT/mongo-odbc-driver/src"
MSI_BUILD_DIR="$PROJECT_ROOT/mongo-odbc-driver/msi-build"
ARTIFACTS_DIR="$PROJECT_ROOT/mongo-odbc-driver/artifacts"
DRIVERS_DIR="$ARTIFACTS_DIR/drivers"
PKG_DIR="$ARTIFACTS_DIR/pkg"
MYSQL_PROJECT_DIR="$PROJECT_ROOT/mongo-odbc-driver/mysql-server"
MYSQL_SCRIPT_DIR="$MYSQL_PROJECT_DIR/bld/bin"
MYSQL_DIR="$MYSQL_PROJECT_DIR/bld/artifacts/mysql-home"
MONGOSQL_AUTH_PROJECT_DIR="$MYSQL_PROJECT_DIR/bld/mongosql-auth-c"

# set the CMake generator
CMAKE_GENERATOR="Visual Studio 14 2015"
if [ "$PLATFORM_ARCH" = "64" ]; then
    CMAKE_GENERATOR="$CMAKE_GENERATOR Win64"
fi

# make sure binaries we use in our scripts are available in the PATH
DEVENV_PATH='/cygdrive/c/Program Files (x86)/Microsoft Visual Studio 14.0/Common7/IDE'
CMAKE_PATH='/cygdrive/c/cmake/bin'
WIX_PATH='/cygdrive/c/wixtools/bin'
PATH="$PATH:$DEVENV_PATH:$CMAKE_PATH:$WIX_PATH"

# export any environment variables that will be needed by subprocesses
export MYSQL_DIR
export PATH

# Each script should run with errexit set and should start in the project root.
# In general, scripts should reference directories via the provided environment
# variables instead of making assumptions about the working directory.
set -o errexit
cd "$PROJECT_ROOT"
