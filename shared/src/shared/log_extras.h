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

void log_rectangle(t_log* logger, char border, char fill, align align, const char* message, ...);
void log_instructions(t_log* logger, t_list* instructions);
void log_pcb(t_log* logger, t_pcb* pcb);

#endif
