#!/usr/bin/env bash
# Copyright (c) 2018-Present MongoDB Inc.
# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"


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

    echo 'running atlas prod connection tests...'

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
        -Password "$BIC_PROD_PASSWORD"

    echo 'running atlas dev connection tests...'

    "$POWERSHELL" \
        -ExecutionPolicy ByPass \
        -NoProfile \
        -NoLogo \
        -NonInteractive \
        -File "$SCRIPT_DIR/run-windows-integration-tests.ps1" \
        -Atlas \
        -Platform "$platform" \
        -Server "$BIC_DEV_SERVER" \
        -Port "${BIC_DEV_PORT:-27015}" \
        -User "$BIC_DEV_USER" \
        -Password "$BIC_DEV_PASSWORD"

    echo 'integration tests passed'

elif [ "macos" = "$PLATFORM" ]; then
    if ! hash iodbctestw &> /dev/null; then
        echo 'building iODBC'
        "$SCRIPT_DIR"/build-iodbc.sh
    fi
    echo 'running atlas prod connection tests...'

    "$SCRIPT_DIR"/run-macos-integration-tests.sh \
        "atlas" \
        "$BIC_PROD_SERVER" \
        "$BIC_PROD_PORT" \
        "$BIC_PROD_USER" \
        "$BIC_PROD_PASSWORD"

    echo 'running atlas dev connection tests...'

    "$SCRIPT_DIR"/run-macos-integration-tests.sh \
        "atlas" \
        "$BIC_DEV_SERVER" \
        "$BIC_DEV_PORT" \
        "$BIC_DEV_USER" \
        "$BIC_DEV_PASSWORD"
else
    echo 'integration tests not yet implemented for linux platforms'
    exit 1
fi
