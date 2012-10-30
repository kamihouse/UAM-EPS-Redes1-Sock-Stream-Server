#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <signal.h>

#define MAX_CONNECTION 5

// Variables Globales
int     sockfd, PUERTO;
char    DIRECTORIO[128], PAGINA[128], USUARIO[128], PASS[128];

void error(const char *msg){
    perror(msg);
    exit(1);
}

char * comparar(char extension[265], char *tipo_mime){
	char    comando[255];
	FILE    *p;
    
    // Usando lo AWK para verificar lo archivo mime.types
    // Direcion: ./etc/mime.types
    // Referencia: mime.types APACHE
	sprintf(comando,"awk '{for(i=2;i<=NF;i++){if($i==\"%s\")print $1}}' etc/mime.types", extension);

	p = popen(comando,"r");
	if(p == NULL){
		error("\n\t#ERROR 2: Archivo 'mime.types' esta vazio.  [Linea: 33]");
	}
	fscanf(p, "%s", tipo_mime);
	return tipo_mime;
}

void process_request(int newsockfd){	
	struct  stat longitud;
	struct  sockaddr_in cli_addr;
	socklen_t clilen;
	FILE    *g = NULL, *config = NULL, *h = NULL;
	int     n, u, i, num_bytes;
	char    recurso[256], extension[256], metodo[256], version[256], cono[256], *tmp = NULL;
	char    buffer[1024], buffer3[8000], buffer2[256], user[128], password[128], aux[256], *envio = NULL;
	char    *tipo_mime = NULL;

	clilen = sizeof(cli_addr);
	bzero(buffer,1024);
	
	// Leemos el SOCK hasta 1025 bytes
	n = read(newsockfd,buffer,1024);
	sscanf(buffer,"%s %s %s",metodo, recurso, version);
	strcpy(cono,DIRECTORIO);
    printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
	printf("....:: Requisición de Procesamiento ::....\n\tRECURSO..: %s \n\tMETODO...: %s \n\tVERSION..: %s",recurso, metodo, version);	
	if (n < 0){
		error("\n\t#ERROR 6: Reading from socket. [Linea: 58]");
    }
	printf("\n\tMESSAGEN.: %s",buffer);

	// Devolvemos la respuesta
	u=strcmp(recurso,"/");
	if(u == 0){
		strcpy(recurso, PAGINA);
	}
	strcpy(extension, recurso);
	tmp = strtok(extension, ".");
	tmp = strtok(NULL, ".");

	if(tmp == NULL){
        // Definicion del ejercicio.
		strcpy(extension, "octectstream");
	} else{
        strcpy(extension, tmp);
	}
	tipo_mime = (char*)malloc (50*sizeof(char));
	tmp = comparar(extension, tipo_mime);
	strcpy(extension, tmp);
    // Liberando tipo_mime.
	free(tipo_mime);
	
	u = strcmp("GET", metodo);
	if (u == 0){
        printf("--> Se produjo una solicitud 'GET'");
		if(strcmp("HTTP/1.1", version) == 0 || strcmp("HTTP/1.0", version) == 0){
            printf("\n---> Version 'HTTP' esta correcta.\n");
            if(strcmp(recurso, "/") == 0){
                strcat(cono, PAGINA);
                g=fopen(cono,"r");
                if(g == NULL){
                    printf("\n\t#ERROR 3: Abriendo la pagina por defecto. [Linea: 93]");
                    exit(1);
                }
                // Rellena una estructura con los datos del archivo
                // Interesa el campo 'st_size' - longitud.st_size
                stat(cono, &longitud);
                sprintf(buffer2, "HTTP/1.1 200 OK\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n", extension, (int)longitud.st_size);
                n = write(newsockfd, buffer2, strlen(buffer2));
                
                if (n < 0){
                    error("\n\t#ERROR 4: Writing from socket. [Linea: 102]");
                }
                envio = (char*)malloc ( longitud.st_size );
                // Abre el archivo g de tamaño longitud.st_size de caracter en caracter y lo guarda en envio.
                i = fread(envio, longitud.st_size, 1, g);
                n = write(newsockfd, envio, (int)longitud.st_size);
                if (n < 0){
                    error("\n\t#ERROR 5: Writing from socket. [Linea: 109]");
                }
                fclose(g);
                free(envio);
            } else if(strcmp(recurso, "/status.cgi") == 0){
                strcpy(cono,DIRECTORIO);					
                strcat(cono,"status.cgi");
                sprintf(buffer2,"MyHttpServer");
                setenv("SERVER_NAME",buffer2,1);
                setenv("REQUEST_METHOD",metodo,1);
                g=popen(cono,"r");
                if (g==NULL){
                    printf("Error abriendo status.cgi\n");
                    exit(1);
                }
                
                buffer[0] = '\0';
                aux[0]='\0';	
                i=0;
                fgets (aux, 1000, g);
                while (!feof (g))
                {
                    strcat(buffer3,aux);
                    fgets (aux, 1000, g);

                }
                i=strlen(buffer3);			
                if (g==NULL){
                    error("\nError al leer status.cgi\n");			
                }	
                sprintf(buffer2,"HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-length: %d\r\n\r\n",i);
                n=write(newsockfd,buffer2,strlen(buffer2));
                n=write(newsockfd,buffer3,i);
                if (n < 0) 
                    error("ERROR writing from socket");
                pclose(g);
			} else{
                strcat(cono, recurso);
                g = fopen(cono, "r");
                if(g == NULL){
                    strcpy(cono,DIRECTORIO);
                    // Deve informar una pagina de ERROR no Encontrado.
                    strcat(cono, "404.html");			
                    stat (cono,&longitud);			
                    sprintf(buffer2, "HTTP/1.1 404 NOT FOUND\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n", extension, (int)longitud.st_size);
                    n = write(newsockfd, buffer2, strlen(buffer2));
                    if (n < 0) {
                        error("\n\t#ERROR 6: Writing from socket. [Linea: 156]");
                    }
                    g = fopen(cono,"r");
                    envio = (char*)malloc (longitud.st_size);			
                    i = fread(envio, longitud.st_size, 1, g);
                    n = write(newsockfd, envio, (int)longitud.st_size);
                    if (n < 0){
                        error("\n\t#ERROR 7: Writing from socket. [Linea: 163]");
                    }
                    fclose(g);
                    free(envio);
                } else{
                    stat(cono, &longitud);
                    sprintf(buffer2, "HTTP/1.1 200 OK\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n", extension, (int)longitud.st_size);
                    n = write(newsockfd, buffer2, strlen(buffer2));
                    if (n < 0){
                        error("\n\t#ERROR 8: Writing from socket. [Linea: 173]");
                    }
                    envio = (char*)malloc (longitud.st_size);
                    //Abre el archivo g de tamaño 'longitud.st_size' de caracter en caracter y lo guarda en envio
                    i = fread(envio, longitud.st_size, 1, g);
                    n = write(newsockfd, envio, (int)longitud.st_size);
                    if (n < 0){
                        error("\n\t#ERROR 9: Writing from socket. [Linea: 180]");
                    }
                    fclose(g);
                    free(envio);
                }
            }
		}
	} else{
		u=strcmp("POST", metodo);

		if (u == 0){
            printf("--> Se produjo una solicitud 'POST'");
			if(strcmp("HTTP/1.1", version) == 0 || strcmp("HTTP/1.0", version) == 0){
				printf("---> Version 'HTTP' esta correcta.\n");
                // config.cgi
                if(strcmp(recurso, "/config.cgi") == 0){					
                    strcpy(buffer2, buffer);
                    tmp = strtok(buffer2, "\r\n");
                    while((tmp=strtok(NULL,"\r\n")) != NULL){	
                        if(strncmp(tmp, "Content-Length: ", strlen("Content-Length: ")) == 0){
                            sscanf(tmp, "Content-Length: %d\r\n", &num_bytes);
                        }
                        if((strncmp(tmp, "username=", strlen("username="))) == 0){
                            tmp = strpbrk(tmp, "=");
                            // Sumo uno a la posicion de memoria para saltar al siguiente caracter
                            tmp++;
                            strcpy(user, strtok(tmp, "&"));							
                        }
                        if((strncmp(tmp, "pass=", strlen("pass="))) == 0){
                            tmp = strpbrk(tmp, "=");
                            // Sumo uno a la posicion de memoria para saltar al siguiente caracter
                            tmp++;
                            strcpy(password, strtok(tmp, "&"));					
                        }
                    }
                    strcpy(cono, DIRECTORIO);
                    strcat(cono, "config.cgi");
                    sprintf(buffer2, "=login=%s&password=%s\r\n", user, password);
                    // Guardar los dados en la Variable de Session do SO
                    // 'QUERY_STRING' definido en el ejercicio
                    setenv("QUERY_STRING", buffer2, 1);
                    config = popen(cono,"r");
                    if(config == NULL){
                        error("\n\t#ERROR 10: Al abrir lo archivo 'config.cgi'. [Linea: 222]");			
                    }
                    // Inicializar generando error en Valgrind
                    buffer[0] = '\0';
                    aux[0]='\0';
                    // Leer lo archivo de configuracion
                    while(fgets(aux, 256, config) != NULL){
                        strcat(buffer, aux);
                    }
                    i = strlen(buffer);
                    if(config == NULL){
                        error("\n\t#ERROR 11: Al leer lo archivo 'config.cgi'. [Linea: 233]");			
                    }	
                    sprintf(buffer2, "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-length: %d\r\n\r\n", i);
                    n = write(newsockfd, buffer2, strlen(buffer2));
                    n = write(newsockfd, buffer, i);
                    
                    if (n < 0){
                        error("\n\t#ERROR 12: Writing from socket. [Linea: 240]");
                    }
                    // Cerando el archivo abierto
                    pclose(config);
                }
                // status.cgi
                if(strcmp(recurso, "/status.cgi") == 0){
                    strcpy(cono, DIRECTORIO);					
                    strcat(cono, "status.cgi");
                    sprintf(buffer2, "MyHttpServer");
                    setenv("SERVER_NAME", buffer2, 1);
                    setenv("REQUEST_METHOD", metodo, 1);
                    h = popen(cono, "r");
                    if(h == NULL){
                        printf("\n\t#ERROR 13: Al abrir lo archivo 'status.cgi'. [Linea: 255]");
                        exit(1);
                    }
                    // Error del valgrind
                    strcpy(buffer3, "");
                    aux[0] = '\0';
                    while((fgets(aux, 256, h)) != NULL){
                        strcat(buffer3, aux);
                    }
                    i=strlen(buffer3);
                    if (h == NULL){
                        error("\n\t#ERROR 14: Al leer lo archivo 'status.cgi'. [Linea: 266]");			
                    }
                    sprintf(buffer2, "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-length: %d\r\n\r\n", i);
                    n=write(newsockfd, buffer2, strlen(buffer2));
                    n=write(newsockfd, buffer3, i);
                    if (n < 0) 
                        error("\n\t#ERROR 15: Al leer lo archivo 'status.cgi'. [Linea: 272]");
                    pclose(h);
                }
                // Informando los dados del Usuario.
                printf("\n\tContent-Length=%d Usuario: %s - Contrasenha: %s\n", num_bytes, user, password);
                if(strcmp(recurso,"/") == 0){
                    strcpy(cono, DIRECTORIO);
                    strcat(cono, PAGINA);
                    g=fopen(cono, "r");
                    if(g == NULL){
                        printf("\n\t#ERROR 16: Abriendo la pagina por defecto. [Linea: 282]\n");
                        exit(1);
                    }
                fclose(g);
                }
            }
		}
    }
	if (n < 0){
		error("\n\t#ERROR 12: Writing from socket. [Linea: 291]");
    }
    // Cerramos el socket nuevo que se ha creado en el otro sentido
    close(newsockfd);
}

