#include <stdarg.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <shared/structures.h>
#include "log_extras.h"
#include "structures_translation.h"

#define RECTANGLE_WIDTH 30

void log_rectangle(t_log* logger, char border, char fill, align align, const char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* msg = string_from_vformat(message, arguments);
	int msg_length = string_length(msg);

	if(msg_length > RECTANGLE_WIDTH) {
		log_trace(logger, "%c%s", border, msg);
		return;
	}

	int padding_left_count = 0;
	int padding_right_count = 0;

	int odd_align = (msg_length % 2 != 0) ? 1 : 0;

	switch(align) {
		case LEFT:
			padding_left_count = 0;
			padding_right_count = RECTANGLE_WIDTH - msg_length;
			break;
		case CENTER: 
			padding_left_count = (RECTANGLE_WIDTH - msg_length) / 2;
			padding_right_count = (RECTANGLE_WIDTH - msg_length) / 2 + odd_align;
			break;
		case RIGHT:
			padding_left_count = RECTANGLE_WIDTH - msg_length;
			padding_right_count = 0;
			break;
	}

	char* padding_left = string_repeat(fill, padding_left_count);
	char* padding_righ = string_repeat(fill, padding_right_count);

	log_trace(logger, "%c%s%s%s%c", border, padding_left, msg, padding_righ, border);

	va_end(arguments);
}

void log_instructions(t_log* logger, t_list* instructions)
{
	// Las muestro por pantall (eliminar luego esto)
	log_rectangle(logger, '|', '-', CENTER, "{instructions}");

	for(int i = 0; i < list_size(instructions); i++) {
		t_instruction* inst = list_get(instructions, i);

		char* log_message = string_new();
		string_append_with_format(&log_message, "%s", t_instruction_type_string[inst->instruction]);

		t_list* parameters = inst->parameters;

		for(int j = 0; j < list_size(parameters); j++)
		{
			t_parameter* param = (t_parameter*)list_get(parameters, j);

			if(param->is_string)
				string_append_with_format(&log_message, " %s", (char*)param->parameter);
			else
				string_append_with_format(&log_message, " %i", (int)param->parameter);
		}

		log_rectangle(logger, '|', ' ', LEFT, log_message);
	}
	log_rectangle(logger, '|', '=', CENTER, "instructions");
}

void log_pcb(t_log* logger, t_pcb* pcb)
{
	log_rectangle(logger, '=', '=', CENTER, "{pcb}");
	log_rectangle(logger, '|', '-', CENTER, "");
	log_rectangle(logger, '|', ' ', LEFT, "pcb->id: %i", pcb->id);
	log_rectangle(logger, '|', ' ', LEFT, "pcb->process_size: %i", pcb->process_size);
	log_rectangle(logger, '|', ' ', LEFT, "pcb->program_counter: %i", pcb->program_counter);
	log_rectangle(logger, '|', ' ', LEFT, "pcb->page_table: %i", pcb->page_table);
	log_rectangle(logger, '|', ' ', LEFT, "pcb->estimated_burst: %f", pcb->estimated_burst);
	log_rectangle(logger, '|', ' ', LEFT, "pcb->socket_consola: %i", pcb->socket_consola);
	log_rectangle(logger, '|', ' ', LEFT, "pcb->start_burst: %f", pcb->start_burst);
	log_rectangle(logger, '|', ' ', LEFT, "pcb->estimated_remaining_burst: %f", pcb->estimated_remaining_burst);
	log_rectangle(logger, '|', ' ', LEFT, "pcb->execution_time: %f", pcb->execution_time);
	log_rectangle(logger, '|', ' ', LEFT, "pcb->instructions:");
	log_instructions(logger, pcb->instructions);
	log_rectangle(logger, '=', '=', CENTER, "pcb");
}