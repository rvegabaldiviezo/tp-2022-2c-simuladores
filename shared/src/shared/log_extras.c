#include <stdarg.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "structures.h"
#include "log_extras.h"
#include "structures_translation.h"

#define RECTANGLE_WIDTH 50

void log_rect(t_log* logger, const char* title, const char* body, ...)
{
	va_list arguments;
	va_start(arguments, body);
	body = string_from_vformat(body, arguments);

	char* rectangle = string_new();
	int odd_align = (string_length(title) % 2 != 0) ? 1 : 0;
	char* top_left = string_repeat('-', (RECTANGLE_WIDTH - string_length(title)) / 2);
	char* top_right = string_repeat('-', (RECTANGLE_WIDTH - string_length(title)) / 2 + odd_align);
	string_append_with_format(&rectangle, "+%s%s%s+\n", top_left, title, top_right);
	free(top_left);
	free(top_right);

	int line_count = 1;
	for (int i = 0; i < strlen(body); i++) {
		char c = body[i];

		if (i == 0) 
		{
			string_append_with_format(&rectangle, "| %c", c);
			line_count++;
		} 
		else if (c == '\n') 
		{
			char* fill = string_repeat(' ', RECTANGLE_WIDTH - line_count >= 0 ? RECTANGLE_WIDTH - line_count : 0);
			string_append_with_format(&rectangle, "%s|\n| ", fill);
			line_count = 1;
			free(fill);
		}
		else if (i == strlen(body) - 1)
		{
			char* fill = string_repeat(' ', RECTANGLE_WIDTH - line_count - 1 >= 0 ? RECTANGLE_WIDTH - line_count - 1 : 0);
			string_append_with_format(&rectangle, "%c%s|\n", c, fill);
			line_count = 1;
			free(fill);
		}
		else 
		{
			string_append_with_format(&rectangle, "%c", c);
			line_count++;
		}
	}
	char* bottom = string_repeat('-', RECTANGLE_WIDTH);
	string_append_with_format(&rectangle, "+%s+\n", bottom);

	log_trace(logger, "\n%s", rectangle);
	free(rectangle);
}

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
	char* log_message = string_new();

	for(int i = 0; i < list_size(instructions); i++) {
		t_instruction* inst = list_get(instructions, i);

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
		if(i != list_size(instructions) - 1) string_append(&log_message, "\n");
	}
	log_rect(logger, "instructions", log_message);
}

void log_pcb(t_log* logger, t_pcb* pcb)
{
	log_rect(logger, "pcb", 
		"pcb->id: %i \n"
		"pcb->interrupt_type: %s \n"
		"pcb->process_size: %i \n"
		"pcb->program_counter: %i \n"
		"pcb->register[%s]: %i \n"
		"pcb->register[%s]: %i \n"
		"pcb->register[%s]: %i \n"
		"pcb->register[%s]: %i \n"
		"pcb->page_table: %i \n"
		"pcb->estimated_burst: %f \n"
		"pcb->socket_consola: %i \n"
		"pcb->start_burst: %f \n"
		"pcb->estimated_remaining_burst: %f \n"
		"pcb->execution_time: %f \n"
		"pcb->instructions:", 
		pcb->id, 
		t_interrupt_type_string[pcb->interrupt_type],
		pcb->process_size,
		pcb->program_counter,
		t_register_string[AX], pcb->registers[AX],
		t_register_string[BX], pcb->registers[BX],
		t_register_string[CX], pcb->registers[CX],
		t_register_string[DX], pcb->registers[DX],
		pcb->page_table,
		pcb->estimated_burst,
		pcb->socket_consola,
		pcb->start_burst,
		pcb->estimated_remaining_burst,
		pcb->execution_time
	);
	log_instructions(logger, pcb->instructions);
}
