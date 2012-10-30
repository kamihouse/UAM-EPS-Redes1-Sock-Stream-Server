#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void main(){
int PUERTO;
char user[128],password[128],PAGINA[128],USUARIO[128],PASS[128],cono[256],DIRECTORIO[256],buffer2[512],*tmp,*aux;
FILE *f;
		
	f=fopen("/home/alumnos/e237391/Escritorio/.config","r");
	if (f==NULL){
		printf("Archivo .config no encontrado\n");
		return;
	}
	
		fscanf(f,"%d\r\n",&PUERTO);
		fscanf(f,"%s\r\n",DIRECTORIO);
		fscanf(f,"%s\r\n",PAGINA);
		fscanf(f,"%s\r\n",USUARIO);
		fscanf(f,"%s\r\n",PASS);
	fclose(f);

strcpy(buffer2,getenv("QUERY_STRING"));		
		
tmp=strtok(buffer2,"\r\n");

	if ((strncmp(tmp,"=login=",strlen("=login=")))==0){
		sscanf(tmp,"=login=%s",user);
		strcpy(user,strtok(user,"&"));
		tmp=strtok(NULL,"&");
	}

	if ((strncmp(tmp,"password=",strlen("password=")))==0){
		tmp=strpbrk(tmp,"=");
		tmp++;	
		strcpy(password,tmp);
	}



if (strcmp(USUARIO,user)==0&&strcmp(PASS,password)==0){
	printf("<html><body><p><p>LOGIN CORRECTO<p><p></html></body>");
}
else{
	printf("<html><body><p><p>LOGIN INCORRECTO<p><p></html></body>");
}
	

}
