//
// Created by timb on 10.04.18.
//

#include <vpi_user.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include "../net/vpcienet.h"

#define VPIERROR() vpi_printf("[!] %s %d\n", __FUNCTION__, __LINE__)

struct {
    vpiHandle op;
    vpiHandle bar;
    vpiHandle width;
    vpiHandle addr;
    vpiHandle size;

    vpiHandle word_data;
    vpiHandle new_msg;
    bool isSet;
} headerModule;

struct {
    vpiHandle running;
    vpiHandle connected;
    bool isSet;
} statusModule;

struct {
    volatile bool running;
} vpcieStatus;

void setIntValue(vpiHandle handle, int value) {
    struct t_vpi_value argval;

    argval.format = vpiIntVal;
    argval.value.integer = value;
    vpi_put_value(handle, &argval, NULL, vpiNoDelay);
}

void setInt64Value(vpiHandle handle, uint64_t value) {
    struct t_vpi_value argval;

    char *buf = calloc(256, sizeof(char));

    sprintf(buf, "%lx", value);

    argval.format = vpiHexStrVal;
    argval.value.str = buf;
    vpi_put_value(handle, &argval, NULL, vpiNoDelay);

    free(buf);
}

int getIntValue(vpiHandle handle) {
    struct t_vpi_value argval;
    argval.format = vpiIntVal;

    vpi_get_value(handle, &argval);
    return argval.value.integer;
}

void setBooleanValue(vpiHandle handle, bool value) {
    struct t_vpi_value argval;

    argval.format = vpiBinStrVal;
    argval.value.str = (value) ? "1" : "0";
    vpi_put_value(handle, &argval, NULL, vpiNoDelay);
}

vpiHandle obtainSubHandle(char *name, vpiHandle parent) {
    vpiHandle handle = vpi_handle_by_name(name, parent);
    if (handle == NULL) {
        vpi_printf("ERROR: could not obtain subhandle %s from %s", name, vpi_get_str(vpiName, parent));
    }
}

int setHeader(char *userdata) {
    // Obtain a handle to the argument list
    vpiHandle systfref = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle args_iter = vpi_iterate(vpiArgument, systfref);


    // Grab the value of the first argument
    vpiHandle module = vpi_scan(args_iter);

    headerModule.op = obtainSubHandle("op", module);
    headerModule.bar = obtainSubHandle("bar", module);
    headerModule.width = obtainSubHandle("width", module);
    headerModule.addr = obtainSubHandle("addr", module);
    headerModule.size = obtainSubHandle("size", module);
    headerModule.new_msg = obtainSubHandle("new_msg", module);
    headerModule.word_data = obtainSubHandle("word_data", module);

    setIntValue(headerModule.op, 0);
    setIntValue(headerModule.new_msg, 0);
    setIntValue(headerModule.word_data, 0);

    headerModule.isSet = true;

    vpi_free_object(args_iter);
}

int setStatusModule(char *userdata) {
    // Obtain a handle to the argument list
    vpiHandle systfref = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle args_iter = vpi_iterate(vpiArgument, systfref);

    // Grab the value of the first argument
    vpiHandle module = vpi_scan(args_iter);

    statusModule.connected = obtainSubHandle("connected", module);
    statusModule.running = obtainSubHandle("running", module);

    statusModule.isSet = true;

    setBooleanValue(statusModule.running, false);
    setBooleanValue(statusModule.connected, false);

    vpi_free_object(args_iter);
}

static pthread_t vpcie_thread_context;
static sem_t vpcie_tick_semaphore_start;
static sem_t vpcie_tick_semaphore_stop;
static sem_t vpcie_msg_token;

static const pcie_net_msg_t *lastMsg;

unsigned int on_msg_recv(const pcie_net_msg_t *msg, pcie_net_reply_t *replay) {
    vpi_printf("Received msg\n");
    vpi_printf("OP: 0x%x\n", msg->op);
    vpi_printf("WIDTH: 0x%x\n", msg->width);
    vpi_printf("BAR: 0x%x\n", msg->bar);
    vpi_printf("SIZE: 0x%x\n", msg->size);
    vpi_printf("ADDR: 0x%x\n", msg->addr);
    setIntValue(headerModule.op, msg->op);
    setIntValue(headerModule.width, msg->width);
    setIntValue(headerModule.bar, msg->bar);
    setIntValue(headerModule.size, msg->size);
    setInt64Value(headerModule.addr, msg->addr);
    setIntValue(headerModule.new_msg, 1);
    lastMsg = msg;
    return 0;
}

