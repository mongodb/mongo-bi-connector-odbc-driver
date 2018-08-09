#!/bin/bash
. "$(dirname "$0")/prepare-shell.sh"

mkdir -p "$BUILD_DIR"/install
cd "$BUILD_DIR"
echo "downloading unixODBC"
unixODBC_dir=unixODBC-2.3.6
curl -O "http://noexpire.s3.amazonaws.com/sqlproxy/binary/linux/unixODBC-2.3.6.tar.gz"
tar xf "$unixODBC_dir.tar.gz"
cd "$unixODBC_dir"
./configure --prefix="$BUILD_DIR"/install
make
make install
