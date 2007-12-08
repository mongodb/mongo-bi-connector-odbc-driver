##############################################################################
#
# mysql-connector-odbc 5.1 RPM specification
#
##############################################################################

# You can control the license file to include building like
# "rpmbuild --with commercial" or "rpm --define '_with_commercial 1'"
# (for RPM 3.x). The default is GPL.

%{?_with_commercial:%define com_lic 1}
%{!?_with_commercial:%define com_lic 0}

%define no_odbc_gui 0

##############################################################################
#
#  Main information section
#
##############################################################################

%if %{com_lic}
Name:       mysql-connector-odbc-commercial
%else
Name:       mysql-connector-odbc
%endif
Summary:    An ODBC 5.1 driver for MySQL - driver package
Group:      Applications/Databases
Version:    @NUMERIC_VERSION@
Release:    0
Provides:   mysqlodbcrpmpack
%if %{com_lic}
Copyright:  Commercial
%else
Copyright:  GPL
%endif
Source0:    %{name}-@VERSION@.tar.gz
URL:        http://www.mysql.com/
Vendor:     MySQL AB
Packager:   MySQL Production Engineering Team <build@mysql.com>

BuildRoot:  %{_tmppath}/%{name}-@VERSION@-build

%if %{no_odbc_gui}
%else
%package setup
Summary:    An ODBC 5.1 driver for MySQL - setup library
Group:      Application/Databases
Requires:   mysqlodbcrpmpack
%endif

##############################################################################
#
#  Documentation
#
##############################################################################

%description
mysql-connector-odbc is an ODBC (3.50) level 0 (with level 1 and level
2 features) driver for connecting an ODBC-aware application to MySQL.
mysql-connector-odbc  works on Windows NT/2000/XP/2003, and most Unix
platforms (incl. OSX and Linux).

mysql-connector-odbc 5.1 is an enhanced version of MyODBC 2.50 to meet
ODBC 3.5 specification. The driver is commonly referred to as
'MySQL ODBC 5.1 Driver'.

%description setup
The setup library for the MySQL ODBC package, handles the optional GUI
dialog for configuring the driver.

##############################################################################
#
#  Build
#
##############################################################################

%prep
%setup -n %{name}-@VERSION@

%define ODBC_DM_PATH_ARG @ODBC_DM_PATH_ARG@
%define MYSQL_PATH_ARG   @MYSQL_PATH_ARG@
%define QT_PATH_ARG      @QT_PATH_ARG@
%define EXTRA_XLIBS      @EXTRA_XLIBS@

%build
./configure \
    --prefix=%{_prefix} \
    --libdir=%{_libdir} \
    --enable-dmlink \
%if %{no_odbc_gui}
    --disable-gui \
%else
    %{QT_PATH_ARG} \
    --with-extra-xlibs='%{EXTRA_XLIBS}' \
%endif
    --with-separate-debug-driver \
    %{ODBC_DM_PATH_ARG} \
    %{MYSQL_PATH_ARG} \
    --without-debug
make

##############################################################################
#
#  Cleanup
#
##############################################################################

%clean 
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

##############################################################################
#
#  Install and deinstall scripts
#
##############################################################################

# ----------------------------------------------------------------------
# Install, but remove the doc files.
# The way %doc <file-without-path> works, we can't
# have these files installed
# ----------------------------------------------------------------------

%install
make DESTDIR=$RPM_BUILD_ROOT install
rm -v $RPM_BUILD_ROOT%{_datadir}/mysql-connector-odbc/{ChangeLog,README,README.debug,LICENSE.*,INSTALL}

# ----------------------------------------------------------------------
# REGISTER DRIVER
# Note that "-e" is not working for drivers currently, so we have to
# deinstall before reinstall to change anything
# ----------------------------------------------------------------------

%post 
myodbc-installer -a -d -n "MySQL ODBC 5.1 Driver" -t "DRIVER=%{_libdir}/libmyodbc5.so"

%post setup
myodbc-installer -r -d -n "MySQL ODBC 5.1 Driver"
myodbc-installer -a -d -n "MySQL ODBC 5.1 Driver" -t "DRIVER=%{_libdir}/libmyodbc5.so;SETUP=%{_libdir}/libmyodbc3S.so"

# ----------------------------------------------------------------------
# DEREGISTER DRIVER 
# ----------------------------------------------------------------------

# Removing the driver package, we simply orphan any related DSNs
%preun
myodbc-installer -r -d -n "MySQL ODBC 5.1 Driver"

# Removing the setup RPM, downgrade the registration
%preun setup
myodbc-installer -r -d -n "MySQL ODBC 5.1 Driver"
myodbc-installer -a -d -n "MySQL ODBC 5.1 Driver" -t "DRIVER=%{_libdir}/libmyodbc5.so"

##############################################################################
#
#  Listing of files to be in the package
#
##############################################################################

%files 
%defattr(-,root,root)
%{_bindir}/myodbc-installer
%{_libdir}/libmyodbc5.*
%{_libdir}/libmyodbc5-*
%doc ChangeLog README README.debug INSTALL
%if %{com_lic}
%doc LICENSE.commercial
%else
%doc LICENSE.gpl
%doc LICENSE.exceptions
%endif

%files setup
%{_libdir}/libmyodbc3S*

##############################################################################
