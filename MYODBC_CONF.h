#ifndef MYODBC_CONF_H
# define MYODBC_CONF_H
# ifdef HAVE_CONFIG_H
#  include "driver/myconf.h"
#  define SETUP_VERSION VERSION
/* Work around iODBC header bug on Mac OS X 10.3 */
#  undef HAVE_CONFIG_H
# endif
#endif
