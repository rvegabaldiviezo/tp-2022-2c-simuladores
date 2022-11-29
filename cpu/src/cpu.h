#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <shared/log_extras.h>
#include <shared/structures.h>
#include <shared/structures_translation.h>
#include <shared/serialization.h>
#include <shared/socket.h>
#include <shared/environment_variables.h>

#ifndef __CPU_H
#define __CPU_H

typedef enum {
	IN,
	OUT
} t_in_out;

#endif

void* start_interrupt(void* arg);

void setup(char **argv);

void connections();

void instruction_cycle();

void free_memory();

void set_execute(t_pcb* pcb, t_register reg1, uint32_t param1);

void add_execute(t_pcb* pcb, t_register reg1, t_register reg2);

void mov_execute(t_pcb* pcb, t_register reg1, uint32_t param1, int in_out);

int check_tlb(int process_id, int segment_num, int page_num);

void mmu (int dl, int* segment_max_size, int* segment_num, int* segment_offset, int* page_num, int* page_offset);

void tlb_access(t_pcb* pcb, int segment_num, int page_num, int frame, int page_offset, uint32_t reg1, int in_out);

void add_to_tlb(int pid, int segment_num, int page_num, int frame);

void request_data_in(int frame, int page_offset, t_pcb* pcb, t_register reg1);

void request_data_out(int frame, int page_offset, t_pcb* pcb, t_register reg1);

void pf_occurred(int pid, int segment_num, int page_num);

void* consistency_check(void* arg);
