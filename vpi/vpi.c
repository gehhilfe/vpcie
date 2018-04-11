//
// Created by timb on 10.04.18.
//

#include <vpi_user.h>
#include <pthread.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

struct {
    vpiHandle op;
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

void setBooleanValue(vpiHandle handle, bool value) {
    struct t_vpi_value argval;

    argval.format = vpiBinStrVal;
    argval.value.str = (value)?"1":"0";
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

    setIntValue(headerModule.op, 0);

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

void *network_main(void *data) {

    bool flag = true;

    while (vpcieStatus.running) {
        sem_wait(&vpcie_tick_semaphore_start);
        setBooleanValue(statusModule.connected, flag);
        flag = !flag;
        sem_post(&vpcie_tick_semaphore_stop);
    }
    return NULL;
}

int startNetwork(char *userdata) {
    vpcieStatus.running = true;
    sem_init(&vpcie_tick_semaphore_start, 0, 0);
    sem_init(&vpcie_tick_semaphore_stop, 0, 0);
    setBooleanValue(statusModule.running, true);
    if(pthread_create(&vpcie_thread_context, NULL, &network_main, NULL) != 0) {
        vpi_printf("ERROR: Could not create network thread\n");
    }
    vpi_printf("VPCIE: Network thread started\n");
}

int vpcieTick(char *userdata) {
    sem_post(&vpcie_tick_semaphore_start);
    vpi_printf("VPCIE: TICK\n");
    sem_wait(&vpcie_tick_semaphore_stop);
    vpi_printf("VPCIE: TICK DONE\n");
}

int stopNetwork(char *userdata) {
    vpcieStatus.running = false;
    setBooleanValue(statusModule.running, false);
    sem_post(&vpcie_tick_semaphore_start);
    pthread_join(vpcie_thread_context, NULL);
}

void registerFunctions() {
    s_vpi_systf_data setHeaderData = {vpiSysTask, 0, "$setHeaderModule", setHeader, 0, 0, 0};
    s_vpi_systf_data setStatusData = {vpiSysTask, 0, "$setStatusModule", setStatusModule, 0, 0, 0};
    s_vpi_systf_data startNetworkData = {vpiSysTask, 0, "$vpcieStart", startNetwork, 0, 0, 0};
    s_vpi_systf_data stopNetworkData = {vpiSysTask, 0, "$vpcieStop", stopNetwork, 0, 0, 0};
    s_vpi_systf_data tickData = {vpiSysTask, 0, "$vpcieTick", vpcieTick, 0, 0, 0};
    headerModule.isSet = false;
    statusModule.isSet = false;
    vpi_register_systf(&setHeaderData);
    vpi_register_systf(&setStatusData);
    vpi_register_systf(&startNetworkData);
    vpi_register_systf(&stopNetworkData);
    vpi_register_systf(&tickData);
}

void (*vlog_startup_routines[])() = {
        registerFunctions,
        0
};
