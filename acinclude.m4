dnl ---------------------------------------------------------
dnl
dnl Macro: 
dnl     AC_FIND_FILE
dnl 
dnl Arguments:
dnl
dnl Description:
dnl     Find file(s) in given directories.
dnl
dnl ---------------------------------------------------------

AC_DEFUN([AC_FIND_FILE],
[
$3=NO
for i in $2;
do
  for j in $1;
  do
    if test -r "$i/$j"; then
      $3=$i
      break 2
    fi
  done
done
])

dnl ---------------------------------------------------------
dnl
dnl Macro: 
dnl     AC_CHECK_ODBC_TYPE
dnl
dnl Arguments:
dnl
dnl Description:
dnl     Checks if $1 is a valid type in the ODBC environment,
dnl     and #defines it to $2 if not
dnl
dnl ---------------------------------------------------------

AC_DEFUN([AC_CHECK_ODBC_TYPE],
[

AC_MSG_CHECKING([for $1])
AC_CACHE_VAL(ac_cv_odbc_$1,
[
echo > conftest.c

for i in $odbc_headers
do
	echo "#include <$i>" >> conftest.c
done

echo "int main(void) { $1 x; return 0; }" >> conftest.c

if $CC -c $CFLAGS $CPPFLAGS conftest.c > /dev/null 2> /dev/null
then
	eval ac_cv_odbc_$1=yes
else
	eval ac_cv_odbc_$2=no
fi
rm -f conftest*
])

eval ac_odbc_check_res=$ac_cv_odbc_$1
if test "x$ac_odbc_check_res" = "xyes"
then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT([no ($2)])
	AC_DEFINE($1,$2,[Define if $1 is undefined])
fi
])


dnl ---------------------------------------------------------
dnl
dnl Macro: 
dnl     AC_CHECK_IODBC
dnl
dnl Arguments: 
dnl     $1=includedir
dnl     $2=libdir
dnl
dnl Description:
dnl     Checks for iODBC. If found configure for it.
dnl
dnl ---------------------------------------------------------

AC_DEFUN([AC_CHECK_IODBC],
[
check_iobc_inc_path="$1"
check_iobc_lib_path="$2"
CPPFLAGS="$CPPFLAGS -I$check_iobc_inc_path"
AC_CHECK_HEADERS([isql.h isqlext.h isqltypes.h],
[iodbc_ok=yes;odbc_headers="$odbc_headers $ac_hdr"],[iodbc_ok=no; break])

if test "x$iodbc_ok" != "xyes"
then
	AC_MSG_ERROR([Unable to find the iodbc headers in '$check_iobc_inc_path'])
fi


# new autoconf tools doesn't detect through ac_hdr, so define
# odbc_headers manually to make AC_CHECK_ODBC_TYPE to work
if test "x$odbc_headers" = "x"
then
  odbc_headers="isql.h isqlext.h isqltypes.h"
fi

AC_CHECK_HEADERS(iodbcinst.h)

if test "x$ac_cv_header_iodbcinst_h" = "xyes"
then

  odbc_headers="$odbc_headers iodbcinst.h"
  save_LDFLAGS="$LDFLAGS"
  LDFLAGS="-L$check_iobc_lib_path $LDFLAGS"

  AC_CHECK_LIB(iodbcinst,SQLGetPrivateProfileString,
  [AC_DEFINE(HAVE_SQLGETPRIVATEPROFILESTRING,1,[Define if SQLGetPrivateProfileString is defined])
   LIBS="$LIBS -L$check_iobc_lib_path -liodbcinst" ; have_iodbcinst=yes], [])

  LDFLAGS="$save_LDFLAGS"

fi
])


dnl ---------------------------------------------------------
dnl
dnl Macro: 
dnl     AC_CHECK_UNIXODBC
dnl
dnl Arguments:
dnl	$1=includedir
dnl	$2=libdir
dnl
dnl Description:
dnl     Check for unixODBC. If found configure for it.
dnl
dnl ---------------------------------------------------------

AC_DEFUN([AC_CHECK_UNIXODBC],
[
check_iobc_inc_path="$1"
check_iobc_lib_path="$2"
CPPFLAGS="$CPPFLAGS -I$check_iobc_inc_path"
AC_CHECK_HEADERS([sql.h sqlext.h sqltypes.h],
[unixODBC_ok=yes;odbc_headers="$odbc_headers $ac_hdr"],[unixODBC_ok=no; break])

if test "x$unixODBC_ok" != "xyes"
then
	AC_MSG_ERROR([Unable to find the unixODBC headers in '$check_iobc_inc_path'])
fi

# new autoconf tools doesn't detect through ac_hdr, so define
# odbc_headers manually to make AC_CHECK_ODBC_TYPE to work
if test "x$odbc_headers" = "x   "
then
  odbc_headers="sql.h sqlext.h sqltypes.h"
fi

AC_CHECK_HEADERS(odbcinst.h)

if test "x$ac_cv_header_odbcinst_h" = "xyes"
then

  odbc_headers="$odbc_headers odbcinst.h"
  save_LDFLAGS="$LDFLAGS"
  LDFLAGS="-L$check_iobc_lib_path $LDFLAGS"

  AC_CHECK_LIB(odbcinst,SQLGetPrivateProfileString,
  [AC_DEFINE(HAVE_SQLGETPRIVATEPROFILESTRING,1,[Define if SQLGetPrivateProfileString is defined])
  LIBS="$LIBS -L$check_iobc_lib_path -lodbcinst" ; have_odbcinst=yes], [])
  LDFLAGS="$save_LDFLAGS"

fi   
])


m4_include([qt.m4])

