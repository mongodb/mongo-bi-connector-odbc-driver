#!/bin/sh
# The default path should be /usr/local

# Get some info from configure
# chmod +x ./scripts/setsomevars

machine=@MACHINE_TYPE@
system=@SYSTEM_TYPE@
version=@VERSION@
export machine system version
SOURCE=`pwd` 
CP="cp -p"
RM=rm
MV=mv
CHMOD=chmod
SED=sed

DEBUG=0
TMP=/tmp
SILENT=0


# This script must run from MyODBC top directory
if ! test -f driver/myodbc3.c
then
  echo "ERROR : You must run this script from the MyODBC top-level directory"
  exit 1
fi

parse_arguments() {
  for arg do
    case "$arg" in
      --debug)    DEBUG=1;;
      --tmp=*)    TMP=`echo "$arg" | sed -e "s;--tmp=;;"` ;;
      --silent)   SILENT=1 ;;
      *)
  echo "Unknown argument '$arg'"
  exit 1
        ;;
    esac
  done
}

parse_arguments "$@"


# This should really be integrated with automake and not duplicate the
# installation list.

BASE=$TMP/mysql-connector-odbc-3.51

if [ -d $BASE ] ; then
 rm -r -f $BASE
fi

mkdir $BASE

for i in ChangeLog COPYING EXCEPTIONS README odbc.ini
do
  if [ -f $i ]
  then
    $CP $i $BASE
  fi
done

for i in driver/.libs/*
do
  if [ -f $i ]
  then
    $CP $i $BASE
  fi
done

# for i in driver_r/.libs/*
# do
#   if [ -f $i ]
#   then
#     $CP $i $BASE
#   fi
# done

if test -f $BASE/libmyodbc3-$version.so
then
  $RM -f $BASE/libmyodbc3.so
  cd $BASE
  ln -s libmyodbc3-$version.so libmyodbc3.so
  cd $SOURCE
fi

if test -f $BASE/libmyodbc3_r-$version.so
then
  $RM -f $BASE/libmyodbc3_r.so
  cd $BASE
  ln -s libmyodbc3_r-$version.so libmyodbc3_r.so
  cd $SOURCE
fi


# Change the distribution to a long descriptive name
NEW_NAME=mysql-connector-odbc-$version-$system-$machine
BASE2=$TMP/$NEW_NAME
$RM -r -f $BASE2
$MV $BASE $BASE2
BASE=$BASE2

#if we are debugging, do not do tar/gz
if [ x$DEBUG = x1 ] ; then
 echo "Debug mode, no archiving, check the files from $BASE"
 echo "`ls -al $BASE`"
 echo "exiting..."
 exit
fi

# This is needed to prefere gnu tar instead of tar because tar can't
# always handle long filenames

PATH_DIRS=`echo $PATH | $SED -e 's/^:/. /' -e 's/:$/ ./' -e 's/::/ . /g' -e 's/:/ /g' `
which_1 ()
{
  for cmd
  do
    for d in $PATH_DIRS
    do
      for file in $d/$cmd
      do
	if test -x $file -a ! -d $file
	then
	  echo $file
	  exit 0
	fi
      done
    done
  done
  exit 1
}

#
# Create the result tar file
#

tar=`which_1 gtar`
if test "$?" = "1" -o "$tar" = ""
then
  tar=tar
fi

echo "Using $tar to create archive"
cd $TMP

OPT=cvf
if [ x$SILENT = x1 ] ; then
  OPT=cf
fi

if test -f $SOURCE/$NEW_NAME.tar.gz
then
  echo "Cleaning the old file..."
  $RM -f $SOURCE/$NEW_NAME.tar.gz
fi

$tar $OPT $SOURCE/$NEW_NAME.tar $NEW_NAME
cd $SOURCE
echo "Compressing archive"
gzip -9 $NEW_NAME.tar
echo "Removing temporary directory"
rm -r -f $BASE

echo "$NEW_NAME.tar.gz created"
