#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <signal.h>
#include "config.cgi"

#define MAX_CONNECTION 5

int     sockfd;
int     PUERTO;
char    DIRECTORIO[128];
char    PAGINA[128];
char    USUARIO[128];
char    PASS[128];

void error(const char *msg){
    perror(msg);
    exit(1);
}

char * comparar(char extension[265]){
	char comando[255];
	char *tipo_mime;
	tipo_mime = (char*)malloc (50*sizeof(char));
	FILE *p;
	sprintf(comando,"awk '{for(i=2;i<=NF;i++){if($i==\"%s\")print $1}}' /etc/mime.types",extension);

	p = popen(comando,"r");
	if(p==NULL){
		error("\n#ERROR: POPEN");
	}
	fscanf(p,"%s",tipo_mime);
	printf("VALOR DE RETORNO %s",tipo_mime);
    
	return tipo_mime;
}


void process_request(int newsockfd){
	struct stat longitud;
	struct sockaddr_in cli_addr;
	socklen_t clilen;
	FILE *g;
	int n,u,i,num_bytes;
	char recurso[256], extension[256], metodo[256], version[256], cono[256], *tmp=NULL;
	char buffer[1024], buffer2[256], user[128], password[128], aux[128], *envio;
	//char * salida_comparar;
	//buffer2 = (char*)malloc ( 50*sizeof(char) );
	//envio = (char*)malloc ( 99999*sizeof(char) );
	clilen = sizeof(cli_addr);
	bzero(buffer,1024);
	
	//leemos del socket hasta 255 bytes
	n = read(newsockfd,buffer,1024);
    
	sscanf(buffer,"%s %s %s",metodo, recurso, version);
	strcpy(cono,DIRECTORIO);
	printf("RECURSO: %s METODO: %s VERSION: %s\n",recurso, metodo, version);	
	if (n < 0){
		error("\n#ERROR: reading from socket");
    }
	printf("\n->Here is the message: %s\n",buffer);

	//devolvemos la respuesta
	//n = write(newsockfd,"I got your message\n",19);
	
	u=strcmp(recurso,"/");
	if(u==0){
		strcpy(recurso,PAGINA);
	}
	strcpy(extension,recurso);
	tmp=strtok(extension,".");
	tmp=strtok(NULL,".");

	if(tmp==NULL){
		strcpy(extension,"octectstream");
	}
	else{
		strcpy(extension,tmp);
	}
	
	u=strcmp("GET",metodo);
	if (u==0){
	printf("\n\t--> Metodo GET:\n");
		if (strcmp("HTTP/1.1",version)==0 ||strcmp("HTTP/1.0",version) == 0){
            printf("\n\t---> Version HTTP correcta\n");
            if (strcmp(recurso,"/")==0){
                strcat(cono,PAGINA);
                g=fopen(cono,"r");
                if (g==NULL){
                    printf("\n\t---> Error: Abriendo la pagina por defecto\n");
                    exit(1);
                }
                // Rellena una Estructura con los datos del archivo; a nosotros nos interesa el campo st_size
                stat(cono,&longitud);
                sprintf(buffer2,"HTTP/1.1 200 OK\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n",extension, (int)longitud.st_size);
                n=write(newsockfd,buffer2,strlen(buffer2));
                if (n < 0)
                    error("\n#ERROR: writing from socket");
                envio = (char*)malloc ( longitud.st_size );
                i=fread(envio,longitud.st_size,1,g);//Abre el archivo g le tamaño longitud.st_size de caracter en caracter(1) y lo guarda en envio
                //comprobar i
                n=write(newsockfd,envio,(int)longitud.st_size);
                if (n < 0) 
                    error("\n#ERROR: writing from socket");
                fclose(g);
                free(envio);
            }
            // Nuevo
            else if (strcmp(recurso,"/status.cgi")==0){
                strcat(cono,"/status.cgi");
                sprintf(buffer2,"localhost:%d",PUERTO);
                setenv("SERVER_NAME",buffer2,1);
                setenv("REQUEST_METHOD",metodo,1);
                g=popen(cono,"r");
                buffer[0] = '\0';
                while(fgets(aux,256,g)!=NULL){
                    strcat(buffer,aux);
                }
                i=strlen(buffer);			
                if (g==NULL){
                    error("\n\t--> Error al leer status.cgi\n");			
                }	
                sprintf(buffer2,"HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-length: %d\r\n\r\n",i);
                n=write(newsockfd,buffer2,strlen(buffer2));
                n=write(newsockfd,buffer,i);
                if (n < 0) 
                    error("\n#ERROR: writing from socket");
                fclose(g);				//
            }
            else {
                strcat(cono,recurso);
                g=fopen(cono,"r");
                if (g==NULL){	
                    strcpy(cono,DIRECTORIO);
                    strcat(cono,"404.html");			
                    stat (cono,&longitud);			
                    sprintf(buffer2,"HTTP/1.1 404 NOT FOUND\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n",extension,(int)longitud.st_size);
                    n=write(newsockfd,buffer2,strlen(buffer2));
                        
                    if (n < 0){
                        error("\n#ERROR: writing from socket\n");
                    }
                    g=fopen(cono,"r");
                    envio = (char*)malloc ( longitud.st_size );			
                    i=fread(envio,longitud.st_size,1,g);
                    n=write(newsockfd,envio,(int)longitud.st_size);
                    
                    if (n < 0){
                        error("\n#ERROR: writing from socke\nt");
                    }
                    fclose(g);
                    free(envio);
                }
                else {
                    stat(cono,&longitud);
                    sprintf(buffer2,"HTTP/1.1 200 OK\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n",extension,(int)longitud.st_size);
                    n=write(newsockfd,buffer2,strlen(buffer2));
                    if (n < 0){
                        error("\n#ERROR: writing from socket\n");
                    }
                    envio = (char*)malloc ( longitud.st_size );
                    i=fread(envio,longitud.st_size,1,g);//Abre el archivo g de tamaño longitud.st_size de caracter en caracter(1) y lo guarda en envio
                    n=write(newsockfd,envio,(int)longitud.st_size);
                    if (n < 0){
                        error("\n#ERROR: writing from socket\n");
                    }
                    fclose(g);
                    free(envio);			
                }
            }
		}	
	}
    // Se eres un POST
	else{
		u=strcmp("POST",metodo);

		if (u==0){
		printf("\n\t--> Metodo POST:\n");
			if(strcmp("HTTP/1.1",version)==0 ||strcmp("HTTP/1.0",version) == 0){
				printf("\n\t---> Version HTTP correcta\n");
                
                strcpy(buffer2,buffer);
                tmp=strtok(buffer2,"\r\n");
                while((tmp=strtok(NULL,"\r\n"))!=NULL){	
                    if (strncmp(tmp,"Content-Length: ",strlen("Content-Length: "))==0){
                        sscanf(tmp,"Content-Length: %d\r\n",&num_bytes);
                    }
                    if ((strncmp(tmp,"username=",strlen("username="))) == 0){
                        tmp=strpbrk(tmp,"=");
                        tmp++;//Sumo uno a la posicion de memoria para saltar al siguiente caracter
                        strcpy(user,strtok(tmp,"&"));							
                    }
                    if ((strncmp(tmp,"pass=",strlen("pass="))) == 0){
                        tmp=strpbrk(tmp,"=");
                        tmp++;//Sumo uno a la posicion de memoria para saltar al siguiente caracter
                        strcpy(password,strtok(tmp,"&"));					
                    }
                }
                /*i=login(user,password,DIRECTORIO,newsockfd); //Esto de momento no hace nada
                if (i==-1){
                    error("\n\nERROR al autentificar\n\n");
                }
                */
                printf("\n\n\nContent-Length=%d user=%s pass=%s\n\n\n",num_bytes,user,password);
                
                if(strcmp(recurso,"/")==0){
                    strcat(cono,PAGINA);
                    g=fopen(cono,"r");
                    if (g==NULL){
                        printf("\n\t---> Error: Abriendo la pagina por defecto\n");
                        exit(1);
                    }
                }
			}
		}
	}
	if (n < 0){
		error("\n#ERROR: writing to socket\n");
    }
	// Cerramos el socket nuevo que se ha creado en el otro sentido
	close(newsockfd);
}

