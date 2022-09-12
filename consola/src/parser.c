#include "parser.h"

t_dictionary* dict_string_to_operator;

void init_dictionary()
{
    dict_string_to_operator = dictionary_create();

    dictionary_put(dict_string_to_operator, "SET", (void*)SET);
    dictionary_put(dict_string_to_operator, "ADD", (void*)ADD);
    dictionary_put(dict_string_to_operator, "MOV_IN", (void*)MOV_IN);
    dictionary_put(dict_string_to_operator, "MOV_OUT", (void*)MOV_OUT);
    dictionary_put(dict_string_to_operator, "I/O", (void*)IO);
    dictionary_put(dict_string_to_operator, "EXIT", (void*)EXIT);
}
void free_dictionary()
{
    dictionary_destroy(dict_string_to_operator);
}
t_operator get_operator(char* str)
{
    return (t_operator)dictionary_get(dict_string_to_operator, str);
}
// Devuelve la siguiente palabra de un stream
// Debe liberarse con free
char* next_word(FILE* stream)
{
    const int word_size = 10;
    char c;
    int i = 0;
    char* word = malloc(sizeof (char) * word_size);

    while((c = getc(stream)) == ' ' || c == '\t' || c == '\n');

    // Grab all words until a white space
    do {
        if(i > word_size) {
            // INSTRUCTION TOO LONG
            exit(EXIT_FAILURE);
        }
        word[i++] = c;
    }
    while((c = getc(stream)) != ' ' && c != '\t' && c != '\n' && c != EOF);

    // add a null terminating character at the end so its a correctly formatted string
    word[i] = '\0';

    return word;
}

void parse(t_log* logger, FILE* stream)
{
    init_dictionary();
    
    char c;
    while((c = getc(stream)) != EOF)
    {
    	// we check if the last character is EOF and then
    	// we put it back in the stream
    	ungetc(c, stream);

        char* word = next_word(stream);
        log_info(logger, "word: %s", word);

        // we get the corresponding operator enum
        t_operator operator = get_operator(word);

        free(word);
    }

    free_dictionary();
}
