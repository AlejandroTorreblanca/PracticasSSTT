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

char * crearCabecera(char * nombreArchivo, int n)
{
	
    //Obtenemos la fecha actual
    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    char fecha[128];
    strftime(fecha,128,"%a,%d %b %Y %H:%M:%S",tlocal);
    
    //Obtenemos la fecha de la ultima modificación del archivo.
    struct stat sb;
    if (stat(nombreArchivo, &sb)== -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    char * aux=ctime(&sb.st_mtime);
    char * modificado = strtok(aux,"\n");

    //Obtenemos el Tamaño del archivo.
    char tam[20];
    sprintf(tam, "%lld", (long long) sb.st_size);
	tamArchivo=sb.st_size;

    //Obtenemos el tipo del archivo.
    char * extension=strchr(nombreArchivo,'.');
	int i=0;
	int control=0;
	while((control==0) && (i<10))
	{
		if(strcmp(extension,extensions[i].ext)==0)
			control=1;
		else i++;
	}
	if (i==10) {
        perror("Extensión no compatible.");
        exit(EXIT_FAILURE);
    }
	
	//Calculamos la fecha de expiración de la cookie y el id.
	char fechaCookie1[128];
	char id[20];
    sprintf(id, "%d",n);
	time_t tiempo2 = time(0);
	tiempo2+=864000;				//Le suamamos los segudnos de diez días.
    struct tm *tlocal2 = localtime(&tiempo2);
    strftime(fechaCookie1,128,"%a,%d %b %Y %H:%M:%S GTM;\r\n\r\n",tlocal2);

    //Los siguientes datos son fijos.
    char cabecera1[BUFSIZE]= "HTTP/1.1";
	char * cabeceraOK=" 200 OK\r\nDATE :";
	char * cabeceraPC=" 206 Partial Content\r\nDATE :";
    char * cabecera2=" GMT \r\nServer: Apache/2.0.52 (CentOS)\r\nLast-Modified :";
    char * cabecera3=" GMT\r\nETag: \"";
	char * cabecera4="\"\r\nAccept-Ranges: bytes\r\nContent-Length: ";
    char * cabecera5="\r\nKeep-Alive: Close\r\nContent-Type:"; 
    char * cabecera6="; charset=ISO-8859-1\r\nSet-Cookie: id:";
	char * cabecera7="; Expires=";
    
    //Concatenamos todos los strings.
	strcat(cabecera1,cabeceraOK);
	strcat(cabecera1,fecha);
	strcat(cabecera1,cabecera2);
	strcat(cabecera1,modificado);
	strcat(cabecera1,cabecera3);
	strcat(cabecera1,nombreArchivo);
	strcat(cabecera1,cabecera4);
	strcat(cabecera1,tam);
	strcat(cabecera1,cabecera5);
	strcat(cabecera1,extensions[i].filetype);
	strcat(cabecera1,cabecera6);
	strcat(cabecera1,id);
	strcat(cabecera1,cabecera7);
	strcat(cabecera1,fechaCookie1);
	char * cabeceraRespuesta=malloc(sizeof(char)*BUFSIZE);
	strcpy(cabeceraRespuesta,&cabecera1[0]);
	return cabeceraRespuesta;
}
main(){
	
	char * a=crearCabecera("imagen.gif", 1);
	char * b=crearCabecera("imagen5.gif", 2);
	fprintf(stderr,"%s\n\n%s",a,b);
	return 0;
}






















