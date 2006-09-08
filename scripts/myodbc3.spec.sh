#!/bin/sh

#
# mysql-connector-odbc 3.51 RPM spec 
#

%define myodbc_version @VERSION@
%define release 1

Name:		   mysql-connector-odbc
Summary:	   An ODBC 3.51 driver for MySQL
Group:	   Applications/Databases
Version:    %{myodbc_version}
Release:    %{release}
Copyright:	Public Domain, GPL
Source0:    %{name}-%{version}.tar.gz
URL:		   http://www.mysql.com/
Vendor:		MySQL AB
Packager:	Peter Harvey <pharvey@mysql.com>

%define prefix /usr

# Think about what you use here since the first step is to
# run a rm -rf
BuildRoot:	%{_tmppath}/%{name}-%{version}-build

# From the manual
%description
mysql-connector-odbc is an ODBC (3.50) level 0 (with level 1 and level 2 features)
driver for connecting an ODBC-aware application to MySQL. mysql-connector-odbc 
works on Windows NT/2000/XP, and most Unix platforms (incl. OSX and Linux).

mysql-connector-odbc 3.51 is an enhanced version of MyODBC 2.50 to meet ODBC 3.5 
specification. The driver is commonly referred to as 'MySQL ODBC 3.51 Driver'.

%prep
%setup -n %{name}-%{version}

%define ODBC_DM_PATH @ODBC_DM_PATH@
%define MYSQL_LIBS  @MYSQL_USED_LIB_PATH@
%define MYSQL_INCLUDES @MYSQL_USED_INCLUDE_PATH@

%build
./configure \
	--prefix=${RPM_BUILD_ROOT}%{prefix} \
  --with-mysql-libs=%{MYSQL_LIBS} \
  --with-mysql-includes=%{MYSQL_INCLUDES} \
  %{ODBC_DM_PATH} \
  --without-debug
make

%clean 
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%install
make PREFIX=$RPM_BUILD_ROOT install

#
# REGISTER DRIVER
#   Hard-coded path here makes this package non-relocatable.
#
%post
myodbc3i -w0 -a -d -t"MySQL ODBC 3.51 Driver;DRIVER=/usr/lib/libmyodbc3.so;SETUP=/usr/lib/libmyodbc3S.so"

#
# DEREGISTER DRIVER 
#   We simply orphan any related DSNs.
#
%preun
myodbc3i -w0 -r -d -n"MySQL ODBC 3.51 Driver"

%files 
%defattr(-,root,root)
%{prefix}/bin/myodbc3i
%{prefix}/bin/myodbc3m
%{prefix}/lib/libmyodbc3*

%doc ChangeLog
%doc README
