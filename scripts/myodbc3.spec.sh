#
# mysql-connector-odbc 3.51 RPM spec 
#

# You can control the license file to include building like
# "rpmbuild --with commercial" or "rpm --define '_with_commercial 1'"
# (for RPM 3.x). The default is GPL.

%{?_with_commercial:%define com_lic 1}
%{!?_with_commercial:%define com_lic 0}

%define myodbc_version @VERSION@
%define release 0

%if %{com_lic}
Name:       mysql-connector-odbc-commercial
%else
Name:       mysql-connector-odbc
%endif
Summary:    An ODBC 3.51 driver for MySQL
Group:      Applications/Databases
Version:    %{myodbc_version}
Release:    %{release}
%if %{com_lic}
Copyright:  Commercial
%else
Copyright:  GPL
%endif
Source0:    %{name}-%{version}.tar.gz
URL:        http://www.mysql.com/
Vendor:	    MySQL AB
Packager:   MySQL Production Engineering Team <build@mysql.com>

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
%define MYSQL_PATH_ARG  @MYSQL_PATH_ARG@

%build
./configure \
    --prefix=%{_prefix} \
    --libdir=%{_libdir} \
    --enable-dmlink \
    --disable-gui \
    --with-separate-debug-driver \
    %{ODBC_DM_PATH} \
    %{MYSQL_PATH_ARG} \
    --without-debug
make

%clean 
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%install
make DESTDIR=$RPM_BUILD_ROOT install
# The way %doc <file-without-path> works, we can't
# have these files installed
rm -v $RPM_BUILD_ROOT%{_datadir}/mysql-connector-odbc/{ChangeLog,README,README.debug}

#
# REGISTER DRIVER
#   Hard-coded path here makes this package non-relocatable.
#
%post
myodbc3i -w0 -a -d -t"MySQL ODBC 3.51 Driver;DRIVER=%{_libdir}/libmyodbc3.so;SETUP=%{_libdir}/libmyodbc3S.so"

#
# DEREGISTER DRIVER 
#   We simply orphan any related DSNs.
#
%preun
myodbc3i -w0 -r -d -n"MySQL ODBC 3.51 Driver"

%files 
%defattr(-,root,root)
%attr(755, root, root) %{_bindir}/myodbc3i
%attr(755, root, root) %{_bindir}/myodbc3m
%attr(644, root, root) %{_libdir}/libmyodbc3*

%doc ChangeLog README README.debug
%if %{com_lic}
%doc LICENSE.commercial
%else
%doc LICENSE.gpl
%doc LICENSE.exceptions
%endif
