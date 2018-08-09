#!/bin/bash

# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

(
    test_set=''
    if [ "$TEST_SET" = 'SSL' ]; then
        test_set='LocalSSL'
        success_file_prefix="$SCRIPT_DIR/local_ssl-integration-test-success-cases"
        "$SCRIPT_DIR"/gen_tsv.py "$success_file_prefix".yml > "$success_file_prefix".tsv
        fail_file_prefix="$SCRIPT_DIR/local_ssl-integration-test-fail-cases"
        "$SCRIPT_DIR"/gen_tsv.py "$fail_file_prefix".yml > "$fail_file_prefix".tsv
    elif [ "$TEST_SET" = '' ]; then
        test_set='Local'
        success_file_prefix="$SCRIPT_DIR/local-integration-test-success-cases"
        "$SCRIPT_DIR"/gen_tsv.py "$success_file_prefix".yml > "$success_file_prefix".tsv
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
            -File "$SCRIPT_DIR/run-windows-integration-tests.ps1" \
            -"$test_set" \
            -Platform "$platform" \
            -Server '127.0.0.1' \
            -Port '3307' \
            -User 'user_not_used' \
            -Password 'password_not_used' \
            -DB 'information_schema'
        echo 'integration tests passed'

    elif [ "macos" = "$PLATFORM" ]; then
        if ! hash iodbctestw &> /dev/null; then
            echo 'building iODBC'
            "$SCRIPT_DIR"/build-iodbc.sh
        fi
        echo 'running local connection tests...'

        "$SCRIPT_DIR"/run-unix-integration-tests.sh \
            "iodbctestw" \
            "DSN=" \
            "local" \
            "127.0.0.1" \
            "3307" \
            "user_not_used" \
            "password_not_used"
    else
        iusql_bin="$BUILD_DIR"/install/bin/iusql
        if [ ! -f "$iusql_bin" ]; then
            echo 'building unixODBC'
            "$SCRIPT_DIR"/build-unixodbc.sh
        fi
        echo 'running local connection tests...'

        "$SCRIPT_DIR"/run-unix-integration-tests.sh \
            "$iusql_bin" \
            "" \
            "local" \
            "127.0.0.1" \
            "3307" \
            "user_not_used" \
            "password_not_used"
    fi
) > $LOG_FILE 2>&1

print_exit_msg

