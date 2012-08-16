# Copyright (c) 2000, 2012, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING. If not, write to the
# Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston
# MA  02110-1301  USA.

##############################################################################
#
# mysql-connector-odbc 5.2 RPM specification
#
##############################################################################

# You can control the license file to include building like
# "rpmbuild --with commercial" or "rpm --define '_with_commercial 1'"
# (for RPM 3.x). The default is GPL.

%{?_with_commercial:%define com_lic 1}
%{!?_with_commercial:%define com_lic 0}

%if %{com_lic}
%define lic_type Commercial
%else
%define lic_type GPL
%endif

%define no_odbc_gui 0

##############################################################################
#
#  Main information section
#
##############################################################################

%define mysql_vendor		Oracle and/or its affiliates

%if %{com_lic}
Name:       mysql-connector-odbc-commercial
%else
Name:       mysql-connector-odbc
%endif
Summary:    An ODBC 5.2 driver for MySQL - driver package
Group:      Applications/Databases
Version:    @NUMERIC_VERSION@
Release:    1
Provides:   mysqlodbcrpmpack
License:    Copyright (c) 2000, @YEAR@, %{mysql_vendor}. All rights reserved.  Under %{lic_type} license as shown in the Description field.
Source0:    %{name}-@VERSION@-src.tar.gz
URL:        http://www.mysql.com/
Vendor:     %{mysql_vendor}
Packager:   %{mysql_vendor} Product Engineering Team <mysql-build@oss.oracle.com>

BuildRoot:  %{_tmppath}/%{name}-@VERSION@-build

%if %{no_odbc_gui}
%else
%package setup
Summary:    An ODBC 5.2 driver for MySQL - setup library
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
platforms (incl. OSX and Linux). MySQL is a trademark of
%{mysql_vendor}

mysql-connector-odbc 5.2 is an enhanced version of MyODBC 2.50 to meet
ODBC 3.5 specification. The driver is commonly referred to as
'MySQL ODBC 5.2 Driver'.

The MySQL software has Dual Licensing, which means you can use the MySQL
software free of charge under the GNU General Public License
(http://www.gnu.org/licenses/). You can also purchase commercial MySQL
licenses from %{mysql_vendor} if you do not wish to be bound by the terms of
the GPL. See the chapter "Licensing and Support" in the manual for
further info.

The MySQL web site (http://www.mysql.com/) provides the latest
news and information about the MySQL software. Also please see the
documentation and the manual for more information.

%if %{no_odbc_gui}
%else
%description setup
The setup library for the MySQL ODBC package, handles the optional GUI
dialog for configuring the driver.
%endif

##############################################################################
#
#  Build
#
##############################################################################

%prep
%setup -n %{name}-@VERSION@-src

%define ODBC_DM_PATH_ARG @ODBC_DM_PATH_ARG@
%define MYSQL_PATH_ARG   @MYSQL_PATH_ARG@
%define EXTRA_XLIBS      @EXTRA_XLIBS@

%build
mkdir release
cd release
cmake -G "Unix Makefiles" \
    -DRPM_BUILD=1 \
    -DCMAKE_INSTALL_PREFIX=%{_prefix} \
%if %{no_odbc_gui}
    -DDISABLE_GUI=1 \
%endif
    %{ODBC_DM_PATH_ARG} \
    %{MYSQL_PATH_ARG} \
    ..

# Note that the ".." needs to be last, in case some arguments expands to 
# the empty string, and then "cmake" thinks is "current directory"

make VERBOSE=1

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
cd release
make DESTDIR=$RPM_BUILD_ROOT install VERBOSE=1
rm -vf  $RPM_BUILD_ROOT%{_prefix}/{ChangeLog,README*,LICENSE.*,COPYING,INSTALL*,Licenses_for_Third-Party_Components.txt}
rm -vfr $RPM_BUILD_ROOT%{_prefix}/test

# ----------------------------------------------------------------------
# REGISTER DRIVER
# Note that "-e" is not working for drivers currently, so we have to
# deinstall before reinstall to change anything
# ----------------------------------------------------------------------

%post 
myodbc-installer -a -d -n "MySQL ODBC 5.2 Driver" -t "DRIVER=%{_libdir}/libmyodbc5@CONNECTOR_DRIVER_TYPE_SHORT@.so"

%if %{no_odbc_gui}
%else
%post setup
myodbc-installer -r -d -n "MySQL ODBC 5.2 Driver"
myodbc-installer -a -d -n "MySQL ODBC 5.2 Driver" -t "DRIVER=%{_libdir}/libmyodbc5@CONNECTOR_DRIVER_TYPE_SHORT@.so;SETUP=%{_libdir}/libmyodbc3S.so"
%endif

# ----------------------------------------------------------------------
# DEREGISTER DRIVER 
# ----------------------------------------------------------------------

# Removing the driver package, we simply orphan any related DSNs
%preun
myodbc-installer -r -d -n "MySQL ODBC 5.2 Driver"

# Removing the setup RPM, downgrade the registration
%if %{no_odbc_gui}
%else
%preun setup
myodbc-installer -r -d -n "MySQL ODBC 5.2 Driver"
myodbc-installer -a -d -n "MySQL ODBC 5.2 Driver" -t "DRIVER=%{_libdir}/libmyodbc5@CONNECTOR_DRIVER_TYPE_SHORT@.so"
%endif

##############################################################################
#
#  Listing of files to be in the package
#
##############################################################################

%files
%defattr(-,root,root)
%{_bindir}/myodbc-installer
%{_libdir}/libmyodbc5@CONNECTOR_DRIVER_TYPE_SHORT@.*
%doc ChangeLog README README.debug INSTALL INSTALL.win Licenses_for_Third-Party_Components.txt
%if %{com_lic}
%doc LICENSE.mysql
%else
%doc COPYING
%endif

%if %{no_odbc_gui}
%else
%files setup
%{_libdir}/libmyodbc3S*
%endif

##############################################################################
