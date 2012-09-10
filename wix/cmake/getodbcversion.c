/* Copyright (c) 2007, 2012, Oracle and/or its affiliates. All rights reserved.

   The MySQL Connector/ODBC is licensed under the terms of the GPLv2
   <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
   MySQL Connectors. There are special exceptions to the terms and
   conditions of the GPLv2 as it is applied to this software, see the
   FLOSS License Exception
   <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; version 2 of the License.
   
   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
   for more details.
   
   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <stdio.h>
#include "..\..\VersionInfo.h"

int main(int argc, char *argv[])
{
	FILE *fp;
	int v1, v2, v3;

	if (argc ==1)
		exit (1);

	if (!(fp = fopen("myodbc_version.xml",  "w")))
		exit (2);

	fprintf(fp, "<Include>\n");

	sscanf(SETUP_VERSION, "%d.%d.%d", &v1, &v2, &v3);

	fprintf(fp, "<?define odbc_ver_short=\"%d.%d\" ?>\n", v1, v2);
	fprintf(fp, "<?define odbc_ver_long=\"%d.%d.%d\" ?>\n", v1, v2, v3);
        if (v3 == 0) {
          if (v2 == 0)
            fprintf(fp, "<?define odbc_ver_prev=\"%d.%d.%d\" ?>\n", v1-1, 999, 999);
          else
            fprintf(fp, "<?define odbc_ver_prev=\"%d.%d.%d\" ?>\n", v1, v2-1, 999);
        } else {
          fprintf(fp, "<?define odbc_ver_prev=\"%d.%d.%d\" ?>\n", v1, v2, v3-1);
        }
	fprintf(fp, "<?define odbc_ver_next=\"%d.%d.%d\" ?>\n", v1, v2, v3+1);
	fprintf(fp, "<?define odbc_driver_type=\""MYODBC_STRDRIVERTYPE"\" ?>\n");
	fprintf(fp, "<?define odbc_driver_type_suffix=\""MYODBC_STRTYPE_SUFFIX"\" ?>\n");
	fprintf(fp, "<?define odbc_driver_series=\""MYODBC_STRDRIVERID"\" ?>\n");

	fclose(fp);

	
	if (!(fp = fopen("myodbc_version.cmake",  "w")))
		exit (2);

	fprintf(fp, "SET(ODBC_VERSION \"%d.%d.%d\")\n", v1,v2,v3);
	fprintf(fp, "SET(ODBC_VERSION_SUFFIX \"%s\")", MYODBC_STRTYPE_SUFFIX);

	fclose(fp);
}
