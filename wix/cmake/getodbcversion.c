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
