
IF(WITH_UNIXODBC)
	# check for location of odbc_config
	FIND_PROGRAM(ODBC_CONFIG odbc_config
				 $ENV{ODBC_PATH}/bin
				 PATHS)
		
	IF(NOT ODBC_CONFIG)
		MESSAGE(FATAL_ERROR "Couldn't find unixODBC")
	ENDIF(NOT ODBC_CONFIG)

	MESSAGE(STATUS "unixODBC: Found odbc_config in ${ODBC_CONFIG}")

	EXEC_PROGRAM(${ODBC_CONFIG} ARGS "--include-prefix" OUTPUT_VARIABLE ODBC_INCLUDE_DIR)
	SET (CMAKE_FLAGS "${CMAKE_FLAGS} -I${ODBC_INCLUDE_DIR}")

	EXEC_PROGRAM(${ODBC_CONFIG} ARGS "--libs" OUTPUT_VARIABLE ODBC_LINK_FLAGS)

ELSE(WITH_UNIXODBC)

	FIND_PROGRAM(ODBC_CONFIG iodbc-config
				 $ENV{ODBC_PATH}/bin
				 PATHS)
		
	IF(NOT ODBC_CONFIG)
		MESSAGE(FATAL_ERROR "Couldn't find iODBC")
	ENDIF(NOT ODBC_CONFIG)

	MESSAGE(STATUS "iODBC: Found iodbc-config in ${ODBC_CONFIG}")

	EXEC_PROGRAM(${ODBC_CONFIG} ARGS "--cflags" OUTPUT_VARIABLE ODBC_CFLAGS)
	SET(CMAKE_FLAGS "${CMAKE_FLAGS} ${ODBC_CFLAGS}")

	EXEC_PROGRAM(${ODBC_CONFIG} ARGS "--libs" OUTPUT_VARIABLE ODBC_LINK_FLAGS)

		
ENDIF(WITH_UNIXODBC)

