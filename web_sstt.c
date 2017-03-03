#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <time.h>

#define VERSION	24
#define BUFSIZE	8096
#define ERROR 42
#define LOG	44
#define PROHIBIDO 403
#define NOENCONTRADO 404
#define TEXPIRACION 600

int bEnviados;
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

void debug(int log_message_type, char *message, char *additional_info, int socket_fd)
{
	int fd ;
	char logbuffer[BUFSIZE*2];
	
	switch (log_message_type) {
		case ERROR: (void)sprintf(logbuffer,"ERROR: %s:%s Errno=%d exiting pid=%d",message, additional_info, errno,getpid());
			break;
		case PROHIBIDO:
			// Enviar como respuesta 403 Forbidden
			(void)sprintf(logbuffer,"FORBIDDEN: %s:%s",message, additional_info);
			break;
		case NOENCONTRADO:
			// Enviar como respuesta 404 Not Found
			(void)sprintf(logbuffer,"NOT FOUND: %s:%s",message, additional_info);
			break;
		case LOG: (void)sprintf(logbuffer," INFO: %s:%s:%d",message, additional_info, socket_fd); break;
	}

	if((fd = open("webserver.log", O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0) {
		(void)write(fd,logbuffer,strlen(logbuffer));
		(void)write(fd,"\n",1);
		(void)close(fd);
	}
	if(log_message_type == ERROR || log_message_type == NOENCONTRADO || log_message_type == PROHIBIDO) exit(3);
}

/*
Creamos la cabecera del mensaje de respuesta.
-nombreArchivo es una cadena con el nombre del archivo que se va a enviar al cliente.
-codigo 
-id es un entero que se enviará como cookie al cliente, en el caso de ser un 0 no se enviará la cookie.
*/
char * crearCabecera(char * nombreArchivo, int codigo, int id)
{
	//Los siguientes datos son fijos.
    char cabecera1[BUFSIZE]= "HTTP/1.1 ";
	char * cabecera2="\r\nDATE: ";
    char * cabecera3="Server: Apache/2.0.52 (CentOS)\r\n";
	char * cabecera4="Last-Modified: ";
    char * cabecera5=" GMT\r\nETag: \"";
	char * cabecera6="\"\r\nAccept-Ranges: bytes\r\nContent-Length: ";
    char * cabecera7="\r\nKeep-Alive: Close\r\nContent-Type:"; 
    char * cabecera8="; charset=ISO-8859-1\r\n";
	char * cabecera9="Set-Cookie: access_counter:";

    //Obtenemos la fecha actuals
    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    char fecha[128];
    strftime(fecha,128,"%a,%d %b %Y %H:%M:%S GTM \r\n",tlocal);
    char * cabeceraAux;
	char * cod;
	switch(codigo)
	{
		case 200:
			cod="200 OK";
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
			char * extension=strchr(nombreArchivo,'/');
            extension=strchr(extension,'.');
			int i=0;
			int control=0;
			while((control==0) && (i<10))
			{
				if(strcmp(extension,extensions[i].ext)==0)
					control=1;
				else i++;
			}
			if (i==10) { //La extensión del archivo no esta soportada por el servidor.
            //Esta comprobación también nos sirve para no permitir accesos fuera del directorio.
				cod="400 Bad Request";
                cabeceraAux="Content-Length: 86\r\nKeep-Alive: Close\r\nContent-Type: text/html\r\ncharset=ISO-8859-1\r\n\r\n<HTML><HEAD><TITLE>web_SSTT</TITLE></HEAD><BODY><H1>400 BAD REQUEST</H1></BODY></HTML>";
				strcat(cabecera1,cod);
				strcat(cabecera1,cabecera2);
				strcat(cabecera1,fecha);
				strcat(cabecera1,cabecera3);
				strcat(cabecera1,cabeceraAux);
			}
			else
			{
				//Concatenamos todos los strings.
				strcat(cabecera1,cod);
				strcat(cabecera1,cabecera2);
				strcat(cabecera1,fecha);
				strcat(cabecera1,cabecera3);
				strcat(cabecera1,cabecera4);
				strcat(cabecera1,modificado);
				strcat(cabecera1,cabecera5);
				strcat(cabecera1,nombreArchivo);
				strcat(cabecera1,cabecera6);
				strcat(cabecera1,tam);
				strcat(cabecera1,cabecera7);
				strcat(cabecera1,extensions[i].filetype);
				strcat(cabecera1,cabecera8);
	
				if(id!=0)	//En los casos en los que haya que añadir una cookie a la cabecera:
				{
				//Calculamos la fecha de expiración de la cookie y el id.
					char fechaCookie1[128];
					char cadenaID[20];
					sprintf(cadenaID, "%d",id);
					time_t tiempo2 = time(0);
					tiempo2+=TEXPIRACION;				
					struct tm *tlocal2 = localtime(&tiempo2);
					strftime(fechaCookie1,128,"; Expires=%a,%d %b %Y %H:%M:%S GTM;\r\n\r\n",tlocal2);
	
					strcat(cabecera1,cabecera9);
					strcat(cabecera1,cadenaID);
					strcat(cabecera1,fechaCookie1);
				}
				else 	//Si en la la cabecera no deseamos añadir ninguna cookie:
					strcat(cabecera1,"\r\n");
			}
		break;
		case 404:
			cod="404 Not Found";
            cabeceraAux="Content-Length: 84\r\nKeep-Alive: Close\r\nContent-Type: text/html\r\ncharset=ISO-8859-1\r\n\r\n<HTML><HEAD><TITLE>web_SSTT</TITLE></HEAD><BODY><H1>404 NOT FOUND</H1></BODY></HTML>";
			strcat(cabecera1,cod);
			strcat(cabecera1,cabecera2);
			strcat(cabecera1,fecha);
			strcat(cabecera1,cabecera3);
			strcat(cabecera1,cabeceraAux);
		break;
		case 429:
			cod="429 Too Many Requests";
            cabeceraAux="Content-Length: 92\r\nKeep-Alive: Close\r\nContent-Type: text/html\r\ncharset=ISO-8859-1\r\n\r\n<HTML><HEAD><TITLE>web_SSTT</TITLE></HEAD><BODY><H1>429 TOO MANY REQUESTS</H1></BODY></HTML>";
			strcat(cabecera1,cod);
			strcat(cabecera1,cabecera2);
			strcat(cabecera1,fecha);
			strcat(cabecera1,cabecera3);
			strcat(cabecera1,cabeceraAux);
		break;
		default:
			cod="418";
			strcat(cabecera1,cod);
			strcat(cabecera1,cabecera2);
			strcat(cabecera1,fecha);
			strcat(cabecera1,cabecera3);
	}
	char * cabeceraRespuesta=malloc(sizeof(char)*BUFSIZE);
	strcpy(cabeceraRespuesta,&cabecera1[0]);
	return cabeceraRespuesta;
}

void enviarCabecera(int descriptorFichero,char* cabecera)
{
	int escritos=write(descriptorFichero,cabecera,strlen(cabecera));
	while(escritos<strlen(cabecera))
	{
		int aux=escritos/4;
		int escritosAux=write(descriptorFichero,cabecera+aux,strlen(cabecera)-escritos);
		if(escritosAux>0)
		{
		 	escritos+=escritosAux;	
		}
	}
}

/*
Creamos y enviamos por el socket el mensaje http response.
-nombreArchivo es una cadena con el nombre del archivo que se va a enviar al cliente.
-descriptorFichero es un entero con el descriptor de fichero del socket en el que deseamos escribir los datos.
-id es un entero con el valor que queremos darle a la cookie.
*/
void sendResponse(char * nombreArchivo, int descriptorFichero, int id)
{
    int buffer[BUFSIZE];
	int total=0;
    int fd=open(nombreArchivo,O_RDONLY);
	if(fd >= 0) 
	{
		if(id<=5) //Si el ID de la cookie es menor que 5 enviamos los datos.
		{
		    char * cabeceraRespuesta=crearCabecera(nombreArchivo,200,id);
			fprintf(stderr,"\n\n**********\ncabecera: \n%s\n**********\n",cabeceraRespuesta);
			enviarCabecera(descriptorFichero, cabeceraRespuesta);
			int leido=read(fd,buffer,BUFSIZE);      
			while(leido!=0)
			{	   
				int escritos=write(descriptorFichero,buffer,leido);
				if (escritos>0)
				{
					while(escritos<leido)
					{
						fprintf(stderr,"Escritura parcial, leidos: %d, escritos: %d\n",leido,escritos);
						int aux=escritos/4;
						int escritosAux=write(descriptorFichero,buffer+aux,leido-escritos);	
						if(escritosAux>0)
						{
						 	escritos+=escritosAux;	
						}						
					} 
					total+=escritos;
					leido=read(fd,buffer,BUFSIZE);	
				}			
			}
			fprintf(stderr,"Escritura completada, escritos: %d\n\n",total);
		}
		else //En otro caso enviamos un mensaje de error 429.
		{
			char * cabeceraRespuesta=crearCabecera(nombreArchivo,429,id);
			fprintf(stderr,"\n\n**********\ncabecera: \n%s\n**********\n",cabeceraRespuesta);
			enviarCabecera(descriptorFichero, cabeceraRespuesta);
		}
	}
	else //Si no podemos abrir el fichero correctamente enviamos un error 404.
	{
	 	perror("open");
     	char * cabeceraRespuesta=crearCabecera(nombreArchivo,404,id);
		fprintf(stderr,"\n\n**********\ncabecera: \n%s\n**********\n",cabeceraRespuesta);
		enviarCabecera(descriptorFichero, cabeceraRespuesta);
	}
    close(fd);
}

/*
Obtenemos el nombre del archivo solicitado en el http request.
-cadena es la linea de la cabecera donde aparece el nombre del archivo solicitado por el cliente.
*/
char * obtenerNombre(char * cadena)
{
    char buffer[BUFSIZE];
	char * aux;
	strcpy(buffer,cadena);
	strtok(buffer," ");
	aux =strtok (NULL, " ");
	if(strcmp(aux,"/")==0) 
    {
		return "./index.html";
    }
	else 
	{
		strcpy(buffer,".");
		strcat(buffer,aux);
		char * salida=malloc(sizeof(char)*BUFSIZE);
		strcpy(salida,&buffer[0]);
		return salida;
	}
}

/*
Obtenemos el número de la cookie.
-cadena es la linea en la que se encuetra el identificador de la cookie.
*/
int obtenerID(char * cadena)
{
	char buffer[BUFSIZE];
	char * aux;
	strcpy(buffer,cadena);
	strtok(buffer,":");
	strtok (NULL, ":");
	aux= strtok (NULL, ":");
	return atoi(aux);
}

void process_web_request(int descriptorFichero)
{
	debug(LOG,"request","Ha llegado una peticion",descriptorFichero);
	char buffer[BUFSIZE];
	char linea[1024];
	char get[1024];
	char cookie[1024];
	char * aux;
	char * aux2;
	char * aux3;
	int posicion;
	int i=0;
    int g=0;
    char* array[10];
	int control=0;

	int flags=fcntl(descriptorFichero, F_GETFL, 0);
	fcntl(descriptorFichero, F_SETFL, flags|O_NONBLOCK);

    int leido=read(descriptorFichero,buffer,BUFSIZE);
	while(leido<=0)
        leido=read(descriptorFichero,buffer,BUFSIZE);    
	while((leido>0))
	{
        fprintf (stderr,"Hemos leido:\n%s", buffer);

        aux = strtok(buffer,"\r\n");
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
			aux3=strstr(aux,"Cookie:");
		    if(aux3!=NULL)
		    {
				control=1;
			    strcpy(cookie,linea);
		    }
            aux = strtok (NULL, "\r\n");
		    i++;
        } 
		leido=read(descriptorFichero,buffer,BUFSIZE);
	}

    char * nombreArchivo=obtenerNombre(get);
	if(strcmp(nombreArchivo,"./index.html")==0) 
    {     
		if(control==0)
		{
			sendResponse(nombreArchivo,descriptorFichero,1);
		}
		else
		{
			int cookieID=obtenerID(cookie);
			sendResponse(nombreArchivo,descriptorFichero,cookieID+1);
		}
    }
	else 
	{
   		sendResponse(nombreArchivo,descriptorFichero,0);
	}
    
    
   
	
	//
	// Definir buffer y variables necesarias para leer las peticiones
	//
	
	
	//
	// Leer la petición HTTP
	//
	
	
	//
	// Comprobación de errores de lectura
	//
	
	
	//
	// Si la lectura tiene datos válidos terminar el buffer con un \0
	//
	
	
	//
	// Se eliminan los caracteres de retorno de carro y nueva linea
	//
	
	
	//
	//	TRATAR LOS CASOS DE LOS DIFERENTES METODOS QUE SE USAN
	//	(Se soporta solo GET)
	//
	
	
	//
	//	Como se trata el caso de acceso ilegal a directorios superiores de la
	//	jerarquia de directorios
	//	del sistema
	//
	
	
	//
	//	Como se trata el caso excepcional de la URL que no apunta a ningún fichero
	//	html
	//
	
	
	//
	//	Evaluar el tipo de fichero que se está solicitando, y actuar en
	//	consecuencia devolviendolo si se soporta u devolviendo el error correspondiente en otro caso
	//
	
	
	//
	//	En caso de que el fichero sea soportado, exista, etc. se envia el fichero con la cabecera
	//	correspondiente, y el envio del fichero se hace en blockes de un máximo de  8kB
	//
	
	close(descriptorFichero);
	exit(1);
}

int main(int argc, char **argv)
{
	int i, port, pid, listenfd, socketfd;
	socklen_t length;
	static struct sockaddr_in cli_addr;		// static = Inicializado con ceros
	static struct sockaddr_in serv_addr;	// static = Inicializado con ceros
	
	//  Argumentos que se esperan:
	//
	//	argv[1]
	//	En el primer argumento del programa se espera el puerto en el que el servidor escuchara
	//
	//  argv[2]
	//  En el segundo argumento del programa se espera el directorio en el que se encuentran los ficheros del servidor
	//
	//  Verficiar que los argumentos que se pasan al iniciar el programa son los esperados
	//

	//
	//  Verficiar que el directorio escogido es apto. Que no es un directorio del sistema y que se tienen
	//  permisos para ser usado
	//

	if(chdir(argv[2]) == -1){ 
		(void)printf("ERROR: No se puede cambiar de directorio %s\n",argv[2]);
		exit(4);
	}
	// Hacemos que el proceso sea un demonio sin hijos zombies
	if(fork() != 0)
		return 0; // El proceso padre devuelve un OK al shell

	(void)signal(SIGCHLD, SIG_IGN); // Ignoramos a los hijos
	(void)signal(SIGHUP, SIG_IGN); // Ignoramos cuelgues
	
	debug(LOG,"web server starting...", argv[1] ,getpid());
	
	/* setup the network socket */
	if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0)
		debug(ERROR, "system call","socket",0);
	
	port = atoi(argv[1]);
	
	if(port < 0 || port >60000)
		debug(ERROR,"Puerto invalido, prueba un puerto de 1 a 60000",argv[1],0);
	
	/*Se crea una estructura para la información IP y puerto donde escucha el servidor*/
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); /*Escucha en cualquier IP disponible*/
	serv_addr.sin_port = htons(port); /*... en el puerto port especificado como parámetro*/
	
	if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0)
		debug(ERROR,"system call","bind",0);
	
	if( listen(listenfd,64) <0)
		debug(ERROR,"system call","listen",0);
	
	while(1){
		length = sizeof(cli_addr);
		if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0)
			debug(ERROR,"system call","accept",0);
		if((pid = fork()) < 0) {
			debug(ERROR,"system call","fork",0);
		}
		else {
			if(pid == 0) { 	// Proceso hijo
				(void)close(listenfd);
				process_web_request(socketfd); // El hijo termina tras llamar a esta función
			} else { 	// Proceso padre
				(void)close(socketfd);
			}
		}
	}
}
