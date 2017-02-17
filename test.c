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
	char * nombreArchivo="index.html";
	//Obtenemos la fecha actual
    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    char fecha[128];
    strftime(fecha,128,"%a,%d %b %Y %H:%M:%S",tlocal);
    
    //Obtenemos la fecha de la ultima modificación del archivo.
    struct stat sb;
    if (stat(nombreArchivo, &sb) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    char * aux=ctime(&sb.st_mtime);
    char * modificado = strtok(aux,"\n");

    //Obtenemos el Tamaño del archivo.
    char tam[20];
    sprintf(tam, "%lld", (long long) sb.st_size);

    //Obtenemos el tipo del archivo.
    char * extension=strchr(nombreArchivo,'.');
	int i=0;
	int control=0;
	while((control==0) && (i<9))
	{
		if(strcmp(extension,extensions[i].ext)==0)
			control=1;
		else i++;
	}
    
    //Los siguientes datos son fijos.
    char cabecera1[8096]= "HTTP/1.1 200 OK\r\nDATE :";
    char cabecera2[]=" GMT \r\nServer: Apache/2.0.52 (CentOS)\r\nLast-Modified :";
    char cabecera3[]=" GMT\r\nETag: \"17dc6-a5c-bf7127272\"\r\nAccept-Ranges: bytes\r\nContent-Length: ";
    char cabecera4[]="\r\nKeep-Alive: timeout=10, max=100\r\nConnection: Keep-Alive\r\nContent-Type:"; 
    char cabecera5[]="; charset=ISO-8859-1\r\n\r\n";
    
    //Concatenamos todos los strings.
    char * cabecera;
	strcat(cabecera1,fecha);
	strcat(cabecera1,cabecera2);
	strcat(cabecera1,modificado);
	strcat(cabecera1,cabecera3);
	strcat(cabecera1,tam);
	strcat(cabecera1,cabecera4);
	strcat(cabecera1,extensions[i].filetype);
	strcat(cabecera1,cabecera5);
	printf("%s",cabecera1);







	

	return 0;
}
