#ifndef ASSIGNMENT4_ST_PROACTOR_HPP
#define ASSIGNMENT4_ST_PROACTOR_HPP

#include <iostream>
#include <vector>
#include <pthread.h>
#include <poll.h>
#include <unistd.h>
#include <functional>
#include <algorithm>
#include <cstdint>
using namespace std;

typedef void (*handler_t)(int);

typedef struct Proactor {
    std::vector<std::pair<int, handler_t>> fd_handlers;
    std::vector<pollfd> poll_fds;
    bool active;
    pthread_t proactor_thread;
} Proactor, *pProactor;


int cancelProactor(void* thisPtr);
int runProactor(void* thisPtr);
int removeHandler(void* thisPtr, int fd);
int addFD2Proactor(void* thisPtr, int fd, handler_t handler);
void* createProactor();
void* proactorLoop(void* arg);
void* handlerWrapper(void* arg);
#endif //ASSIGNMENT4_ST_REACTOR_HPP