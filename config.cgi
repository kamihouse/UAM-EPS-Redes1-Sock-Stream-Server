#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <signal.h>

// Problema com nombre
void error1(const char *msg){
    perror(msg);
    exit(1);
}

int login(char user[128], char pass[128],char DIRECTORIO[128],int newsockfd){

struct stat longitud;
int PUERTO,n,i;
char PAGINA[128],USUARIO[128],PASS[128],cono[256],buffer2[512],*envio;
FILE *f,*g;
	f=fopen(".config","r");
	if (f==NULL){
		//error1("Archivo .config no encontrado\n");
	}
	
		fscanf(f,"%d\r\n",&PUERTO);
		fscanf(f,"%s\r\n",DIRECTORIO);
		fscanf(f,"%s\r\n",PAGINA);
		fscanf(f,"%s\r\n",USUARIO);
		fscanf(f,"%s\r\n",PASS);
	fclose(f);

if (strcmp(USUARIO,user)==0&&strcmp(PASS,pass)==0){
	printf("\n\n LOGIN CORRECTO \n\n");
	/*strcat(cono,PAGINA);
	g=fopen(cono,"r");
	if (g==NULL){
		printf("Error abriendo la pagina por defecto\n");
		exit(1);
	}
	stat(cono,&longitud);
	sprintf(buffer2,"HTTP/1.1 200 OK\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n",extension,(int)longitud.st_size);
	n=write(newsockfd,buffer2,strlen(buffer2));
	if (n < 0) 
		//error1("ERROR writing from socket\n");
	envio = (char*)malloc ( longitud.st_size );
	i=fread(envio,longitud.st_size,1,g);//Abre el archivo g de tamaño longitud.st_size de caracter en caracter(1) y lo guarda en envio
	
	n=write(newsockfd,envio,(int)longitud.st_size);
	if (n < 0) 
		//error1("ERROR writing from socket\n");
*/}
else{
	printf("\n\nLOGIN INCORRECTO\n\n");
	strcpy(PAGINA,"/incorrecto.html");	
	strcat(cono,PAGINA);
	stat(cono,&longitud);
	sprintf(buffer2,"HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-length: %d\r\n\r\n",(int)longitud.st_size);
	n=write(newsockfd,buffer2,strlen(buffer2));
	if (n < 0) 
		error1("ERROR writing from socket\n");
	envio = (char*)malloc ( longitud.st_size );
	i=fread(envio,longitud.st_size,1,g);//Abre el archivo g de tamaño longitud.st_size de caracter en caracter(1) y lo guarda en envio
	
	n=write(newsockfd,envio,(int)longitud.st_size);
	if (n < 0) 
		//error1("ERROR writing from socket\n");
	free(envio);
}
	
	
	return i;
}
