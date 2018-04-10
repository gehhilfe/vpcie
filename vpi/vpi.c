//
// Created by timb on 10.04.18.
//

#include <vpi_user.h>
#include <pthread.h>

struct {
    vpiHandle op;
} header;

int setIntValue(vpiHandle handle, int value) {
    struct t_vpi_value argval;

    argval.format = vpiIntVal;
    argval.value.integer = value;
    vpi_put_value(handle, &argval, NULL, vpiNoDelay);
}

int setHeader(char *userdata) {
    // Obtain a handle to the argument list
    vpiHandle systfref = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle args_iter = vpi_iterate(vpiArgument, systfref);


    // Grab the value of the first argument
    vpiHandle headerModule = vpi_scan(args_iter);

    header.op = vpi_handle_by_name("op", headerModule);

    if(header.op == NULL)
    {
        vpi_printf("ERROR: could not obtain op register\n");
        return(0);
    }

    setIntValue(header.op, 0);

    vpi_free_object(args_iter);
}

void registerFunctions() {
    s_vpi_systf_data setHeaderData = {vpiSysTask, 0, "$setHeader", setHeader, 0, 0, 0};
    vpi_register_systf(&setHeaderData);
}

void (*vlog_startup_routines[])() = {
        registerFunctions,
        0
};
