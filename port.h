#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>

int getAvailablePort(int *bogus_socket, int *port_no){
    struct sockaddr_in bogus_server_address;
    int new_port_no;
    new_port_no = *port_no - 1  ;

    bzero((char *) &bogus_server_address, sizeof(bogus_server_address));
    bogus_server_address.sin_family = AF_INET;
    bogus_server_address.sin_addr.s_addr = INADDR_ANY;
    printf("Checking available port...\n\n");
    do {
        ++new_port_no;
        bogus_server_address.sin_port = htons(new_port_no);
    }while (bind(*bogus_socket, (struct sockaddr *) & bogus_server_address, sizeof(bogus_server_address)) < 0);
 
    printf("Found port available at %d\n", new_port_no);

    *port_no =  new_port_no;
}