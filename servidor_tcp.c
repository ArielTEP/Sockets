/*****
*UPIITA-IPN México
*Autor de este programa: Espindola Pizano Ariel Tonatiuh
*CopyRight© Derechos Reservados
*/


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>//inet_ntoa
#include <netdb.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include <math.h>
#define BUFF_SIZE 512
#define N 3

FILE *file[N];
char **fileName;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct Copy
{
	unsigned int idPackage;
    unsigned int total_packages;
	unsigned int size_package;
	unsigned int size_file;
	int reads;
	int flag;
	unsigned char buffer[BUFF_SIZE];	
}copy_f;

FILE *createFile(const char *nameFile,char *format){
	FILE *file = fopen(nameFile,format);
	if (file != NULL){return file;}
	else{printf("Error al crear/abrir archivo.\n"); exit(0);}
}

unsigned int remainingBytes(unsigned int idPackage,int buffer_size, unsigned int size_file){
  return (size_file - (idPackage*buffer_size));
}

void sendFile(int newsockfd, copy_f *data, int indice){

  unsigned int pos = 0;
  unsigned int restante;

  do{       

        read(newsockfd,data,sizeof(copy_f));
        printf("Paquete enviando: %u\n",data->size_package );

          if(data->flag == 1)
          {
             data->flag = 1;
             write(newsockfd,data,sizeof(copy_f));
          }
          if(data->flag == 0)
          {
             pthread_mutex_lock(&mutex);
             fseek(file[indice-1],pos,SEEK_SET);
             data->idPackage++;
             
             restante = remainingBytes(data->idPackage-1,BUFF_SIZE,data->size_file);
             data->size_package = ( restante < BUFF_SIZE )?restante : BUFF_SIZE;  

             data->reads = fread(data->buffer,data->size_package,1,file[indice-1]);
             if(data->idPackage <= data->total_packages){
             printf("Pos: %u, %u / %u Paquete: %u bytes\n",
                    pos,data->idPackage, data->total_packages,data->size_package);}

             pos = ftell(file[indice-1]);
             pthread_mutex_unlock(&mutex);
             data->flag = 1;
             write(newsockfd,data,sizeof(copy_f));
          }
        }while(!feof(file[indice-1]));

         printf(" [100--] \n");


}

void initNameFile(char ***fileName){

	(*fileName)[0] = "MATLAB_2014.zip";
	(*fileName)[1] = "archivo2.pdf";
	(*fileName)[2] = "archivo3.png";

	printf("Nombre1: %s\n",(*fileName)[0]);
	printf("Nombre2: %s\n",(*fileName)[1]);
  printf("Nombre3: %s\n",(*fileName)[2]);
}


void error(const char *msg){
    perror(msg);
    exit(1);
}

void *codigo_hilo_cliente(void *_newsockfd){
  int newsockfd = *(int *)_newsockfd;
	copy_f *data;
	data = (copy_f*)malloc(sizeof(copy_f));

    char buffer_char[256];
    int indice;
    

	bzero(buffer_char,256);
	read(newsockfd,&indice,sizeof(indice));
	printf("indice %d\n",indice);
	if(indice == 0){ printf("Dato 0 no valido. \n"); exit(0);}

	   write(newsockfd,fileName[indice-1],sizeof(buffer_char));

	   pthread_mutex_lock(&mutex);
     fseek(file[indice-1],0,SEEK_END);
     data->size_file = ftell(file[indice-1]);

       if(data->size_file == 0)
        {printf("Archivo vacío, no se pudo leer\n"); exit(-1);}

     data->total_packages = (int)ceil((float)data->size_file / (float)BUFF_SIZE);
     printf("TOTAL: %u\n",data->total_packages);
     data->idPackage = 0;
     data->size_package = 0;

     fseek(file[indice-1],0,SEEK_SET);
     pthread_mutex_unlock(&mutex);

     data->flag = 0;

     write(newsockfd,data,sizeof(copy_f));
     sendFile(newsockfd,data,indice);   

    close(newsockfd);
    pthread_exit(&newsockfd);
}

int main(int argc, char *argv[]){
  int sockfd;
  int portno;  
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  fileName = (char **)malloc(sizeof(char *));
  initNameFile(&fileName);
	
  if (argc < 2) {
		fprintf(stderr,"ERROR, no se ha proveido puerto\n");
	    exit(1);
	}
  
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
    if (sockfd < 0) 
    error("ERROR al abrir socket");
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
      
  portno = atoi(argv[1]);
  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
   	error("ERROR en enlazar");

   listen(sockfd,5);

   file[0] = createFile((const char*)fileName[0],"rb");
   file[1] = createFile((const char*)fileName[1],"rb");
   file[2] = createFile((const char*)fileName[2],"rb");

  while(1){
    int newsockfd;
  	printf("Esperando a un cliente\n");
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    
  	printf("Cliente aceptado\n");
  	if (newsockfd < 0)
     	error("ERROR en aceptar conecciones");
    pthread_t hilo;
    pthread_create(&hilo,NULL,codigo_hilo_cliente,&newsockfd);
  }
	
  close(sockfd);
 	return 0; 
}