static pcie_net_t *pcie_net;

void *network_main(void *data) {
    pcie_net = calloc(1, sizeof(pcie_net_t));
    pcie_net_reply_t reply;

    vpi_printf("Waiting for incoming connection from QEMU on port 10420\n");
    if ((pcie_net_init(pcie_net, "localhost", "10420")) == -1) {
        VPIERROR();
        goto onerror_not_free;
    }

    setBooleanValue(statusModule.connected, true);

    // Reserve space for a message
    pcie_net_msg_t *msg;
    if ((msg = malloc(PCIE_NET_MSG_MAX_SIZE)) == NULL) {
        VPIERROR();
        goto onerror_not_free;
    }

    struct timeval tm;
    tm.tv_sec = 0;
    tm.tv_usec = 1000;

    fd_set rfds;
    int max_fd;
    int err;
    sem_post(&vpcie_tick_semaphore_stop);

    while (vpcieStatus.running) {
        sem_wait(&vpcie_tick_semaphore_start);

        //Check if data is pending
//        if(pcie_net->task_fn != NULL)
//            tm = &pcie_net->task_tm;

        if (getIntValue(headerModule.new_msg)) {
            setIntValue(headerModule.new_msg, 0);
        }

        int freeTokens = 0;
        sem_getvalue(&vpcie_msg_token, &freeTokens);
        if (freeTokens > 0) {
            FD_ZERO(&rfds);
            FD_SET(pcie_net->fd, &rfds);

            max_fd = pcie_net->fd;

            if (pcie_net->ev_fd != -1) {
                FD_SET(pcie_net->ev_fd, &rfds);
                if (pcie_net->fd < pcie_net->ev_fd) max_fd = pcie_net->ev_fd;
            }

            err = select(max_fd + 1, &rfds, NULL, NULL, &tm);
            if (err < 0) {
                VPIERROR();
                goto onerror;
            } else if (err == 0) {
                /* timeout elapsed, task to execute */
            } else {
                sem_wait(&vpcie_msg_token);
                if (FD_ISSET(pcie_net->fd, &rfds)) {
                    err = pcie_net_recv_msg(pcie_net, msg);
                    if (err == -1) {
                        VPIERROR();
                        goto onerror;
                    } else if (err != 1) /* not icmp_unreachable case */
                    {
                        /* handle new message and reply if asked to */
                        if (on_msg_recv(msg, &reply) != 0) {
                            if (pcie_net_send_reply(pcie_net, &reply) == -1) {
                                VPIERROR();
                                goto onerror;
                            }
                        }
                    }
                } /* socket fd was set */
            }
        }
        sem_post(&vpcie_tick_semaphore_stop);
    }

    pcie_net_fini(pcie_net);

    free(msg);
    return NULL;

    onerror:
    free(msg);
    onerror_not_free:
    vpi_printf("ERROR: VPCIE network error");
    sem_post(&vpcie_tick_semaphore_stop);
    return NULL;
}

int startNetwork(char *userdata) {
    vpcieStatus.running = true;
    sem_init(&vpcie_tick_semaphore_start, 0, 0);
    sem_init(&vpcie_tick_semaphore_stop, 0, 0);
    sem_init(&vpcie_msg_token, 0, 0);
    setBooleanValue(statusModule.running, true);
    if (pthread_create(&vpcie_thread_context, NULL, &network_main, NULL) != 0) {
        vpi_printf("ERROR: Could not create network thread\n");
    }
    vpi_printf("VPCIE: Network thread started\n");
    sem_wait(&vpcie_tick_semaphore_stop);
}

int vpcieTick(char *userdata) {
    sem_post(&vpcie_tick_semaphore_start);
    sem_wait(&vpcie_tick_semaphore_stop);
}

