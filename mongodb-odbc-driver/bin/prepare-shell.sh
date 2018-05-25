#!/bin/bash
# Copyright (c) 2018-Present MongoDB Inc.
# This file should be sourced by all scripts in mongodb-odbc-driver/bin

# we start by sourcing platforms.sh. this will set environment variables that
# differ depending on which platform we are building on
# shellcheck source=platforms.sh
. "$(dirname "$0")/platforms.sh"

# create variables for a number of useful directories
SCRIPT_DIR=$(cd $(dirname $0) && pwd -P)
if [ "$OS" = "Windows_NT" ]; then
    SCRIPT_DIR="$(cygpath -m "$SCRIPT_DIR")"
fi
export MDBODBC_VER="$(cat "$SCRIPT_DIR"/VERSION.txt)"
PROJECT_ROOT="$SCRIPT_DIR/../.."
BUILD_DIR="$PROJECT_ROOT/mongodb-odbc-driver/build"
BUILD_SRC_DIR="$PROJECT_ROOT/mongodb-odbc-driver/src"
MSI_BUILD_DIR="$PROJECT_ROOT/mongodb-odbc-driver/msi-build"
DMG_BUILD_DIR="$PROJECT_ROOT/mongodb-odbc-driver/dmg-build"
ARTIFACTS_DIR="$PROJECT_ROOT/mongodb-odbc-driver/artifacts"
DRIVERS_DIR="$ARTIFACTS_DIR/drivers"
PKG_DIR="$ARTIFACTS_DIR/pkg"
SQLPROXY_DIR="$ARTIFACTS_DIR/mongosqld"
MONGODB_DIR="$ARTIFACTS_DIR/mongodb"
MYSQL_PROJECT_DIR="$PROJECT_ROOT/mongodb-odbc-driver/libmongosql"
MYSQL_SCRIPT_DIR="$MYSQL_PROJECT_DIR/bld/bin"
MYSQL_DIR="$MYSQL_PROJECT_DIR/bld/artifacts/mysql-home"
MONGOSQL_AUTH_PROJECT_DIR="$MYSQL_PROJECT_DIR/bld/mongosql-auth-c"
CMAKE_MODULE_PATH="$BUILD_SRC_DIR/cmake"
export IODBC_VERSION=iODBC-3.52.12
export IODBC_BUILD_DIR="$BUILD_DIR"/"$IODBC_VERSION"/mac
IODBCTEST_PATH="$IODBC_BUILD_DIR"/iODBCtest/build/Deployment
IODBCTESTW_PATH="$IODBC_BUILD_DIR"/iODBCtestw/build/Deployment

PATH="$PATH:$DEVENV_PATH:$CMAKE_PATH:$WIX_PATH:$IODBCTEST_PATH:$IODBCTESTW_PATH"

# export any environment variables that will be needed by subprocesses
export CMAKE_MODULE_PATH
export MYSQL_DIR

# Each script should run with errexit set and should start in the project root.
# In general, scripts should reference directories via the provided environment
# variables instead of making assumptions about the working directory.
set -o errexit
cd "$PROJECT_ROOT"
