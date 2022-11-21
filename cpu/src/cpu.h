#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <shared/log_extras.h>
#include <shared/structures.h>
#include <shared/structures_translation.h>
#include <shared/serialization.h>
#include <shared/socket.h>
#include <shared/environment_variables.h>
#include <pthread.h>
#include <semaphore.h>

#ifndef __CPU_H
#define __CPU_H

typedef struct {
	int pid;
	int segment;
	int page;
	int frame;
	int time;
} t_tlb;

#endif

void* start_interrupt(void* arg);

void setup(char* path);

void connections();

void instruction_cycle();

void free_memory();

void set_execute(t_pcb* pcb, t_register reg1, uint32_t param1);

void add_execute(t_pcb* pcb, t_register reg1, t_register reg2);

// void mov_in_execute(t_pcb* pcb, t_register reg1, uint32_t param1);