int stopNetwork(char *userdata) {
    vpcieStatus.running = false;
    setBooleanValue(statusModule.running, false);
    sem_post(&vpcie_tick_semaphore_start);
    pthread_join(vpcie_thread_context, NULL);
}

int creditToken(char *userdata) {
    sem_post(&vpcie_msg_token);
    vpi_printf("VPCIE: token credited\n");
}

int readData(char *userdata) {
    if(lastMsg == NULL)
        return 0;

    // Obtain a handle to the argument list
    vpiHandle systfref = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle args_iter = vpi_iterate(vpiArgument, systfref);

    // Grab the value of the first argument
    vpiHandle argHandle = vpi_scan(args_iter);
    int offset = getIntValue(argHandle);

    uint32_t data = lastMsg->data[offset] |
                    lastMsg->data[offset + 1] << 8 |
                    lastMsg->data[offset + 2] << 16 |
                    lastMsg->data[offset + 3] << 24;

    setIntValue(headerModule.word_data, data);

    vpi_printf("VPCIE: Read data %x\n", data);

    vpi_free_object(args_iter);
}

int sendMemReadResponse(char *userData) {
    // Obtain a handle to the argument list
    vpiHandle systfref = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle args_iter = vpi_iterate(vpiArgument, systfref);

    // Grab the value of the first argument
    vpiHandle argHandle = vpi_scan(args_iter);
    uint32_t data = (uint32_t) getIntValue(argHandle);

    vpi_printf("VPCIE: Mem Read Response %x\n", data);
    pcie_net_reply_t reply;
    *(uint32_t *)reply.data = data;
    pcie_net_send_reply(pcie_net, &reply);
}

int sendConfigReadResponse(char *userData) {
    // Obtain a handle to the argument list
    vpiHandle systfref = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle args_iter = vpi_iterate(vpiArgument, systfref);

    // Grab the value of the first argument
    vpiHandle argHandle = vpi_scan(args_iter);
    uint32_t data = (uint32_t) getIntValue(argHandle);

    vpi_printf("VPCIE: Config Read Response %x\n", data);

    pcie_net_reply_t reply;
    *(uint32_t *)reply.data = data;
    pcie_net_send_reply(pcie_net, &reply);
}

void registerFunctions() {
    s_vpi_systf_data setHeaderData = {vpiSysTask, 0, "$setHeaderModule", setHeader, 0, 0, 0};
    s_vpi_systf_data setStatusData = {vpiSysTask, 0, "$setStatusModule", setStatusModule, 0, 0, 0};
    s_vpi_systf_data startNetworkData = {vpiSysTask, 0, "$vpcieStart", startNetwork, 0, 0, 0};
    s_vpi_systf_data stopNetworkData = {vpiSysTask, 0, "$vpcieStop", stopNetwork, 0, 0, 0};
    s_vpi_systf_data tickData = {vpiSysTask, 0, "$vpcieTick", vpcieTick, 0, 0, 0};
    s_vpi_systf_data creditTokenData = {vpiSysTask, 0, "$vpcieCreditToken", creditToken, 0, 0, 0};
    s_vpi_systf_data readDataData = {vpiSysTask, 0, "$vpcieReadData", readData, 0, 0, 0};
    s_vpi_systf_data sendConfigReadResponseData = {vpiSysTask, 0, "$vpcieSendConfigResponse", sendConfigReadResponse, 0, 0, 0};
    s_vpi_systf_data sendMemReadResponseData = {vpiSysTask, 0, "$vpcieSendMemReadResponse", sendMemReadResponse, 0, 0, 0};
    headerModule.isSet = false;
    statusModule.isSet = false;
    vpi_register_systf(&setHeaderData);
    vpi_register_systf(&setStatusData);
    vpi_register_systf(&startNetworkData);
    vpi_register_systf(&stopNetworkData);
    vpi_register_systf(&tickData);
    vpi_register_systf(&creditTokenData);
    vpi_register_systf(&readDataData);
    vpi_register_systf(&sendConfigReadResponseData);
    vpi_register_systf(&sendMemReadResponseData);
}

void (*vlog_startup_routines[])() = {
        registerFunctions,
        0
};
