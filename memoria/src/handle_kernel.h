#include "memoria.h"

#ifndef __HANDLE_KERNEL_H
#define __HANDLE_KERNEL_H

void handle_kernel();
void process_started();
void process_finished();
void resolve_page_fault();
int find_free_frame(t_pcb* pcb, int segment, int page);
int* read_page_from_swap(t_page_table_data* page_data, t_pcb* pcb, int segment, int page);
void write_page_to_swap(t_page_table_data* page_data, t_pcb* pcb, int segment, int page);
t_page_table_data* find_victim(t_pcb* pcb, int segment, int page);
bool is_victim(t_page_table_data* page, int iteration);


#endif