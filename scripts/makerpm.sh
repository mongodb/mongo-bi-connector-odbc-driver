#!/bin/sh

#
# Configure function
#

function configure_driver
{
  make -f Makefile.bk
  ./configure \
    --prefix=${RPM_BUILD_ROOT}%{prefix} \
    --with-mysql-libs=%{MYSQL_LIBS} \
    --with-mysql-includes=%{MYSQL_INCLUDES} \
    ${ODBC_DM_PATH_ARG} \
    --enable-shared=yes \
    --enable-static=yes \
    --without-debug
  make dist;
}

#
# mysql-connector-odbc 3.51 rpm build script
#

RELEASE=@VERSION@
SPEC_FILE=mysql-connector-odbc-@VERSION@.spec
ARCH=`uname -m | perl -p -e 's/i[0-9]{3}/i386/'`
SOURCE=mysql-connector-odbc

#
# This script must run from MyODBC top directory
#

if [ ! -f "./driver/myodbc3.c" ]; 
then
  echo "ERROR : You must run this script from the MyODBC top-level directory"
  exit 1
fi

ODBC_DM_PATH_ARG="@ODBC_DM_PATH_ARG@"
MYSQL_LIBS="@MYSQL_USED_LIB_PATH@"
MYSQL_INCLUDES="@MYSQL_USED_INCLUDE_PATH@"
MyODBC_ROOT=`pwd`

#
# Find the RPM build area
#

if [ -d /usr/src/redhat ]
then
    RPM_BUILD_AREA=/usr/src/redhat
else if [ -d /usr/src/packages ]
then
    RPM_BUILD_AREA=/usr/src/packages
fi
fi

#
# setup the src tar ball
#

SRC_FILE=$SOURCE-${RELEASE}
SRC_TAR_ARCH=$SRC_FILE.tar.gz

if [ ! -f "$SRC_TAR_ARCH" ]; 
then
  echo "Tarball file '$SRC_TAR_ARCH' does not exist, please make one first"
  exit 1
fi

if [ ! -f "./scripts/${SPEC_FILE}" ]; 
then
  echo "Spec file '${SPEC_FILE}' does not exist, please make one first"
  exit 1
fi

echo "Building RPM for mysql-connector-odbc version ${RELEASE}"

cp ./scripts/${SPEC_FILE} ${RPM_BUILD_AREA}/SPECS/
cp $SRC_TAR_ARCH ${RPM_BUILD_AREA}/SOURCES/

cd ${RPM_BUILD_AREA}/SPECS

#
# Generate RPMs
#

echo "Running the spec file from '${RPM_BUILD_AREA}/SPECS'"

rpmbuild -ba ${SPEC_FILE}
if test $? -eq 0 
then
  ln -s ${RPM_BUILD_AREA}/RPMS/${ARCH}/$SRC_FILE-1.${ARCH}.rpm $SRC_FILE-1.${ARCH}.rpm
  ln -s ${RPM_BUILD_AREA}/SRPMS/$SRC_FILE-1.src.rpm $SRC_FILE-1.src.rpm
else
  echo "ERROR: rpm build failed for '$SRC_FILE'"
fi

