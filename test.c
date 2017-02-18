#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define BUFSIZE		8096
#define TAMMAX 89056

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

	
    //Los siguientes datos son fijos.
    char cabecera1[BUFSIZE]= "HTTP/1.1";
	char * cabeceraOK=" 200 OK\r\nDATE :";
	char * cabeceraPC=" 206 Partial Content\r\nDATE :";
    char * cabecera2=" GMT \r\nServer: Apache/2.0.52 (CentOS)\r\nLast-Modified :";
    char * cabecera3=" GMT\r\nETag: \"";
	char * cabecera4="\"\r\nAccept-Ranges: bytes\r\nContent-Length: ";
    char * cabecera5="\r\nKeep-Alive: timeout=10, max=100\r\nConnection: Keep-Alive\r\nContent-Type:"; 
    char * cabecera6="; charset=ISO-8859-1\r\n\r\n";
    
    //Concatenamos todos los strings.
	if(sb.st_size<TAMMAX)	//Se puede enviar todo el archivo en un solo mensaje.
	{	
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
		char * cabeceraRespuesta=&cabecera1[0];
		fprintf(stderr,"cabecera:\n%s \n",cabeceraRespuesta);
		return cabeceraRespuesta;
	}
	else	//Debemos realizar el envio por trozos.
	{
		char t[100];
		int aux;
		strcat(cabecera1,cabeceraPC);
		strcat(cabecera1,fecha);
		strcat(cabecera1,cabecera2);
		strcat(cabecera1,modificado);
		strcat(cabecera1,cabecera3);
		strcat(cabecera1,nombreArchivo);
		strcat(cabecera1,cabecera4);
		int taux=TAMMAX;
		if((sb.st_size-(n-1)*TAMMAX)>TAMMAX)
		{
			sprintf(t, "%d", TAMMAX);
			strcat(cabecera1,t);			
		}
		else
		{
			taux=(sb.st_size-(n-1)*TAMMAX);
			sprintf(t, "%d", taux);
			strcat(cabecera1,t);
		}
		char * cabecera7="\r\nContent-Range: bytes ";
		strcat(cabecera1,cabecera7);
		aux=TAMMAX*(n-1);
		sprintf(t, "%d",aux);
		strcat(cabecera1,t);
		strcat(cabecera1,"-");
		aux=TAMMAX*(n-1)+taux-1;
		sprintf(t, "%d", aux);
		strcat(cabecera1,t);
		strcat(cabecera1,"/");
		strcat(cabecera1,tam);
		char * cabecera8="\r\nContent-Type:";
		strcat(cabecera1,cabecera8);
		strcat(cabecera1,extensions[i].filetype);
		char * cabeceraRespuesta=&cabecera1[0];
		fprintf(stderr,"cabecera:\n%s \n",cabeceraRespuesta);
		return cabeceraRespuesta;	
	}
 
}

main(){
	
	char * a=crearCabecera("index.html", 1);

	fprintf(stderr,"cabecera1:\n%s \n",a);
	fprintf(stderr,"\n**********\n");
	char * b=crearCabecera("imagen2.jpg", 1);
	fprintf(stderr,"cabecera2:\n%s \n",b);
	

	int n=7/2;
	printf("%d\n",n);

	return 0;
}






















