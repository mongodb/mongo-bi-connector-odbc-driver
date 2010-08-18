MySQL Connector/ODBC (formerly MyODBC)
Oracle's ODBC Driver for MySQL
Copyright 1995, 2010 Oracle and/or its affiliates. All rights reserved.


CONTENTS

* Introduction
* License
* Building Source
* Installing
* Binary Files
* Connector/ODBC SDK
* Resources


INTRODUCTION
------------------------------------------------------------

This is the source or binary distribution of ODBC for 
MySQL. This software is distributed with restrictions - please
see the license file for details.

Open Database Connectivity (ODBC) is a widely accepted standard
for reading/writing data. ODBC is designed in a similar manner
to the MS Windows printing system in that it has a Driver Manager
and Drivers. MySQL Connector/ODBC is a driver for the ODBC system which
allows applications to communicate with the MySQL server using
the ODBC standard. ODBC implementations exist on all popular
platforms and MySQL Connector/ODBC is also available on those 
platforms. 

Oracle provides MySQL support for ODBC by means of the MySQL Connector/ODBC driver and programs. MySQL Connector/ODBC is the ODBC driver for MySQL which is ODBC 3.5x compliant; MyODBC is ODBC 2.5x compliant.

To get all functionality from the ODBC 5.1 driver (e.g. 
transaction support) you should use it against MySQL Database Server 4.1 or later.


LICENSE
---------------------------------------------------------------

MySQL Connector/ODBC is licensed under the GPLv2 or a commercial license from Oracle Corporation. 

If you have licensed this product under the GPLv2, please see the
LICENSE.gpl file for more information. 

There are special exceptions to the terms and conditions of the 
GPLv2 as it is applied to this software, see the FLOSS License
Exception 
<http://www.mysql.com/about/legal/licensing/foss-exception.html>. 


BUILDING SOURCE
---------------------------------------------------------------

Please see the BUILD file for details on building the source
code. This is included with the source distribution.

INSTALLING
---------------------------------------------------------------

Please see the INSTALL file for details on installing.

BINARY FILES
---------------------------------------------------------------

The following, key, files are provided by MySQL Connector/ODBC (the MS 
Windows names are included for reference but the file names for 
other platforms would be similar).

Driver (myodbc3.dll or myodbc5.dll)

        The most important part of Connector/ODBC is the driver.
        Application developers can link directly to the driver but 
        more often use the driver indirectly, via a Driver Manager
        such as the one provided by Microsoft or unixODBC.

        Many options can be provided to the driver to adjust its
        behaviour. 

Setup Library (myodbc3S.dll or myodbc3S.dll)

        The setup library provides a graphical user interface needed
        during certian activities provided by the driver. For example;
        it provides the GUI when you use the ODBC Administrator to
        create/edit a Data Source Name (DSN).

Installer (myodbc3i.exe)

        This command-line utility program can be used to manage ODBC
        system information. For example it can be used to register
        or deregister a driver. It can also manage data source names.
        This was created to test the ODBC system information
        abstraction and portability layer within Connector/ODBC but 
        is also useful for installing and uninstalling Conector/ODBC.

        Invoke myodbc3i, in a command shell and without options, to 
        get help on its use.

Library Loading Tester (dltest.exe)

        This command-line utility is useful fo debugging problems
        related to loading a library and resolving symbols in it.

Connector/ODBC SDK
---------------------------------------------------------------

MyySQL Connector/ODBC provides an SDK for developers in the form of several reusable libraries. These libraries can be used to save development time/effort and to increase stability of resulting software.

myodbc3u
        This library provides the ODBC system information abstraction
        and portability used by Connector/ODBC.

RESOURCES
---------------------------------------------------------------

For more information about MySQL, see
http://www.mysql.com

For more information about MySQL Connector/ODBC, including
installation instructions, please visit;
http://www.mysql.com/products/myodbc/index.html
