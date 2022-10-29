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

#endif