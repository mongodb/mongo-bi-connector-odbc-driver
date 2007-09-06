+-------------------------------------------------------------+
| Connector/ODBC                                              |
| Debug                                                       |
+-------------------------------------------------------------+


INTRODUCTION
---------------------------------------------------------------

This brief document describes some methods to debug a problem
in the driver - it is hoped to be useful to experienced 
programmers - particularly driver developers. 

Debugging the driver can be done in a number of ways but is
not usually done by stepping through the source code in debug
mode of gdb or an IDE for example. The most common method is to
get the driver manager to produce trace information showing the
calls being made, the return codes and other useful information.


COMPILER DEBUG INFORMATION
---------------------------------------------------------------

As usual; the driver can be built with or without compiler 
generated debugging information. In practice; this is not the 
most common method to debug problems with the driver. 

This is independent of any of the trace options listed here.


ODBC TRACE
---------------------------------------------------------------

If you are using a Driver Manager (DM) - and most people do - 
then you can turn on ODBC Trace. An ODBC Trace will produce a 
file which shows all of the ODBC calls being made and some very
useful details.

ODBC Trace is particularly useful to see the interaction between
the application and the DM. In some cases an application may be
using software layered on top of ODBC (ADO for example) and in 
this case ODBC Trace is the best way to see how that intermediate 
layer is interacting with ODBC.

ODBC Trace can be turned on/off using the ODBC Administrator.

MS Windows
----------

Invoke the ODBC Administrator from the Start menu;

Start -> Control Panel -> Administrative Tools -> ODBC Administrator

Use the options on the Trace tab to turn tracing on/off. 

NOTE: You should close the ODBC Administrator program to ensure 
      that the changes have taken affect - Apply/Ok is not always 
      enough.

unixODBC
--------

Invoke the ODBCConfig GUI. This can be done from the command-line
in a shell. Turn ODBC Trace on/off and close the application.


FLAG_LOG_QUERY
--------------

This is a DSN config option which can be set to turn on feature
to log queries to a file.

The location of the trace output is dependent upon the
DRIVER_QUERY_LOGFILE const.

This option may be presented as "Save Queries to" in the
DSN edit.

This is meaningless if DBUG_OFF has been set at build-time.

DRIVER_QUERY_LOGFILE
--------------------

This is a hardcoded file location/name for the query log.

           WIN : "c:\\myodbc.sql"
           UNIX: "/tmp/myodbc.sql"


SQL_OPT_TRACE
-------------

This connection attribute is implemented in the Driver Manager.

SQL_OPT_TRACEFILE
-----------------

This connection attribute is implemented in the Driver Manager.
