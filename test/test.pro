# #########################################################
#
# To build multiple targets in the same directory, we
# tell qmake it is the same sub directories but with
# different .pro files
#
# #########################################################

TEMPLATE = subdirs

SUBDIRS += my_basics.pro
SUBDIRS += my_blob.pro
SUBDIRS += my_bulk.pro
SUBDIRS += my_catalog.pro
SUBDIRS += my_curext.pro
SUBDIRS += my_cursor.pro
SUBDIRS += my_datetime.pro
SUBDIRS += my_dyn_cursor.pro
SUBDIRS += my_error.pro
SUBDIRS += my_info.pro
SUBDIRS += my_keys.pro
SUBDIRS += my_param.pro
SUBDIRS += my_prepare.pro
SUBDIRS += my_relative.pro
SUBDIRS += my_result.pro
SUBDIRS += my_scroll.pro
SUBDIRS += my_tran.pro
SUBDIRS += my_types.pro
SUBDIRS += my_unixodbc.pro
SUBDIRS += my_use_result.pro
