#!/usr/bin/env bash

# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

(
    echo "-------- TEST_SET is $TEST_SET"
    echo " ------ ENV -----------"
    env
    echo " ----------------------"
    SQLPROXY_ARGS="-vvv --logPath $SQLPROXY_DIR/mongosqld.log"
    if [ "$TEST_SET" = 'SSL' ]; then
        SQLPROXY_ARGS="$SQLPROXY_ARGS --sslMode requireSSL --minimumTLSVersion=TLS1_2 --sslPEMKeyFile $PROJECT_ROOT/mongodb-odbc-driver/resources/server.pem"
    elif [ "$TEST_SET" = 'GSSAPI' ]; then
        export KRB5_KTNAME=$PROJECT_DIR/mongodb-odbc-driver/resources/gssapi/mongosqlsvc.keytab
        klist -k $KRB5_KTNAME
        SQLPROXY_ARGS="$SQLPROXY_ARGS --mongo-authenticationMechanism GSSAPI --auth --mongo-username drivers@LDAPTEST.10GEN.CC --mongo-password powerbook17 --gssapiHostname localhost --gssapiServiceName mongosql2 --mongo-uri mongodb://ldaptest.10gen.cc:27017 --sampleNamespaces kerberos.test  --addr 127.0.0.1 --schemaMappingMode majority"
    fi

    if [ "$PLATFORM_NAME" = "windows" ]; then
        echo 'stopping and deleting existing sqlproxy service...'
        echo
        net stop mongosql || true
        sc.exe delete mongosql || true
        reg delete 'HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\EventLog\Application\mongosql' /f || true
        echo 'stopped and deleted existing sqlproxy service'
        echo

        echo 'downloading sqlproxy $SQLPROXY_URI...'
        echo
        rm -rf "$SQLPROXY_DIR"
        mkdir -p "$SQLPROXY_DIR"
        cd "$SQLPROXY_DIR"
        curl -o sqlproxy.msi "$SQLPROXY_URI"
        echo 'downloaded sqlproxy'
        echo

        echo 'uninstalling sqlproxy...'
        echo
        msiexec /x sqlproxy /qn /log $ARTIFACTS_DIR/log/mongosql-uninstall.log || true
        echo 'uninstalled sqlproxy'
        echo

        echo 'installing sqlproxy...'
        echo
        msiexec /i sqlproxy /qn /log $ARTIFACTS_DIR/log/mongosql-install.log
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
        echo 'downloading sqlproxy $SQLPROXY_URI...'
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
        # sqlproxy also has no idea where the openssl libs are on macos spawns hosts because we decided to move them at
        # some point.
        if [ "$PLATFORM_NAME" = "macos" ]; then
            curl -O https://mongo-bic-odbc-driver-resources.s3.amazonaws.com/macos/openssl-1.0.2n.zip
            unzip openssl-1.0.2n.zip
            chmod 777 ./1.0.2n/lib/*
            mv ./1.0.2n/lib/* ./
            install_name_tool -change /usr/local/opt/openssl/lib/libssl.1.0.0.dylib "@loader_path/libssl.1.0.0.dylib"  mongosqld
            install_name_tool -change /usr/local/opt/openssl/lib/libcrypto.1.0.0.dylib "@loader_path/libcrypto.1.0.0.dylib"  mongosqld
            # we also need to patch up libssl, which I should have just done before uploading to s3, but I
            # did not.
            install_name_tool -change /usr/local/Cellar/openssl/1.0.2n/lib/libcrypto.1.0.0.dylib "@loader_path/libcrypto.1.0.0.dylib" libssl.1.0.0.dylib
        fi
        echo "printing mongosqld args..."
        echo "$SQLPROXY_ARGS"
        echo "starting mongosqld..."
        mongosqld $SQLPROXY_ARGS &> sqlproxy.log &
        echo 'started sqlproxy'

    fi

    echo 'sleeping so that mongosqld can generate schema...'
    echo
    sleep 5
    echo 'done sleeping'
    echo

) > $LOG_FILE 2>&1

print_exit_msg
