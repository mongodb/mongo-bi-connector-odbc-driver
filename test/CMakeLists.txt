INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/test/include)

ADD_EXECUTABLE(my_basics my_basics.c)
TARGET_LINK_LIBRARIES(my_basics myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_blob my_blob.c)
TARGET_LINK_LIBRARIES(my_blob myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_bulk my_bulk.c)
TARGET_LINK_LIBRARIES(my_bulk myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_catalog my_catalog.c)
TARGET_LINK_LIBRARIES(my_catalog myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_curext my_curext.c)
TARGET_LINK_LIBRARIES(my_curext myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_cursor my_cursor.c)
TARGET_LINK_LIBRARIES(my_cursor myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_datetime my_datetime.c)
TARGET_LINK_LIBRARIES(my_datetime myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_dyn_cursor my_dyn_cursor.c)
TARGET_LINK_LIBRARIES(my_dyn_cursor myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_error my_error.c)
TARGET_LINK_LIBRARIES(my_error myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_info my_info.c)
TARGET_LINK_LIBRARIES(my_info myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_keys my_keys.c)
TARGET_LINK_LIBRARIES(my_keys myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_param my_param.c)
TARGET_LINK_LIBRARIES(my_param myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_position my_position.c)
TARGET_LINK_LIBRARIES(my_position myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_prepare my_prepare.c)
TARGET_LINK_LIBRARIES(my_prepare myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_relative my_relative.c)
TARGET_LINK_LIBRARIES(my_relative myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_result my_result.c)
TARGET_LINK_LIBRARIES(my_result myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_scroll my_scroll.c)
TARGET_LINK_LIBRARIES(my_scroll myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_tran my_tran.c)
TARGET_LINK_LIBRARIES(my_tran myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_types my_types.c)
TARGET_LINK_LIBRARIES(my_types myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_unixodbc my_unixodbc.c)
TARGET_LINK_LIBRARIES(my_unixodbc myodbc3 myodbc3u mysqlclient)

ADD_EXECUTABLE(my_use_result my_use_result.c)
TARGET_LINK_LIBRARIES(my_use_result myodbc3 myodbc3u mysqlclient)

#---------------- ADD TESTS --------------
ENABLE_TESTING()
ADD_TEST(my_use_result my_use_result)
ADD_TEST(my_unixodbc my_unixodbc)
ADD_TEST(my_types my_types)
ADD_TEST(my_tran my_tran)
ADD_TEST(my_scroll my_scroll)
ADD_TEST(my_result my_result)
ADD_TEST(my_relative my_relative)
ADD_TEST(my_prepare my_prepare)
ADD_TEST(my_position my_position)
ADD_TEST(my_param my_param)
ADD_TEST(my_keys my_keys)
ADD_TEST(my_info my_info)
ADD_TEST(my_error my_error)
ADD_TEST(my_basics my_basics)
ADD_TEST(my_blob my_blob)
ADD_TEST(my_bulk my_bulk)
ADD_TEST(my_catalog my_catalog)
ADD_TEST(my_curext my_curext)
ADD_TEST(my_cursor my_cursor)
ADD_TEST(my_datetime my_datetime)
ADD_TEST(my_dyn_cursor my_dyn_cursor)
