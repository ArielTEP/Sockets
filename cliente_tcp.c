/*
*
*UPIITA-IPN México
*Autor de este programa: Espindola Pizano Ariel Tonatiuh
*CopyRight© Todos los derechos reservados.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFF_SIZE 512

FILE *file;

typedef struct Copy
{
	unsigned int idPackage_res;
  unsigned int total_packages_res;
	unsigned int size_package_res;
	unsigned int size_file_res;
	int reads_res;
	int flag_res;
	unsigned char buffer_res[BUFF_SIZE];	
}paste_f;

FILE *createFile(const char *nameFile,char *format){
	FILE *file = fopen(nameFile,format);
	if (file != NULL){return file;}
	else{printf("Error al crear/abrir archivo.\n"); exit(0);}
}

void error(const char *msg){
  perror(msg);
  exit(0);
}

void receiveFile(int sockfd, paste_f* data){

  do{

      if(data->flag_res == 0)
      {
         data->flag_res = 0;
         write(sockfd,data,sizeof(paste_f));
      }  

      if(data->flag_res == 1)
      {
         fwrite(data->buffer_res , data->size_package_res, data->reads_res,file);
         printf("Recibido: %u/%u , Paquete: %u bytes\n",
         data->idPackage_res, data->total_packages_res,data->size_package_res);
         data->flag_res = 0;
         write(sockfd,data,sizeof(paste_f));
      }  
      
         read(sockfd,data,sizeof(paste_f));

       } while(data->idPackage_res <= data->total_packages_res);

}

int main(int argc, char const *argv[])
{
  int sockfd;
  int portno;   
  struct sockaddr_in serv_addr;
  struct hostent *server;
  int n;
  char buffer_res_char[256];

  if (argc < 3) {
    fprintf(stderr,"Uso %s servidor puerto \n", argv[0]);
    exit(0);
  }
  
  portno = atoi(argv[2]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR al abrir socket");
  
  server = gethostbyname(argv[1]);
  if (server == NULL) {
    fprintf(stderr,"ERROR, no existe host\n");
    exit(0);
  }
  
  bzero((char *) &serv_addr, sizeof(serv_addr)); 
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);  
 
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
  error("ERROR al conectar");  
	    
//+++++++++++++++++++++++++++++++++++++++++++++++++//

	paste_f *data;
	data=(paste_f*)malloc(sizeof(paste_f));
  int buff_int;
  int at;
  char texto[256];

    printf("********************************************\n");
    printf("*    Bienvenido al Servidor de Archivos    *\n");
    printf("********************************************\n");
    printf("Seleccione un archivo, sock %d : \n",sockfd);
    printf("1. archivo1.jpg\n");
    printf("2. archivo2.pdf\n");
    printf("3. archivo3.png\n");
    printf("0. Salir.\n\n");
    printf("--------->");
    scanf("%d",&buff_int);

    printf("Ingrese un nuevo nombre:");
    bzero(texto,256);
    scanf("%s",texto);
    printf("OK!\n");

	write(sockfd,&buff_int,sizeof(int));
	
	read(sockfd,buffer_res_char,sizeof(buffer_res_char));
	printf("buffer_res_char: %s\n",buffer_res_char);
  at = strlen(buffer_res_char);
  char extension[5];
  bzero(extension,5);
  int i = at-4;
  int j=0;
  for(;i<at;i++,j++)
    extension[j] = buffer_res_char[i];

  printf("Extension: %s\n",extension);
  strcat(texto,extension);
	
	file=createFile(texto,"wb+");

	read(sockfd,data,sizeof(paste_f));
  receiveFile(sockfd,data);
  printf("[100--] \nDescarga completada.\n");
  fclose(file);

  close(sockfd);
	return 0;
}
