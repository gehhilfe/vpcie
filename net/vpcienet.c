//
// Created by timb on 10.04.18.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "vpcienet.h"


#define CONFIG_DEBUG 1
#if CONFIG_DEBUG

#include <stdio.h>
#include <stdlib.h>

#define PRINTF(__s, ...)  \
do { printf(__s, ## __VA_ARGS__); } while (0)
#define PERROR() printf("[!] %s %d\n", __FUNCTION__, __LINE__)
#else
#define PRINTF(__s, ...)
#define PERROR()
#endif

static int open_tcp_socket(const char *laddr, const char *lport, const char *raddr, const char *rport, int *server_fd,
                           int *client_fd) {
    static const int on = 1;

    int err = -1;
    struct addrinfo ai;
    struct addrinfo *lai = NULL;

    *server_fd = -1;
    *client_fd = -1;

    /* local addressing info */
    memset(&ai, 0, sizeof(ai));
    ai.ai_flags = AI_CANONNAME | AI_ADDRCONFIG;
    ai.ai_family = PF_INET;
    ai.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(laddr, lport, &ai, &lai)) {
        PERROR();
        goto on_error;
    }

    *server_fd = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
    if (*server_fd == -1) {
        PERROR();
        goto on_error;
    }
    setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof(on));

    if (bind(*server_fd, lai->ai_addr, lai->ai_addrlen) < 0) {
        PERROR();
        goto on_error;
    }
    if (listen(*server_fd, 1)) {
        PERROR();
        goto on_error;
    }

    *client_fd = accept(*server_fd, NULL, NULL);
    if (*client_fd < 0) {
        PERROR();
        goto on_error;
    }

    /* success */
    err = 0;

    on_error:
    if (lai != NULL) freeaddrinfo(lai);
    if (err == 0) return 0;

    if (*server_fd != -1) {
        shutdown(*server_fd, SHUT_RDWR);
        close(*server_fd);
    }

    if (*client_fd != -1) {
        shutdown(*client_fd, SHUT_RDWR);
        close(*client_fd);
    }

    return -1;
}

int pcie_net_init(pcie_net_t *net, const char *laddr, const char *lport, const char *raddr, const char *rport) {
    /* important, use by event pump */
    net->task_fn = NULL;

    net->ev_fd = -1;

    if (open_tcp_socket(laddr, lport, raddr, rport, &net->server_fd, &net->fd))
        return -1;

    return 0;
}

int pcie_net_fini(pcie_net_t *net) {
    shutdown(net->server_fd, SHUT_RDWR);
    close(net->server_fd);
    shutdown(net->fd, SHUT_RDWR);
    close(net->fd);
    return 0;
}

ssize_t pcie_net_recv_buf(pcie_net_t *net, void *buf, size_t max_size) {
    pcie_net_header_t h;
    ssize_t n;
    size_t rem_size;

    /* headerModule always comes first */
    if (max_size < sizeof(h)) {
        PERROR();
        return -1;
    }
    n = recv(net->fd, (void *) &h, sizeof(h), MSG_WAITALL);
    if (n != sizeof(h)) {
        PERROR();
        return -1;
    }

    if (h.size > max_size) {
        PERROR();
        return -1;
    }

    rem_size = h.size - sizeof(h);
    n = recv(net->fd, (uint8_t *) buf + sizeof(h), rem_size, MSG_WAITALL);
    if (n != (ssize_t) rem_size) {
        PERROR();
        return -1;
    }
    return (ssize_t) h.size;
}

int pcie_net_add_task(pcie_net_t *net, const struct timeval *tm, pcie_net_taskfn_t fn, void *data) {
    net->task_tm = *tm;
    net->task_fn = fn;
    net->task_data = data;
    return 0;
}

int pcie_net_add_ev(pcie_net_t *net, int fd, pcie_net_evfn_t fn, void *data) {
    net->ev_fn = fn;
    net->ev_fd = fd;
    net->ev_data = data;
    return 0;
}

int pcie_net_loop(pcie_net_t *net, pcie_net_recvfn_t on_msg_recv, void *opak) {
    pcie_net_msg_t *msg;
    pcie_net_reply_t reply;
    struct timeval *tm;
    fd_set rfds;
    int err;
    int max_fd;
    unsigned int must_stop;

    if ((msg = malloc(PCIE_NET_MSG_MAX_SIZE)) == NULL) return -1;

    while (1) {
        tm = NULL;
        if (net->task_fn != NULL) tm = &net->task_tm;

        FD_ZERO(&rfds);

        FD_SET(net->fd, &rfds);
        max_fd = net->fd;

        if (net->ev_fd != -1) {
            FD_SET(net->ev_fd, &rfds);
            if (net->fd < net->ev_fd) max_fd = net->ev_fd;
        }

        err = select(max_fd + 1, &rfds, NULL, NULL, tm);
        if (err < 0) {
            PERROR();
            break;
        } else if (err == 0) {
            /* timeout elapsed, task to execute */
            pcie_net_taskfn_t fn = net->task_fn;
            /* set to NULL before executing, in case of reloading */
            net->task_fn = NULL;
            fn(net->task_data);
        } else /* something to read, either network or event */
        {
            must_stop = 0;

            if (FD_ISSET(net->fd, &rfds)) {
                err = pcie_net_recv_msg(net, msg);
                if (err == -1) {
                    PERROR();
                    must_stop = 1;
                } else if (err != 1) /* not icmp_unreachable case */
                {
                    /* handle new message and reply if asked to */
                    if (on_msg_recv(msg, &reply, opak) != 0) {
                        if (pcie_net_send_reply(net, &reply) == -1) {
                            PERROR();
                            must_stop = 1;
                        }
                    }
                }
            } /* socket fd was set */

            if (FD_ISSET(net->ev_fd, &rfds)) {
                unsigned int buf[32];
                ssize_t n;
                ssize_t i;

                if ((n = read(net->ev_fd, buf, sizeof(buf))) >= 0) {
                    for (i = 0; i < (n / sizeof(unsigned int)); ++i) {
                        /* buf[i] used as a key */
                        if (net->ev_fn(buf[i], net->ev_data) != 0) must_stop = 1;
                    }
                }
            } /* event fd was set */

            if (must_stop) break;

        } /* something to read */
    } /* while (1) */

    free(msg);

    return 0;
}