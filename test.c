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

int obtenerID(char * cadena)
{
	char buffer[BUFSIZE];
	char * aux;
	strcpy(buffer,cadena);
	strtok(buffer,":");
	strtok (NULL, "=");
	aux= strtok (NULL, "=");
	return atoi(aux);
}

main(){
	
	char* nombreArchivo="<HTML><HEAD><TITLE>web_SSTT</TITLE></HEAD><BODY><H1>429 INTERNAL SERVER ERROR</H1></BODY></HTML>";
	int m=strlen(nombreArchivo);
	fprintf(stderr,"res: %d\n",m);
	return 0;
}






