// Tienes que capturar lo signal de Ctl + C
// Informaciones en ejercicio - LINEA 355
void handle(int nsignal){
	printf("\n- - - - - - - - - - - - - - - - -\n- -   Apagar el servidor...   - -\n- -   Adios...                - -\n- - - - - - - - - - - - - - - - -\n");
	close(sockfd);
	exit(0);
}

int main(int argc, char *argv[])
{
	FILE    *f;
	int     portno, n, newsockfd;
	struct  sockaddr_in serv_addr, cli_addr;

	f = fopen("config.config", "r");
	if (f == NULL){
		error("\t#ERROR 1: Lo archivo 'config.config' no encontrado.\n");
	}
    // Leer las informaciones del archivo de configuracion
    fscanf(f,"%d\r\n", &PUERTO);
    fscanf(f,"%s\r\n", DIRECTORIO);
    fscanf(f,"%s\r\n", PAGINA);
    fscanf(f,"%s\r\n", USUARIO);
    fscanf(f,"%s\r\n", PASS);
    fclose(f);

	// Crear un socket para manejar conexiones TCP
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		error("\n\t#ERROR 13: Opening socket. [Linea: 326]\n");
    }

	// Preparar una estructura con informacion sobre como vamos a recibir las conexiones TCP
	// En este ejemplo, enlazaremos el socket a la IP local al puerto TCP PORT
	// Esto lo conseguiremos inicializando la estructura a cero mediante.
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = PUERTO;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("\n\t#ERROR 14: ERROR on binding. [Linea: 338]\n");

	// Activaremos una propiedad del socket que permitira que otros sockets puedan reutilizar cualquier puerto al que nos enlacemos.
	// Esto permitira en protocolos como el TCP, poder ejecutar un mismo programa varias veces seguidas y enlazarlo siempre al mismo puerto. De lo contrario habria que esperar a que
    // el puerto quedase disponible (TIME_WAIT en el caso de TCP)
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &serv_addr, sizeof(struct sockaddr_in)) == -1){
 		error("\n\t#ERROR 15: Fallo al reutilizar el puerto. [Linea: 344]\n");
	}

	// Habilitar socket para recibir conexiones
	if(listen(sockfd, MAX_CONNECTION) == -1){
		printf("\n\t#ERROR 16: Fallo en listen. [Linea: 349]\n");
		close(sockfd);
		return -1;
	}

	// Captura la signal
	if(signal(SIGINT, handle) == SIG_ERR){
		error("\n\t#ERROR 17: Fallo al capturar la senal SIGINT. [Linea: 356]\n");
	}

	while(1){
		// Esperar conexion
		n = sizeof(struct sockaddr_in);
		if((newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, (socklen_t *) &n)) == -1){
			printf("\n\t#ERROR 18: En Accept. [Linea: 363]\n");
			continue;
		}
		// Procesar peticion
		process_request(newsockfd);
	}
	close(sockfd);
	return 0; 
}