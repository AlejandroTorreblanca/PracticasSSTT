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
#define TAMMAX 89056

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

//Creamos la cabecera del mensaje de respuesta.
char * crearCabecera(char * nombreArchivo, int id)
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
	
	

    //Los siguientes datos son fijos.
    char cabecera1[BUFSIZE]= "HTTP/1.1";
	char * cabeceraOK=" 200 OK\r\nDATE :";
	char * cabeceraPC=" 206 Partial Content\r\nDATE :";
    char * cabecera2=" GMT \r\nServer: Apache/2.0.52 (CentOS)\r\nLast-Modified :";
    char * cabecera3=" GMT\r\nETag: \"";
	char * cabecera4="\"\r\nAccept-Ranges: bytes\r\nContent-Length: ";
    char * cabecera5="\r\nKeep-Alive: Close\r\nContent-Type:"; 
    char * cabecera6="; charset=ISO-8859-1\r\n";
	char * cabecera7="Set-Cookie: id:";
    
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
	
	
	if(id!=0)
	{
	//Calculamos la fecha de expiración de la cookie y el id.
		char fechaCookie1[128];
		char cadenaID[20];
		sprintf(cadenaID, "%d",id);
		time_t tiempo2 = time(0);
		tiempo2+=864000;				//Le suamamos los segundos de diez días.
		struct tm *tlocal2 = localtime(&tiempo2);
		strftime(fechaCookie1,128,"; Expires=%a,%d %b %Y %H:%M:%S GTM;\r\n\r\n",tlocal2);
	
		strcat(cabecera1,cabecera7);
		strcat(cabecera1,cadenaID);
		strcat(cabecera1,fechaCookie1);
	}
	else 
		strcat(cabecera1,"\r\n");

	char * cabeceraRespuesta=malloc(sizeof(char)*BUFSIZE);
	strcpy(cabeceraRespuesta,&cabecera1[0]);
	return cabeceraRespuesta;
}

//Creamos y enviamos por el socket el mensaje http response.
void sendResponse(char * nombreArchivo, int descriptorFichero, int id)
{
    int buffer[BUFSIZE];
	int total=0;
    int fd=open(nombreArchivo,O_RDONLY);
	if(fd >= 0) 
	{
        char * cabeceraRespuesta=crearCabecera(nombreArchivo,id);
		fprintf(stderr,"\n\n**********\ncabecera: \n%s\n**********\n",cabeceraRespuesta);
		int escritos=write(descriptorFichero,cabeceraRespuesta,strlen(cabeceraRespuesta));
		int totalcabecera=escritos;
		int leido=read(fd,buffer,BUFSIZE);      
		while(leido!=0)
		{	   
		    escritos=write(descriptorFichero,buffer,leido);
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
	else
	{
	 	perror("open");
     	exit(EXIT_FAILURE);
	}
    close(fd);
}

//Obtenemos el nombre del archivo solicitado en el http request.
char * obtenerNombre(char * cadena)
{
    char buffer[BUFSIZE];
	char * aux;
	char * aux2;
	strcpy(buffer,cadena);
	aux = strtok(buffer,"/");
	aux2 = strtok (NULL, "/");
	if(strcmp(aux2," HTTP")==0) 
    {
        char * respuesta="index.html";
        return respuesta;
    }
	else 
	{
		strcpy(buffer,aux2);
		aux = strtok(buffer," ");
        return aux;
	}
}

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
    fprintf (stderr,"LLega un nuevo mensaje por un socket\n");
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
        fprintf (stderr,"Hemos leido:\n %s", buffer);

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
	if(strcmp(nombreArchivo,"index.html")==0) 
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
