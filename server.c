#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/select.h>

void main()
{
    int sockfd,newsockfd,portno,n;
    char ibuffer[255],obuffer[255];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int f;
    char names[50][25];
    bzero(names,sizeof(names));
    sockfd =socket(AF_INET,SOCK_STREAM,0);//Creating the Socket with IPv4 and TCP
    if (sockfd<0)//Socket Creation Failure
    {
        perror("Socket Error");
        exit(1);     
    }
    bzero(&server_addr,sizeof(server_addr));// Clear the server address variable
    portno =3000;

    server_addr.sin_family=AF_INET;//To indicate IPv4
    server_addr.sin_addr.s_addr=INADDR_ANY;// To indicate any IP can connect
    server_addr.sin_port=htons(portno);//Set the port number of the destination

    //Bind the socket to the destination IP and port number
    if(bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0)
    {
        perror("Bind Failed");
        exit(1);
    }
    fd_set current_sockets,ready_sockets; 
    char message[500],misc[255];
    FD_ZERO(&current_sockets);
    FD_SET(sockfd,&current_sockets);
    listen(sockfd,50);

    while(1)
    {   
        ready_sockets=current_sockets;

        if(select(FD_SETSIZE+1,&ready_sockets,NULL,NULL,NULL)<0)
        {
            perror("Select Error\n");
            exit(1);
        }
        for (int i = 0; i < FD_SETSIZE; i++)
        {  // printf("FOR\n");
            if(FD_ISSET(i,&ready_sockets))
            {   
                if(i==sockfd)
                {   
                    client_len = sizeof(client_addr);
                    newsockfd = accept(sockfd,(struct sockaddr *)&client_addr,&client_len);//Accept the first connection from the queue of requested connections.
    
                    if(newsockfd<0)//Accept connection failed
                    {
                        perror("Error Accepting");
                        exit(1);
                    }
                    FD_SET(newsockfd,&current_sockets);
                    read(newsockfd,names[newsockfd],sizeof(names[newsockfd]));
                    strcpy(misc,"Online Users:\n");
                    f=0;
                    for (int k = 0; k < FD_SETSIZE; k++)
                    {
                        if(FD_ISSET(k,&current_sockets)&&k!=newsockfd&&k!=sockfd)
                        {
                            f=1;
                            strcat(misc,names[k]);
                            strcat(misc,"\n");
                        }
                    }
                    if(f==0)
                    strcpy(misc,"There's no one here  :(\n");
                    n=write(newsockfd,misc,sizeof(misc));
                    fprintf(stdout,"%s has joined the chat.\n",names[newsockfd]);

                    for (int j = 0; j < FD_SETSIZE; j++)
                    {
                        if(FD_ISSET(j,&current_sockets))
                        {
                            if(j!=sockfd&&j!=newsockfd)
                            {
                                strcpy(misc,names[newsockfd]);
                                strcat(misc," has joined the chat.\n");
                                
                                 n=write(j,misc,sizeof(misc));//Write to the socket
                                if(n<0)
                                {
                                    perror("Error Writing");
                                    exit(1);
                                }
                            }
                        }
                    }
                }
                else{
                    bzero(ibuffer,255);
                    n = read(i,ibuffer,sizeof(ibuffer));//Read the message from the socket
                    if(n<0)//Error reading
                    {
                        perror("Error Reading");
                        exit(1);
                    }

                   
                    for (int j = 0; j < FD_SETSIZE; j++)
                    {
                        if(FD_ISSET(j,&current_sockets))
                        {
                            if(j!=sockfd&&j!=i&&(strncmp("tata",ibuffer,4)!=0))
                            {
                                
                                strcpy(message,names[i]);
                                strcat(message," : ");
                                strcat(message,ibuffer);
                                 n=write(j,message,sizeof(message));//Write to the socket
                                if(n<0)
                                {
                                    perror("Error Writing");
                                    exit(1);
                                }
   
                            }
                        }
                    }
                    if(strncmp("tata",ibuffer,4)==0)
                    {
                        //printf("%s left the Chat\n",names[i]);
                        FD_CLR(i,&current_sockets);
                        bzero(misc,sizeof(misc));
                        strcpy(misc,names[i]);
                        strcat(misc," : ");
                        strcat(misc,ibuffer);
                        strcat(misc,"\n\n");
                        strcat(misc,names[i]);
                        strcat(misc," left the chat.\n");
                        printf("%s has left the chat.\n",names[i]);
                        for (int j = 0; j < FD_SETSIZE; j++)
                    {
                        if(FD_ISSET(j,&current_sockets))
                        {
                            if(j!=sockfd)
                            {   
                                
                                
                                 n=write(j,misc,sizeof(misc));//Write to the socket
                                if(n<0)
                                {
                                    perror("Error Writing");
                                    exit(1);
                                }
                            }
                        }
                    }
                    }
                    bzero(ibuffer,sizeof(ibuffer));
                }
            }
        }
        

    }
    close(newsockfd);//Close the session socket
    close(sockfd);//Close the socket
    
}