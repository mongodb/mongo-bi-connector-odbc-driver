MySQL Connector/ODBC 5.3

This is a release of MySQL Connector/ODBC (formerly MyODBC),
Oracle's dual-license ODBC Driver for MySQL. For the avoidance
of doubt, this particular copy of the software is released
under the version 2 of the GNU General Public License. 
MySQL Connector/ODBC is brought to you by Oracle.

Copyright (c) 1995, 2013, Oracle and/or its affiliates. All rights reserved.

License information can be found in the COPYING file.

MySQL FOSS License Exception
We want free and open source software applications under 
certain licenses to be able to use the GPL-licensed MySQL 
Connector/ODBC (specified GPL-licensed MySQL client libraries)
despite the fact that not all such FOSS licenses are 
compatible with version 2 of the GNU General Public License.
Therefore there are special exceptions to the terms and
conditions of the GPLv2 as applied to these client libraries, 
which are identified and described in more detail in the 
FOSS License Exception at
<http://www.mysql.com/about/legal/licensing/foss-exception.html>

This distribution may include materials developed by third
parties. For license and attribution notices for these
materials, please refer to the documentation that accompanies
this distribution (see the "Licenses for Third-Party Components"
appendix) or view the online documentation at 
<http://dev.mysql.com/doc/>

GPLv2 Disclaimer
For the avoidance of doubt, except that if any license choice
other than GPL or LGPL is available it will apply instead, 
Oracle elects to use only the General Public License version 2 
(GPLv2) at this time for any software where a choice of GPL 
license versions is made available with the language indicating 
that GPLv2 or any later version may be used, or where a choice 
of which version of the GPL is applied is otherwise unspecified.

CONTENTS

* Introduction
* Building Source
* Installing
* Binary Files
* Connector/ODBC SDK
* Resources

INTRODUCTION
------------------------------------------------------------

This is the source or binary distribution of ODBC for 
MySQL. This software is distributed with restrictions - please
see the license information for details.

Open Database Connectivity (ODBC) is a widely accepted standard
for reading/writing data. ODBC is designed in a similar manner
to the MS Windows printing system in that it has a Driver Manager
and Drivers. MySQL Connector/ODBC is a driver for the ODBC system 
which allows applications to communicate with the MySQL Server 
using the ODBC standard. ODBC implementations exist on all popular
platforms and MySQL Connector/ODBC is also available on those 
platforms. 

Oracle provides MySQL support for ODBC by means of the MySQL
Connector/ODBC driver and programs. MySQL Connector/ODBC is the
ODBC driver for MySQL which is ODBC 3.5x compliant; MyODBC is
ODBC 2.5x compliant.

To get all functionality from the ODBC 5.3 driver (e.g. 
transaction support) you should use it against MySQL Database 
Server 4.1 or later.

BUILDING SOURCE
---------------------------------------------------------------

Please see the BUILD file for details on building the source
code. This is included with the source distribution.

INSTALLING
---------------------------------------------------------------

Please see the INSTALL file for details on installing.

BINARY FILES
---------------------------------------------------------------

The following, key, files are provided by MySQL Connector/ODBC
(the MS Windows names are included for reference but the file 
names for other platforms would be similar).

Driver (myodbc5a.dll and myodbc5w.dll)

     The most important part of Connector/ODBC is the driver.
     Application developers can link directly to the driver but 
     more often use the driver indirectly, via a Driver Manager
     such as the one provided by Microsoft or unixODBC.

     There is an ANSI version, myodbc5a.dll, and an Unicode
     version, myodbc5w.dll, of the driver.

     Many options can be provided to the driver to adjust its
     behavior. 

Setup Library (myodbc5S.dll)

     The setup library provides a graphical user interface needed
     during certain activities provided by the driver. For example;
     it provides the GUI when you use the ODBC Administrator to
     create/edit a Data Source Name (DSN).

Installer (myodbc-installer.exe)

     This command-line utility program can be used to manage ODBC
     system information. For example it can be used to register
     or deregister a driver. It can also manage data source names.
     This was created to test the ODBC system information
     abstraction and portability layer within Connector/ODBC but 
     is also useful for installing and uninstalling Connector/ODBC.

     Invoke myodbc-installer.exe, in a command shell and without
     options, to get help on its use.

Library Loading Tester (dltest.exe)

     This command-line utility is useful for debugging problems
     related to loading a library and resolving symbols in it.

Connector/ODBC SDK
---------------------------------------------------------------

MySQL Connector/ODBC provides an SDK for developers in the form 
of several reusable libraries. These libraries can be used to 
save development time/effort and to increase stability of resulting
software.

myodbc-util

     This library provides the ODBC system information abstraction
     and portability used by Connector/ODBC.

RESOURCES
---------------------------------------------------------------

For more information about MySQL, see
http://www.mysql.com

For more information about MySQL Connector/ODBC, including
installation instructions, please visit;
http://www.mysql.com/products/myodbc/index.html
