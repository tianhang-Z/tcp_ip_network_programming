#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#define BUFF_SIZE 30    
void error_handling(char *message);

int main(int argc,char *argv[]){

    int sock;
    char message[BUFF_SIZE]={0};
    int str_len;

    struct sockaddr_in serv_adr,from_adr;

    if(argc!=3){
        printf("usage :%s <ip> <port> \n",argv[0]);
        exit(1);
    }

    sock=socket(PF_INET,SOCK_DGRAM,0);
    if(sock==-1) error_handling("socket() error\n");

    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_adr.sin_port=htons(atoi(argv[2]));

    connect(sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr));
    while(1){
        fputs("input message (Q to quit)\n",stdout);
        fgets(message,BUFF_SIZE,stdin);

        if(!strcmp(message,"q\n")||!strcmp(message,"Q\n"))
            break;
        
        sendto(sock,message,strlen(message),0,
            (struct sockaddr*)&serv_adr,sizeof(serv_adr));
        socklen_t adr_sz;
        adr_sz=sizeof(from_adr);
        str_len=recvfrom(sock,message,BUFF_SIZE,0,
            (struct sockaddr*)&from_adr,&adr_sz);
      ///  message[str_len]=0;
        printf("message from server: %s",message);

    }
    close(sock);
    return 0;
}

void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}