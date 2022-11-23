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
#include <math.h>


void* start_interrupt(void* arg);

void setup(char* path);

void connections();

void instruction_cycle();

void free_memory();

void set_execute(t_pcb* pcb, t_register reg1, uint32_t param1);

void add_execute(t_pcb* pcb, t_register reg1, t_register reg2);

void mov_in_execute(t_pcb* pcb, t_register reg1, uint32_t param1);

void mov_out_execute(t_pcb* pcb, uint32_t param1, t_register reg1);

int check_tlb(int process_id, int segment_num, int page_num);

void request_data_in(int frame, int page_offset, t_pcb* pcb, t_register reg1);

void request_data_out(int frame, int page_offset, t_pcb* pcb, t_register reg1);
