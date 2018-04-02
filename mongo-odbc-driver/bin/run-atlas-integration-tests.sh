#!/bin/bash
# Copyright (c) 2018-Present MongoDB Inc.
# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

cd "$PKG_DIR"

if [ "Windows_NT" = "$OS" ]; then

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
        -File "$SCRIPT_DIR/run-integration-tests.ps1" \
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
        -File "$SCRIPT_DIR/run-integration-tests.ps1" \
        -Atlas \
        -Platform "$platform" \
        -Server "$BIC_DEV_SERVER" \
        -Port "${BIC_DEV_PORT:-27015}" \
        -User "$BIC_DEV_USER" \
        -Password "$BIC_DEV_PASSWORD"

    echo 'integration tests passed'

else
    echo 'integration tests not yet implemented for non-windows platforms'
    exit 1
fi
