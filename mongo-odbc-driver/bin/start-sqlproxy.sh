#!/bin/bash

# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

SQLPROXY_ARGS="-vv --logPath $SQLPROXY_DIR/mongosqld.log"
if [ "$TEST_SET" = 'SSL' ]; then
    SQLPROXY_ARGS="$SQLPROXY_ARGS --sslMode allowSSL --sslPEMKeyFile $PROJECT_ROOT/mongo-odbc-driver/resources/server.pem"
fi

echo "stopping and deleting existing sqlproxy service..."
net stop mongosql || true
sc.exe delete mongosql || true
reg delete 'HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\EventLog\Application\mongosql' /f || true
echo "stopped and deleted existing sqlproxy service"

echo 'downloading sqlproxy...'
rm -rf "$SQLPROXY_DIR"
mkdir -p "$SQLPROXY_DIR"
cd "$SQLPROXY_DIR"
curl -o sqlproxy.msi "$SQLPROXY_URI"
echo 'downloaded sqlproxy'

echo 'installing sqlproxy...'
msiexec /i sqlproxy /qn /log mongosql-install.log
echo 'installed sqlproxy'

echo 'installing and starting sqlproxy service...'
cd /cygdrive/c/Program\ Files/MongoDB/Connector\ for\ BI/2.4/bin
pwd
./mongosqld.exe install $SQLPROXY_ARGS
echo 'installed, starting...'
net start mongosql
echo 'started sqlproxy service'

echo 'sleeping so that mongosqld can generate schema...'
sleep 5
echo 'done sleeping'
