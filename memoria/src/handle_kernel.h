#include "memoria.h"

#ifndef __HANDLE_KERNEL_H
#define __HANDLE_KERNEL_H

void handle_kernel();
void process_started();
void process_finished();
void resolve_page_fault();
int find_free_frame();
void* read_page_from_swap(t_page_table_data* page);
void write_page_to_swap(t_page_table_data* page);
t_page_table_data* find_victim();
bool is_victim(t_page_table_data* page, int iteration);


#endif