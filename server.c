#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "tcp_lib.h"

#define SRV_PORT 8096

char BUFF[BUFSIZ];

void catch_child(int ret)
{
    ret =0;
    while((ret = waitpid(0,NULL,WNOHANG) > 0)){
        ;
    }
}


int main()
{
    pid_t pid ;
    int lfd , cfd,opt = 1;
    struct sockaddr_in srv_addr,clt_addr;
    socklen_t srv_len = sizeof(srv_addr);
    socklen_t clt_len = sizeof(clt_addr);
    bzero(&srv_addr,srv_len);
    bzero(&clt_addr,clt_len);
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(SRV_PORT);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    lfd = Socket(AF_INET,SOCK_STREAM,0);
        
    setsockopt(lfd,SOL_SOCKET,SO_REUSEPORT,(void *)&opt,sizeof(opt));
    
    Bind(lfd,(struct sockaddr *)&srv_addr,srv_len);

    Listen(lfd,128);
    printf("Accepting connections ...\n");
    while(1){
        cfd = Accept(lfd,(struct sockaddr *)&clt_addr,&clt_len);
        printf("-------------------------%d\n", cfd);
        pid = fork();
        if(pid < 0){
            perror("fork error");
            exit(-1);
        }else if(pid == 0){
            close(lfd);
            break;
        }else{
            struct sigaction act;
            act.sa_handler = catch_child;
            sigemptyset(&act.sa_mask); 
            act.sa_flags = 0;
            int ret = sigaction(SIGCHLD,&act,NULL);
            if(ret == -1){
                perror("sigaction error");
                exit(-1);
            }
            close(cfd);
            //continue;
        }
    }
    // 子进程
    while(1){
        int read_len = Read(cfd,BUFF,sizeof(BUFF));
        if(read_len  == 0){
            printf("the other side has been closed.\n");
            break;
        }
        IP_print(AF_INET,&clt_addr);
        for(int i=0;i<read_len;i++){
            BUFF[i] = toupper(BUFF[i]);
        }
        write(STDOUT_FILENO,BUFF,read_len);
        Write(cfd,BUFF,read_len);
    }
    close(cfd);
    return 0;
}

