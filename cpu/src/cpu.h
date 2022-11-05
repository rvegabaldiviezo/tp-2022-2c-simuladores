

void* start_interrupt(void* arg);

//void setup();

void connections();

void instruction_cycle();

void free_memory();

void set_execute(t_pcb* pcb, t_register reg1, uint32_t param1);

void add_execute(t_pcb* pcb, t_register reg1, t_register reg2);

// void mov_in_execute(t_pcb* pcb, t_register reg1, uint32_t param1);
