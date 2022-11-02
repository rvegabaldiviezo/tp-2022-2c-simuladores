#include <commons/collections/list.h>
#include "structures.h"

#ifndef __STRUCTURES_TRANSLATION_H
#define __STRUCTURES_TRANSLATION_H

const char * const t_instruction_type_string[] = {
    [SET] = "SET",
    [ADD] = "ADD",
    [MOV_IN] = "MOV_IN",
    [MOV_OUT] = "MOV_OUT",
    [IO] = "I/O",
    [EXIT] = "EXIT"
};
const char * const t_register_string[] = {
    [AX] = "AX",
    [BX] = "BX",
    [CX] = "CX",
    [DX] = "DX"
};
const char * const t_interrupt_type_string[] = {
    [INT_QUANTUM] = "INT_QUANTUM",
    [INT_IO] = "INT_IO",
    [INT_PAGE_FAULT] = "INT_PAGE_FAULT",
    [EXECUTION_FINISHED] = "EXECUTION_FINISHED"
};

#endif