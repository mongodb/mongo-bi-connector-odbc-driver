#!/bin/sh

SCRIPT_DIR=$(dirname $(readlink -f $0))
ROOT=$SCRIPT_DIR/..
S3_URL=https://s3.amazonaws.com/mongo-odbc-build-scratch/mysql-5.7.21-winx64.zip
DL_DIR=$BUILD_DIR/mysql-5.7.21-winx64
ZIP_FILE=mysql-64.zip
BUILD_DIR=$ROOT/build-win64

mkdir -p $BUILD_DIR
DBC_DIR=$ROOT/src/mongo-odbc-driver/mysql-connector-odbc

export PATH='/cygdrive/c/cmake/bin':'/cygdrive/c/Program Files (x86)/Microsoft Visual Studio 12.0/Common7/IDE':'/cygdrive/c/wixtools/bin':$PATH
export MYSQL_DIR=$(cygpath -w $DL_DIR)

cd $BUILD_DIR

if [ ! -e $DL_DIR ]; then
    curl $S3_URL --output $ZIP_FILE
    unzip $ZIP_FILE
fi

cd $ODBC_DIR

cmake -G "Visual Studio 12 2013 Win64" -DMYSQLCLIENT_STATIC_LINKING:BOOL=TRUE

devenv.com MySQL_Connector_ODBC.sln /build Release

cd $BUILD_DIR

#we use cp -R because symlinks are problematic on cygwin
cp -R $ROOT/src/mongo-odbc-driver/installer/msi/win32_installer/* ./

#copy relevant files to installer dir
FILE_DIR=$ODBC_DIR/lib/Release
DEST_DIR=$BUILD_DIR/Files/File

cp $FILE_DIR/myodbc5a.dll $DEST_DIR/myodbc5a.dll
cp $FILE_DIR/myodbc5S.dll $DEST_DIR/myodbc5S.dll
cp $FILE_DIR/myodbc5w.dll $DEST_DIR/myodbc5w.dll
cp $FILE_DIR/myodbc5S.lib $DEST_DIR/myodbc5S.lib
cp $FILE_DIR/myodbc5a.lib $DEST_DIR/myodbca.lib
cp $FILE_DIR/myodbc5w.lib $DEST_DIR/myodbcw.lib

candle odbc.wxs
light odbc.wixobj
