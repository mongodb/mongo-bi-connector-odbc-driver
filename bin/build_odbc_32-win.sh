#!/usr/bin/sh

S3_URL=https://s3.amazonaws.com/mongo-odbc-build-scratch/mysql-5.7.21-win32.zip
DL_DIR=$(pwd)/mysql-5.7.21-win32/
ZIP_FILE=mysql-32.zip
ODBC_DIR=mysql-connector-odbc

export PATH='/cygdrive/c/cmake/bin':'/cygdrive/c/Program Files (x86)/Microsoft Visual Studio 12.0/Common7/IDE':$PATH
export MYSQL_DIR=$(cygpath -w $DL_DIR)

if [ ! -e $DL_DIR ]; then
    curl $S3_URL --output $ZIP_FILE
    unzip $ZIP_FILE
fi

if [ ! -e $ODBC_DIR ]; then
    git clone https://github.com/mysql/mysql-connector-odbc.git
fi

cd $ODBC_DIR

cmake -G "Visual Studio 12 2013" -DMYSQLCLIENT_STATIC_LINKING:BOOL=TRUE

devenv.com MySQL_Connector_ODBC.sln /build Release
