#include "st_proactor.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <vector>
#include <dlfcn.h>
#include <iostream>
using namespace std;

#define PORT "9034"

void handle_listen(int listener);
void handle_clients(int fd);
void* sendMassageToAll(void* arg);



static pProactor ourProactor;

// Get sockaddr
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Return a listening socket
int get_listener_socket(void)
{
    int listener;     // Listening socket descriptor
    int yes=1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // Reuse address
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        return -1;
    }

    // Listen
    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}



int main(void)
{

    // Create reactor
    ourProactor = static_cast<pProactor>(createProactor());
    
    // Listening socket fd
    int listener; 

    // Set up and get a listening socket
    listener = get_listener_socket();
    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    cout << "Chat is open!" << endl;

    // Run the proactor
    runProactor(ourProactor);

    // Add the listening socket fd to the proactor
    addFD2Proactor(ourProactor, listener, handle_listen);

    // Wait for the proactor to finish its work
    pthread_join(ourProactor->proactor_thread, nullptr);

    return 0;

}

void handle_listen(int listener){

    // Client address
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;

    // Prepering a new fd for the client socket
    int newfd; 
    char remoteIP[INET6_ADDRSTRLEN];     
    addrlen = sizeof(remoteaddr);

    // Accept for the client socket fd
    newfd = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);
    if (newfd == -1) {
    perror("accept");
    } else {
        // If succeed, add the socket fd to the fds data structure using the reactor
        addFD2Proactor(ourProactor,newfd,handle_clients);
        printf("New client has joined the chat from %s on ""socket %d\n",
        inet_ntop(remoteaddr.ss_family,
        get_in_addr((struct sockaddr *)&remoteaddr),remoteIP, INET6_ADDRSTRLEN),newfd);
    }
}

void handle_clients(int fd)
{
    char buf[256];
    int nbytes = recv(fd, buf, sizeof(buf), 0);
                    
    int sender_fd = fd;

    if (nbytes <= 0) {
        // Got error or connection closed by client
        if (nbytes == 0) {
            // Connection closed
            printf("Server: socket %d hung up\n", sender_fd);
        } else {
            perror("recv");
        }

    } else {

        pthread_t message_thread;
        std::tuple<char*, int, int> messageTuple;
        messageTuple = std::make_tuple(buf, nbytes, sender_fd);
        pthread_create(&message_thread, nullptr, sendMassageToAll, &messageTuple);
    }
}

void* sendMassageToAll(void* arg)
 {

    std::tuple<char*, int, int>* messageTuple = reinterpret_cast<std::tuple<char*, int, int>*>(arg);

    char* buf = std::get<0>(*messageTuple);
    int nbytes = std::get<1>(*messageTuple);
    int sender_fd = std::get<2>(*messageTuple);

    // We got some data from a client
    for (size_t j = 0; j < ourProactor->fd_handlers.size(); j++) {
        // Send to everyone
        int dest_fd = ourProactor->fd_handlers[j].first;

        // Except the listener and ourselves
        int listener = ourProactor->fd_handlers[0].first;
        if (dest_fd != listener && dest_fd != sender_fd) {
            if (send(dest_fd, buf, nbytes, 0) == -1) {
                perror("send");
            }
        }
    }

    pthread_t threadId = pthread_self();
    std::cout << "Thread ID: " << threadId << std::endl;
    pthread_exit(nullptr);

    return nullptr;
}