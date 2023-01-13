#!/usr/bin/env bash
# Copyright (c) 2018-Present MongoDB Inc.
# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

(
    success_file_prefix="$SCRIPT_DIR/atlas-integration-test-success-cases"
    "$SCRIPT_DIR"/gen_tsv.py "$success_file_prefix".yml > "$success_file_prefix".tsv
    fail_file_prefix="$SCRIPT_DIR/atlas-integration-test-fail-cases"
    "$SCRIPT_DIR"/gen_tsv.py "$fail_file_prefix".yml > "$fail_file_prefix".tsv

    if [ "Windows_NT" = "$OS" ]; then
        cd "$PKG_DIR"
        if [ "$PLATFORM_ARCH" = "64" ]; then
            platform="64-bit"
        else
            platform="32-bit"
        fi

        echo 'running atlas connection tests...'

        # 32-bit powershell is configured to disallow scripts by default.
        # This invocation allows us to temporarily bypass that restriction.
        # https://docs.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_execution_policies?view=powershell-6
        "$POWERSHELL" \
            -ExecutionPolicy ByPass \
            -NoProfile \
            -NoLogo \
            -NonInteractive \
            -File "$SCRIPT_DIR/run-windows-integration-tests.ps1" \
            -Atlas \
            -Platform "$platform" \
            -Server "$BIC_PROD_SERVER" \
            -Port "${BIC_PROD_PORT:-27015}" \
            -User "$BIC_PROD_USER" \
            -Password "$BIC_PROD_PASSWORD" \
            -Version $(< "$SCRIPT_DIR/VERSION.txt")

    elif [ "macos" = "$PLATFORM" ]; then
        if ! hash iodbctestw &> /dev/null; then
            "$SCRIPT_DIR/download-iodbctest.sh"
        fi
        echo 'running atlas connection tests...'

        "$SCRIPT_DIR"/run-unix-integration-tests.sh \
            "iodbctestw" \
            "DSN=" \
            "atlas" \
            "$BIC_PROD_SERVER" \
            "$BIC_PROD_PORT" \
            "$BIC_PROD_USER" \
            "$BIC_PROD_PASSWORD"
    else
        iusql_bin="$BUILD_DIR"/install/bin/iusql
        if [ ! -f "$iusql_bin" ]; then
            echo 'building unixODBC'
            "$SCRIPT_DIR"/build-unixodbc.sh
        fi
        echo 'running atlas connection tests...'

        "$SCRIPT_DIR"/run-unix-integration-tests.sh \
            "$iusql_bin" \
            "" \
            "atlas" \
            "$BIC_PROD_SERVER" \
            "$BIC_PROD_PORT" \
            "$BIC_PROD_USER" \
            "$BIC_PROD_PASSWORD"
    fi
) > $LOG_FILE 2>&1

print_exit_msg
