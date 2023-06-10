#include "st_proactor.hpp"

void* createProactor() {
    pProactor ourProactor = static_cast<pProactor>(malloc(sizeof(Proactor)));
    ourProactor->active = false;
    ourProactor->fd_handlers.reserve(5);
    ourProactor->poll_fds.reserve(5);
    return ourProactor;
}


int addFD2Proactor(void* thisPtr, int fd, handler_t handler) {
    Proactor* proactor = static_cast<Proactor*>(thisPtr);
    proactor->fd_handlers.push_back({fd, handler});
    proactor->poll_fds.push_back({fd, POLLIN, 0});
    return 0;
}

int removeHandler(void* thisPtr, int fd) {
    Proactor* proactor = static_cast<Proactor*>(thisPtr);
    proactor->fd_handlers.erase(
        std::remove_if(
            proactor->fd_handlers.begin(),
            proactor->fd_handlers.end(),
            [fd](const std::pair<int, handler_t>& fd_handler) {
                return fd_handler.first == fd;
            }
        ),
        proactor->fd_handlers.end()
    );
    proactor->poll_fds.erase(
        std::remove_if(
            proactor->poll_fds.begin(),
            proactor->poll_fds.end(),
            [fd](const pollfd& poll_fd) {
                return poll_fd.fd == fd;
            }
        ),
        proactor->poll_fds.end()
    );
    return 0;
}

void* proactorLoop(void* arg) {
    Proactor* proactor = static_cast<Proactor*>(arg);

    while (proactor->active) {
        int num_ready_fds = poll(proactor->poll_fds.data(), proactor->poll_fds.size(), -1);
        if (num_ready_fds < 0) {
            std::cerr << "Error in poll\n";
            break;
        }

        for (const auto& poll_fd : proactor->poll_fds) {
            if (poll_fd.revents & POLLIN) {
                int fd = poll_fd.fd;
                auto it = std::find_if(
                    proactor->fd_handlers.begin(),
                    proactor->fd_handlers.end(),
                    [fd](const std::pair<int, handler_t>& fd_handler) {
                        return fd_handler.first == fd;
                    }
                );
                if (it != proactor->fd_handlers.end()) {
                     it->second(fd);   
                }
            }
        }
    }

    pthread_exit(nullptr);
}


int runProactor(void* thisPtr) {
    Proactor* proactor = static_cast<Proactor*>(thisPtr);
    if (!proactor->active) {
        proactor->active = true;
        pthread_create(&proactor->proactor_thread, nullptr, &proactorLoop, thisPtr);
        return 0;
    }
    return 1;
}

int cancelProactor(void* thisPtr) {
    Proactor* proactor = static_cast<Proactor*>(thisPtr);
    proactor->active = false;
    pthread_cancel(proactor->proactor_thread);
    return 0;
}