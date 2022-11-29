#include <commons/collections/list.h>
#include <commons/log.h>
#include "structures.h"

#ifndef __LOG_EXTRAS_H
#define __LOG_EXTRAS_H

typedef enum {
	CENTER,
	LEFT,
	RIGHT
} align;

void log_rect(t_log* logger, const char* title, const char* body, ...);
void log_rectangle(t_log* logger, char border, char fill, align align, const char* message, ...);
void log_instructions(t_log* logger, t_list* instructions);
void log_pcb(t_log* logger, t_pcb* pcb);
void log_tlb(t_log* logger, t_list* tlb);

#endif
