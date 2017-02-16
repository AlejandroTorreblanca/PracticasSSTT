#include <stdio.h>
#include <stdlib.h>
#include <string.h>

main(){

	char cabecera[]= "HTTP/1.1 200 OK\r\nDATE: Sun,26 Sep 2010 20:09:20 GMT \r\nServer: Apache/2.0.52 (CentOS)\r\nLast-Modified: Tue, 30 Oct 2007 17:00:02 GMT\r\nETag: \"17dc6-a5c-bf7127272\"\r\nAccept-Ranges: bytes\r\nContent-Length: \r\nKeep-Alive: timeout=10, max=100\r\nConnection: Keep-Alive\r\nContent-Type: text/html; charset=ISO-8859-1\r\n\r\n";

  	char * linea;
	int i=0;
	char* array[10];
  	linea = strtok (cabecera,"\r");
  	while (linea != NULL)
  	{
		array[i]=linea;
    	printf ("%d: %s\n",i,array[i]);
    	linea = strtok (NULL, "\r\n");
		i++;
  	}
	printf ("%s\n",array[2]);
	char * w=strrchr(array[2],' ');
	printf ("%s\n",w);



	

	return 0;
}