// Signal de Interupción
void handle(int nsignal){
	printf("\n--\nServer is shutting down now...\n");
	close(sockfd);
	exit(0);
}

int main(int argc, char *argv[]){
	FILE *f;
		
	int portno;
	
	struct sockaddr_in serv_addr,cli_addr;
	int n, newsockfd;
    
    // Datos del fichero de configuracion
	f=fopen(".config","r");
	if (f==NULL){
		error("\n\t-> Archivo .config no encontrado\n");
	}
	
    fscanf(f,"%d\r\n",&PUERTO);
    fscanf(f,"%s\r\n",DIRECTORIO);
    fscanf(f,"%s\r\n",PAGINA);
    fscanf(f,"%s\r\n",USUARIO);
    fscanf(f,"%s\r\n",PASS);
	fclose(f);

	// Crear un socket para manejar conexiones TCP
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		error("\n#ERROR: Opening socket");
    }

	// Preparar una estructura con informacion sobre como vamos
	// a recibir las conexiones TCP
	// En este ejemplo, enlazaremos el socket a la IP local al puerto TCP PORT
	// Esto lo conseguiremos inicializando
	// la estructura a cero mediante, por ejemplo, 'bzero'.
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = PUERTO;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		error("\n#ERROR: on binding");
    }

	// Activaremos una propiedad del socket que permitira que otros
	// sockets puedan reutilizar cualquier puerto al que nos enlacemos.
	// Esto permitira en protocolos como el TCP, poder ejecutar un
	// mismo programa varias veces seguidas y enlazarlo siempre al
	// mismo puerto. De lo contrario habria que esperar a que el puerto
	// quedase disponible (TIME_WAIT en el caso de TCP)
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&serv_addr,sizeof(struct sockaddr_in))==-1){
 		error("\n#ERROR: Fallo al reutilizar el puerto.\n");
	}

	/* Habilitar socket para recibir conexiones */
	if (listen(sockfd, MAX_CONNECTION) == -1){
		printf("\n#ERROR: en listen\n");
		close(sockfd);
		return -1;
	}

	/* Captura la senal SIGINT */
	if(signal(SIGINT,handle)==SIG_ERR){
		error("\n#ERROR: Fallo al capturar la senal SIGINT.\n");
	}

	/* While (TRUE) */
	while (1){
		/* Esperar conexion */
		n = sizeof(struct sockaddr_in);
		if ((newsockfd=accept(sockfd,(struct sockaddr*)&cli_addr,(socklen_t *) &n)) == -1){
			printf("\n#ERROR: en accept\n");
			continue;
		}
		/* procesar peticion */
		process_request(newsockfd);
	}
	close(sockfd);
	return 0; 
}