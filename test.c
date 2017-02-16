#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFSIZE		8096
main(){
	char cabecera[]= "GET / HTTP/1.1\r\nHost: 192.168.1.53:9000\r\nUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:44.0) Gecko/20100101 Firefox/44.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate\r\nConnection: keep-alive\r\n\r\n20:09:20 GMT\r\nServer: Apache/2.0.52 (CentOS)\r\nLast-Modified: Tue, 30 Oct 2007 17:00:02 GMT\r\nETag: \"17dc6-a5c-bf7127272\"\r\nAccept-Ranges: bytes\r\nContent-Length: \r\nKeep-Alive: timeout=10, max=100\r\nConnection: Keep-Alive\r\nContent-Type: text/html; charset=ISO-8859-1";

/*  	char linea[1024];
	char get[1024];
	char host[1024];
	char server[1024];
	char * aux;
	char * aux2;
	int posicion;
	int i=0;
	char* array[10];
/*
  	aux = strtok(cabecera,"\r\n");
  	strcpy(linea,aux);
	printf ("%s\n",linea);
	aux = strtok(NULL,"\r\n");
  	strcpy(linea1,aux);
	printf ("%s\n",linea1);
	array[5]=&linea[0];
	printf ("array: %s\n",array[5]);


	aux = strtok(cabecera,"\r\n");
	while (aux != NULL)
  	{
		memset(linea, '\0', sizeof(linea));
		strcpy(linea,aux);
		array[i]=&linea[0];
		aux2=strstr(aux,"GET");
		if(aux2!=NULL)
		{
			strcpy(get,linea);
		}
		aux2=strstr(aux,"Host");
		if(aux2!=NULL)
		{
			strcpy(host,linea);
		}
		aux2=strstr(aux,"Server");
		if(aux2!=NULL)
		{
			strcpy(server,linea);
		}
    		printf ("%d: %s\n",i,array[i]);
    		aux = strtok (NULL, "\r\n");
		i++;
  	}
	
	printf ("get: %s\n",get);
	printf ("host: %s\n",host);
	printf ("server: %s\n",server);
*/
	int fd;
	char buffer[BUFSIZE];
	if((fd = open("web.html",O_RDONLY)) >= 0) 
	{
		int leido=read(fd,buffer,BUFSIZE);
		printf("%s",buffer);
		int i=strlen(buffer);
		printf("%d",i);
	}













	

	return 0;
}
