#!/bin/bash

# shellcheck source=prepare-shell.sh
. "$(dirname "$0")/prepare-shell.sh"

pkill mongod || true

echo 'downloading mongodb...'
rm -rf "$MONGODB_DIR"
mkdir -p "$MONGODB_DIR"
cd "$MONGODB_DIR"
curl -o mongodb.archive "$MONGODB_URI"
echo 'downloaded mongodb'

echo 'starting mongodb...'
$EXTRACT mongodb.archive '*/bin/mongod*'
mv ./**/bin .
rm -rf mongodb*
mkdir -p data
chmod +x ./bin/mongod
./bin/mongod -v --dbpath data --logpath mongod.log &
echo 'started mongodb'
