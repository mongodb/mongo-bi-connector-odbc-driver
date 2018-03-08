#!/bin/sh

SCRIPT_DIR=$(dirname $(readlink -f $0))
BUILD_DIR=$SCRIPT_DIR/../build-win32
mkdir -p $BUILD_DIR
S3_URL=https://s3.amazonaws.com/mongo-odbc-build-scratch/mysql-5.7.21-win32.zip
DL_DIR=$BUILD_DIR/mysql-5.7.21-win32
ZIP_FILE=mysql-32.zip
ODBC_DIR=mysql-connector-odbc

export PATH='/cygdrive/c/cmake/bin':'/cygdrive/c/Program Files (x86)/Microsoft Visual Studio 12.0/Common7/IDE':'/cygdrive/c/wixtools/bin':$PATH
export MYSQL_DIR=$(cygpath -w $DL_DIR)

cd $BUILD_DIR

if [ ! -e $DL_DIR ]; then
    curl $S3_URL --output $ZIP_FILE
    unzip $ZIP_FILE
fi

if [ ! -e $ODBC_DIR ]; then
    git clone https://github.com/mysql/mysql-connector-odbc.git
fi

cd $ODBC_DIR

#configure build
cmake -G "Visual Studio 12 2013" -DMYSQLCLIENT_STATIC_LINKING:BOOL=TRUE

#build
devenv.com MySQL_Connector_ODBC.sln /build Release

cd ..

#we use cp -R because symlinks are problematic on cygwin
cp -R ../resources/win32_installer/* ./

#copy relevant files to installer installer dir
FILE_DIR=$BUILD_DIR/mysql-connector-odbc/lib/Release
DEST_DIR=$BUILD_DIR/Files/File

cp $FILE_DIR/myodbc5a.dll $DEST_DIR/myodbc5a.dll
cp $FILE_DIR/myodbc5S.dll $DEST_DIR/myodbc5S.dll
cp $FILE_DIR/myodbc5w.dll $DEST_DIR/myodbc5w.dll
cp $FILE_DIR/myodbc5S.lib $DEST_DIR/myodbc5S.lib
cp $FILE_DIR/myodbc5a.lib $DEST_DIR/myodbca.lib
cp $FILE_DIR/myodbc5w.lib $DEST_DIR/myodbcw.lib

candle odbc.wxs
light odbc.wixobj
