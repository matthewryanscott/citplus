#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>

void main(void)
	{
	int fhexe, fhdat, fhnew;

	printf("Citadel/065 EXE/DAT combiner\n"
		"By Anticlimactic Teleservices\n");

	fhexe = open("ctdl.exe", O_RDONLY | O_BINARY);
	fhdat = open("ctdl.dat", O_RDONLY | O_BINARY);
	fhnew = open("ctdl.new", O_RDWR | O_CREAT | O_TRUNC | O_BINARY,
			S_IREAD | S_IWRITE);

	if (fhexe != -1 && fhdat != -1 && fhnew != -1)
		{
		long length = filelength(fhexe);
		int i;
		char buffer[5120];

		printf("Copying exe...");
		while ((i = read(fhexe, buffer, sizeof(buffer))) != 0)
			{
			if (write(fhnew, buffer, i) != i)
				{
				printf("Error writing.");
				exit(0);
				}
			}

		printf(" Copying dat...");
		while ((i = read(fhdat, buffer, sizeof(buffer))) != 0)
			{
			if (write(fhnew, buffer, i) != i)
				{
				printf("Error writing.");
				exit(0);
				}
			}

		printf(" Writing length...");
		if (write(fhnew, &length, sizeof(length)) != sizeof(length))
			{
			printf("Error writing.");
			exit(0);
			}

		printf(" Writing signature...");
		length = 0x69696969;
		if (write(fhnew, &length, sizeof(length)) != sizeof(length))
			{
			printf("Error writing.");
			exit(0);
			}

		printf(" Done.\n");

		close(fhexe);
		close(fhdat);
		close(fhnew);

		unlink("ctdl.exe");
		rename("ctdl.new", "ctdl.exe");
		}
	else
		{
		printf("could not open all files.");
		}
	}
