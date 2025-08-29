#!/bin/bash

# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

(
    venv='venv'
    if [ "$VARIANT" = 'centos6-perf' ]; then
      venv="$PROJECT_DIR/../../../../venv"
    fi

    PYTHON_EXEC=python3
    if command -v python3 >/dev/null 2>&1; then
        PYTHON_EXEC=python3
    elif command -v python >/dev/null 2>&1; then
        PYTHON_EXEC=python
    fi

    # Setup or use the existing virtualenv for mtools
    if [ -f "$venv"/bin/activate ]; then
      echo 'using existing virtualenv'
      . "$venv"/bin/activate
    elif [ -f "$venv"/Scripts/activate ]; then
      echo 'using existing virtualenv'
      . "$venv"/Scripts/activate
    elif "$PYTHON_EXEC" -m venv "$venv" || virtualenv "$venv" || "$PYTHON_EXEC" -m virtualenv "$venv"; then
      echo 'creating new virtualenv'
      if [ -f "$venv"/bin/activate ]; then
        . "$venv"/bin/activate
      elif [ -f "$venv"/Scripts/activate ]; then
        . "$venv"/Scripts/activate
      fi
    fi

    echo 'installing yaml...'
    pip install PyYAML

    test_set_type=''
    if [ "$TEST_SET" = 'SSL' ]; then
        test_set_type='LocalSSL'
        success_file_prefix="$SCRIPT_DIR/local_ssl-integration-test-success-cases"
        "$SCRIPT_DIR"/gen_tsv.py "$success_file_prefix".yml > "$success_file_prefix".tsv
        echo "--------------- $success_file_prefix.tsv for LocalSSL-------------"
        less "$success_file_prefix".tsv
        echo "------------------------------------------------------"
        fail_file_prefix="$SCRIPT_DIR/local_ssl-integration-test-fail-cases"
        "$SCRIPT_DIR"/gen_tsv.py "$fail_file_prefix".yml > "$fail_file_prefix".tsv
        echo "--------------- $success_file_prefix.tsv for LocalSSL-------------"
        less "$fail_file_prefix".tsv
        echo "------------------------------------------------------"
    elif [ "$TEST_SET" = '' ]; then
        test_set_type='Local'
        success_file_prefix="$SCRIPT_DIR/local-integration-test-success-cases"
        "$SCRIPT_DIR"/gen_tsv.py "$success_file_prefix".yml > "$success_file_prefix".tsv
        less "$success_file_prefix".tsv
        echo "--------------- $success_file_prefix.tsv -for Local------------"
        less "$success_file_prefix".tsv
        echo "------------------------------------------------------"
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
            -"$test_set_type" \
            -Platform "$platform" \
            -Server '127.0.0.1' \
            -Port '3307' \
            -User 'user_not_used' \
            -Password 'password_not_used' \
            -Version $(< "$SCRIPT_DIR/VERSION.txt") \
            -DB 'information_schema'

        echo 'integration tests passed'

    elif [ "macos" = "$PLATFORM" ]; then
        if ! hash iodbctestw &> /dev/null; then
            "$SCRIPT_DIR/download-iodbctest.sh"
        fi
        echo 'running local connection tests...'

        "$SCRIPT_DIR"/run-unix-integration-tests.sh \
            iodbctestw \
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
