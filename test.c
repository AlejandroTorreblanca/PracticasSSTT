#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <malloc.h>

#define BUFSIZE		8096
#define TAMMAX 89056
int tamArchivo;
struct {
	char *ext;
	char *filetype;
} extensions [] = {
	{".gif", "image/gif" },
	{".jpg", "image/jpg" },
	{".jpeg","image/jpeg"},
	{".png", "image/png" },
	{".ico", "image/ico" },
	{".zip", "image/zip" },
	{".gz",  "image/gz"  },
	{".tar", "image/tar" },
	{".htm", "text/html" },
	{".html","text/html" },
	{0,0} };


main(){
	
	char * cadena="Cookie: id:1";
	char buffer[BUFSIZE];
	char * aux;
	char * aux2;
	strcpy(buffer,cadena);
	strtok(buffer,":");
	strtok (NULL, ":");
	aux2 = strtok (NULL, ":");
	int c = atoi(aux2);
	
	fprintf(stderr,"cookie: %d\n",c);
	return 0;
}






















