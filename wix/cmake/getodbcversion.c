/* Copyright 2007 MySQL AB

   The MySQL Connector/ODBC is licensed under the terms of the
   GPL, like most MySQL Connectors. There are special exceptions
   to the terms and conditions of the GPL as it is applied to
   this software, see the FLOSS License Exception available on
   mysql.com.

   This program is free software; you can redistribute it and/or modify
   it under the terms of version 2 of the GNU General Public License as
   published by the Free Software Foundation.

   There are special exceptions to the terms and conditions of the GPL as it
   is applied to this software. View the full text of the exception in file
   EXCEPTIONS in the directory of this software distribution.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

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
	fprintf(fp, "<?define odbc_ver_prev=\"%d.%d.%d\" ?>\n", v1, v2, v3-1);
	fprintf(fp, "<?define odbc_ver_next=\"%d.%d.%d\" ?>\n", v1, v2, v3+1);

	fclose(fp);

	
	if (!(fp = fopen("myodbc_version.cmake",  "w")))
		exit (2);

	fprintf(fp, "SET(ODBC_VERSION \"%d.%d.%d\")", v1,v2,v3);
	fclose(fp);
}
