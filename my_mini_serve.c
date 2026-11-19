#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct s_client// <-------------- added to the given main
{
	int id;
	char msg[1000000];
}	t_client;

t_client clients[1024];// <-------------- added to the given main
int max_fd = 0, next_id = 0;// <-------------- added to the given main
char readbuff[1000000], writebuff[1000000];
fd_set active_fds, read_fds, write_fds;// <-------------- added to the given main

void exiterror()// <-------------- added to the given main
{
	write (2, "Fatal error\n", 12);
	exit (1);
}

void send_msg(int sender_fd)// <-------------- added to the given main
{
	for (int fd = 0; fd <= max_fd; fd++)
		if (FD_ISSET(fd, &write_fds) && fd != sender_fd)
			send (fd, writebuff, strlen (writebuff), 0);
}

int main(int ac, char **av)// <-------------- added to the given main ---> int ac, char **av
{
	if (ac != 2)// <-------------- added to the given main
	{
		write (2, "Wrong number of arguments\n", 26);
		exit (1);
	}
	int sockfd;// <-------------- deleted connfd and len
	struct sockaddr_in servaddr; // <-------------- deleted cli

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
		exiterror();// <-------------- added to the given main

	max_fd = sockfd;// <-------------- added to the given main
	FD_ZERO(&active_fds);// <-------------- added to the given main
	FD_SET(sockfd, &active_fds);// <-------------- added to the given main

	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1])); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		exiterror();// <-------------- added to the given main
	if (listen(sockfd, 10) != 0)
		exiterror();// <-------------- added to the given main

	while (1)// <-------------- added to the given main (all the code inside the while loop)
	{
		read_fds = write_fds = active_fds;
        if(select(max_fd + 1, &read_fds, &write_fds, NULL, NULL) < 0)
            continue;
        for(int fd = 0; fd <= max_fd; fd++)
        {
            if(FD_ISSET(fd, &read_fds))
            {
                if (fd == sockfd)
                {
                    int cl_sock = accept(sockfd, 0, 0);
                    if(cl_sock < 0)
                        continue;
                    if(cl_sock > max_fd)
                        max_fd = cl_sock;
                    clients[cl_sock].id = next_id++;
                    sprintf(writebuff, "server: client %d just arrived\n", clients[cl_sock].id);
                    send_msg(cl_sock);
                    bzero(clients[cl_sock].msg, sizeof(clients[cl_sock].msg));
                    FD_SET(cl_sock, &active_fds);
                    break;		
                }
                else
                {
                    int read_bytes = recv(fd, readbuff, sizeof(readbuff) - 1, 0);
                    if(read_bytes <= 0)
                    {
                        sprintf(writebuff, "server: client %d just left\n", clients[fd].id);
                        send_msg(fd);
                        FD_CLR(fd, &active_fds);
                        close(fd);
                        break;
                    }
                    else
                    {
                        readbuff[read_bytes] = '\0';
                        for( int i = 0, j = strlen(clients[fd].msg); i < read_bytes && (size_t)j < sizeof(clients[fd].msg) - 1; i++, j++)
                        {
                            clients[fd].msg[j] = readbuff[i];
                            if(clients[fd].msg[j] == '\n')
                            {
                                clients[fd].msg[j] = '\0';
                                sprintf(writebuff, "client %d: %.999980s\n", clients[fd].id, clients[fd].msg);
                                send_msg(fd);
                                bzero(clients[fd].msg, sizeof(clients[fd].msg));
                                j = -1;
                            }
                        }
                    }
                }
            }
        }
	}
}