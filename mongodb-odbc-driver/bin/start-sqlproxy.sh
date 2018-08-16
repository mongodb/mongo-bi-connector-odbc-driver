#!/usr/bin/env bash

# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

(
    SQLPROXY_ARGS="-vv --logPath $SQLPROXY_DIR/mongosqld.log"
    if [ "$TEST_SET" = 'SSL' ]; then
        SQLPROXY_ARGS="$SQLPROXY_ARGS --sslMode allowSSL --sslPEMKeyFile $PROJECT_ROOT/mongodb-odbc-driver/resources/server.pem"
    fi

    if [ "$PLATFORM_NAME" = "windows" ]; then
        echo 'stopping and deleting existing sqlproxy service...'
        echo
        net stop mongosql || true
        sc.exe delete mongosql || true
        reg delete 'HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\EventLog\Application\mongosql' /f || true
        echo 'stopped and deleted existing sqlproxy service'
        echo

        echo 'downloading sqlproxy...'
        echo
        rm -rf "$SQLPROXY_DIR"
        mkdir -p "$SQLPROXY_DIR"
        cd "$SQLPROXY_DIR"
        curl -o sqlproxy.msi "$SQLPROXY_URI"
        echo 'downloaded sqlproxy'
        echo

        echo 'installing sqlproxy...'
        echo
        msiexec /i sqlproxy /qn /log mongosql-install.log
        sleep 3
        echo 'installed sqlproxy'
        echo

        echo 'installing and starting sqlproxy service...'
        echo
        LATEST_DIR=$(ls -v /cygdrive/c/Program\ Files/MongoDB/Connector\ for\ BI/ | tail -n 1)
        cd /cygdrive/c/Program\ Files/MongoDB/Connector\ for\ BI/"$LATEST_DIR"/bin
        ./mongosqld.exe install $SQLPROXY_ARGS
        echo 'installed, starting...'
        echo
        net start mongosql
        echo 'started sqlproxy service'
        echo
    else
        echo 'killing existing sqlproxy service...'
        echo
        killall mongosqld || true
        echo 'downloading sqlproxy...'
        echo
        rm -rf "$SQLPROXY_DIR"
        mkdir -p "$SQLPROXY_DIR"
        cd "$SQLPROXY_DIR"
        curl -o sqlproxy.tgz "$SQLPROXY_URI"
        $EXTRACT sqlproxy.tgz
        # remove the tgz so that there will only be one file in the
        # directory (the extracted directory)
        rm sqlproxy.tgz
        # there is only one file in the directory (the extracted directory),
        # so we can use *
        cd */bin
        echo 'starting sqlproxy'
        echo
        nohup mongosqld $SQLPROXY_ARGS < /dev/null &> /dev/null &
        echo 'started sqlproxy'
        echo
    fi

    echo 'sleeping so that mongosqld can generate schema...'
    echo
    sleep 5
    echo 'done sleeping'
    echo
) > $LOG_FILE 2>&1

print_exit_msg
