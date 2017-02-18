#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define BUFSIZE		8096

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
	char buffer[BUFSIZE];
	char * a="GET /felix.gif HTTP/1.1";
//	char * a="GET / HTTP/1.1";
	char * aux;
	char * aux2;
	strcpy(buffer,a);
	fprintf (stderr,"buffer:%s\n", buffer);
	aux = strtok(buffer,"/");
	aux2 = strtok (NULL, "/");
	fprintf (stderr,"aux2:%s\n", aux2);
	if(strcmp(aux2," HTTP")==0) 
		fprintf (stderr,"correcto\n");
	else 
	{
		strcpy(buffer,aux2);
		aux = strtok(buffer," ");
		fprintf (stderr,"fin:%s\n", aux);
	}

	

	return 0;
}






















