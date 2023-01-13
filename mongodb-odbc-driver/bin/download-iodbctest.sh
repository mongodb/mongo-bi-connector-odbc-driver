#!/usr/bin/env bash
#shellcheck source=./prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

(
    echo 'downloading iodbctestw'
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    curl -O https://mongo-bic-odbc-driver-resources.s3.amazonaws.com/macos/iodbctest.zip
    unzip iodbctest.zip
    chmod 777 iodbctest/*
    cd -
) > $LOG_FILE 2>&1

print_exit_msg
