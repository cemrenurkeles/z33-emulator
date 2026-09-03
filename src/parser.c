#include "../include/parser.h"
#include "../include/exception.h"
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>

static   Z33_OpcodeEntry opcode_table[] = {
    {"ld",    OP_LD,    2},
    {"st",    OP_ST,    2},

    {"add",   OP_ADD,   2},
    {"sub",   OP_SUB,   2},
    {"mul",   OP_MUL,   2},
    {"div",   OP_DIV,   2},
    {"neg",   OP_NEG,   1},

    {"and",   OP_AND,   2},
    {"or",    OP_OR,    2},
    {"xor",   OP_XOR,   2},
    {"not",   OP_NOT,   1},
    {"shl",   OP_SHL,   2},
    {"shr",   OP_SHR,   2},

    {"cmp",   OP_CMP,   2},

    {"jmp",   OP_JMP,   1},
    {"jeq",   OP_JEQ,   1},
    {"jne",   OP_JNE,   1},
    {"jlt",   OP_JLT,   1},
    {"jle",   OP_JLE,   1},
    {"jgt",   OP_JGT,   1},
    {"jge",   OP_JGE,   1},

    {"swap",  OP_SWAP,  2},

    {"push",  OP_PUSH,  1},
    {"pop",   OP_POP,   1},

    {"call",  OP_CALL,  1},
    {"rtn",   OP_RTN,   0},

    {"trap",  OP_TRAP,  0},
    {"rti",   OP_RTI,   0},

    {"reset", OP_RESET, 0},
    {"nop",   OP_NOP,   0},

    {"fas",   OP_FAS,   2},
    {"in",    OP_IN,    2},
    {"out",   OP_OUT,   2}
};

static Z33_Define defines[MAX_DEFINES];
size_t define_count = 0;

bool is_word_directive(char *line) {
    return strncmp(line, ".word", 5) == 0 &&
           (line[5] == '\0' || isspace((unsigned char)line[5]));
}

bool parse_word_directive(char *line, Z33_Word *value) {
    char *text = trim(line + 5);

    if (text[0] == '\0') {
        fprintf(stderr, "Error: .word expects a value\n");
        return false;
    }

    char *end;
    errno = 0;
    long long parsed = strtoll(text, &end, 0);

    if (errno == ERANGE || end == text || *trim(end) != '\0') {
        fprintf(stderr, "Error: invalid .word value\n");
        return false;
    }

    *value = (Z33_Word)parsed;
    return true;
}

bool is_addr_directive(char *line) {
    return strncmp(line, ".addr", 5) == 0 &&
           (line[5] == '\0' || isspace((unsigned char)line[5]));
}

bool is_define_directive(char *line) {
    return strncmp(line, "#define", 7) == 0 &&
           (line[7] == '\0' || isspace((unsigned char)line[7]));
}

bool parse_define_directive(char *line) {
    char *text = trim(line + 7);

    if (text[0] == '\0') {
        fprintf(stderr, "Error: #define expects a symbol\n");
        return false;
    }

    if (define_count >= MAX_DEFINES) {
        fprintf(stderr, "Error: maximum number of defines reached\n");
        return false;
    }

    char *value = text;

    while (*value != '\0' && !isspace((unsigned char)*value))
        value++;

    if (*value != '\0') {
        *value = '\0';
        value = trim(value + 1);
    }

    strcpy(defines[define_count].name, text);
    strcpy(defines[define_count].value, value);
    define_count++;

    return true;
}
bool replace_defines(char *line) {
    char result[LENGTH_MAX_LINE];
    char *src = line;
    size_t result_len = 0;

    while (*src != '\0') {
        bool replaced = false;

        for (size_t i = 0; i < define_count; i++) {
            size_t name_len = strlen(defines[i].name);

            if (strncmp(src, defines[i].name, name_len) == 0) {
                char before = src == line ? '\0' : src[-1];
                char after = src[name_len];

                bool valid_before = before == '\0' ||
                                    (!isalnum((unsigned char)before) && before != '_');

                bool valid_after = after == '\0' ||
                                   (!isalnum((unsigned char)after) && after != '_');

                if (valid_before && valid_after) {
                    size_t value_len = strlen(defines[i].value);

                    if (result_len + value_len >= LENGTH_MAX_LINE) {
                        fprintf(stderr, "Error: line too long after #define expansion\n");
                        return false;
                    }

                    memcpy(result + result_len, defines[i].value, value_len);
                    result_len += value_len;
                    src += name_len;
                    replaced = true;
                    break;
                }
            }
        }

        if (!replaced) {
            if (result_len + 1 >= LENGTH_MAX_LINE) {
                fprintf(stderr, "Error: line too long after #define expansion\n");
                return false;
            }

            result[result_len++] = *src++;
        }
    }

    result[result_len] = '\0';
    strcpy(line, result);

    return true;
}

bool parse_addr_directive(char *line, Z33_Address *address) {
    char *value_text = trim(line + 5);

    if (value_text[0] == '\0') {
        fprintf(stderr, "Error: .addr expects an address\n");
        return false;
    }

    errno = 0;
    char *end;
    long long value = strtoll(value_text, &end, 10);

    if (errno == ERANGE || end == value_text || *end != '\0') {
        fprintf(stderr, "Error: invalid .addr value\n");
        return false;
    }

    if (value < 0 || value >= Z33_MEMORY_SIZE) {
        fprintf(stderr, "Error: .addr out of memory range\n");
        return false;
    }

    *address = (Z33_Address)value;
    return true;
}

void print_operand(const Z33_Operand *operand) {
    switch (operand->type) {
        case OPERAND_IMM:
            fprintf(stderr,"%lld", (long long)operand->value.immediate);
            break;

        case OPERAND_REG:
            switch (operand->value.reg) {
                case REG_A:  fprintf(stderr,"%%a"); break;
                case REG_B:  fprintf(stderr,"%%b"); break;
                case REG_PC: fprintf(stderr,"%%pc"); break;
                case REG_SP: fprintf(stderr,"%%sp"); break;
                case REG_SR: fprintf(stderr,"%%sr"); break;
            }
            break;

        case OPERAND_DIR:
            fprintf(stderr,"[%u]", operand->value.address);
            break;

        case OPERAND_IND:
            switch (operand->value.reg) {
                case REG_A:  fprintf(stderr,"[%%a]"); break;
                case REG_B:  fprintf(stderr,"[%%b]"); break;
                case REG_PC: fprintf(stderr,"[%%pc]"); break;
                case REG_SP: fprintf(stderr,"[%%sp]"); break;
                case REG_SR: fprintf(stderr,"[%%sr]"); break;
            }
            break;

        case OPERAND_IDX:
            switch (operand->value.indexed.reg) {
                case REG_A:  fprintf(stderr,"[%%a"); break;
                case REG_B:  fprintf(stderr,"[%%b"); break;
                case REG_PC: fprintf(stderr,"[%%pc"); break;
                case REG_SP: fprintf(stderr,"[%%sp"); break;
                case REG_SR: fprintf(stderr,"[%%sr"); break;
            }

            if (operand->value.indexed.offset >= 0)
                fprintf(stderr,"+%d]", operand->value.indexed.offset);
            else
                fprintf(stderr,"%d]", operand->value.indexed.offset);

            break;
    }
}

void print_instruction(const Z33_Instruction *instruction) {
    const char *mnemonic = "invalid";

    size_t count = sizeof(opcode_table) / sizeof(opcode_table[0]);

    for (size_t i = 0; i < count; i++) {
        if (opcode_table[i].opcode == instruction->opcode) {
            mnemonic = opcode_table[i].mnemonic;
            break;
        }
    }

    fprintf(stderr,"%s", mnemonic);

    if (instruction->n_op >= 1) {
        fprintf(stderr," ");
        print_operand(&instruction->op[0]);
    }

    if (instruction->n_op == 2) {
        fprintf(stderr,", ");
        print_operand(&instruction->op[1]);
    }
}

bool is_string_directive(char *line) {
    return strncmp(line, ".string", 7) == 0 &&
           (line[7] == '\0' || isspace((unsigned char)line[7]));
}

bool get_string_length(char *line, size_t *length) {
    char *text = trim(line + 7);

    if (text[0] != '"') {
        fprintf(stderr, "Error: .string expects a quoted string\n");
        return false;
    }

    size_t len = strlen(text);

    if (len < 2 || text[len - 1] != '"') {
        fprintf(stderr, "Error: missing closing quote in .string\n");
        return false;
    }

    size_t count = 0;

    for (size_t i = 1; i < len - 1; i++) {
        if (text[i] == '\\') {
            if (i + 1 >= len - 1) {
                fprintf(stderr, "Error: invalid escape sequence\n");
                return false;
            }

            i++;
        }

        count++;
    }

    *length = count;
    return true;
}

char *trim(char *str)
{
    while (*str == ' ' || *str == '\t')
        str++;
    char *end = str + strlen(str);
    while (end > str &&
          (end[-1] == ' ' || end[-1] == '\t' ||
           end[-1] == '\n' || end[-1] == '\r')) {
        end--;
    }

    *end = '\0';

    return str;
}

bool isLabel(char * text){
    trim(text);
    if(text[strlen(text)-1]==':'){
        if(strlen(text)<LENGTH_MAX_LABEL){
            if(isalpha((unsigned char)text[0]))
                return true;
            else{
                fprintf(stderr,"Error: label must start with a letter\n");
                return false;
            }
        }
        else{
            fprintf(stderr,"Error: label cannot exceed 256 characters \n");
            return false;
        }
    }
    return false;
}
bool write_string_directive(Z33_Machine *machine, char *line, Z33_Address *address) {
    char *text = trim(line + 7);
    size_t len = strlen(text);

    for (size_t i = 1; i < len - 1; i++) {
        unsigned char c;

        if (text[i] == '\\') {
            i++;

            switch (text[i]) {
                case 'n':
                    c = '\n';
                    break;
                case 't':
                    c = '\t';
                    break;
                case '\\':
                    c = '\\';
                    break;
                case '"':
                    c = '"';
                    break;
                default:
                    fprintf(stderr, "Error: invalid escape sequence \\%c\n", text[i]);
                    return false;
            }
        } else {
            c = (unsigned char)text[i];
        }

        if (*address >= Z33_MEMORY_SIZE) {
            fprintf(stderr, "Error: string exceeds memory size\n");
            return false;
        }

        if (!write_Word_to_memory(&machine->memory, (Z33_Word)c, *address))
            return false;

        (*address)++;
    }

    return true;
}

bool parse_file(Z33_Machine *machine, char *filename) {
    Z33_Label labels[MAX_LABELS];
    size_t label_count = 0;
    Z33_Address address = 1000;
    int n_line = 0;

    define_count = 0;

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening the file\n");
        return false;
    }

    char line[LENGTH_MAX_LINE];

    /* ---------- FIRST PASS: collect labels ---------- */

    while (fgets(line, sizeof(line), file) != NULL) {
        n_line++;
        line[strcspn(line, "\r\n")] = '\0';

        char *current = trim(line);

        if (current[0] == '\0')
            continue;
        if (is_word_directive(current)) {
            Z33_Word value;

            if (!parse_word_directive(current, &value)) {
                fclose(file);
                return false;
            }

            if (address >= Z33_MEMORY_SIZE) {
                fprintf(stderr, "Error: .word exceeds memory\n");
                fclose(file);
                return false;
            }

            address++;
            continue;
        }
        if (is_define_directive(current)) {
            if (!parse_define_directive(current)) {
                fprintf(stderr, "Error parsing line %d: %s\n", n_line, current);
                fclose(file);
                return false;
            }

            continue;
        }

        if (!replace_defines(current)) {
            fclose(file);
            return false;
        }

        if (is_addr_directive(current)) {
            if (!parse_addr_directive(current, &address)) {
                fprintf(stderr, "Error parsing line %d: %s\n", n_line, current);
                fclose(file);
                return false;
            }

            continue;
        }

        if (is_string_directive(current)) {
            size_t length;

            if (!get_string_length(current, &length)) {
                fprintf(stderr, "Error parsing line %d: %s\n", n_line, current);
                fclose(file);
                return false;
            }

            if ((size_t)address + length > Z33_MEMORY_SIZE) {
                fprintf(stderr, "Error: string exceeds memory size\n");
                fclose(file);
                return false;
            }

            address += (Z33_Address)length;
            continue;
        }

        if (isLabel(current)) {
            if (label_count >= MAX_LABELS) {
                fprintf(stderr, "Error: maximum number of labels reached\n");
                fclose(file);
                return false;
            }

            current[strlen(current) - 1] = '\0';
            current = trim(current);

            for (size_t i = 0; i < label_count; i++) {
                if (strcmp(labels[i].name, current) == 0) {
                    fprintf(stderr, "Error: label '%s' already defined\n", current);
                    fclose(file);
                    return false;
                }
            }

            strcpy(labels[label_count].name, current);
            labels[label_count].address = address;
            label_count++;

            continue;
        }

        if (address >= Z33_MEMORY_SIZE) {
            fprintf(stderr, "Error: program exceeds memory size\n");
            fclose(file);
            return false;
        }

        address++;
    }

    rewind(file);

    address = 1000;
    n_line = 0;
    define_count = 0;

    /* ---------- SECOND PASS: parse and load ---------- */

    while (fgets(line, sizeof(line), file) != NULL) {
        n_line++;
        line[strcspn(line, "\r\n")] = '\0';

        char *current = trim(line);

        if (current[0] == '\0')
            continue;

        
        if (is_word_directive(current)) {
            Z33_Word value;

            if (!parse_word_directive(current, &value)) {
                fclose(file);
                return false;
            }

            if (!write_Word_to_memory(&machine->memory, value, address)) {
                fclose(file);
                return false;
            }

            address++;
            continue;
        }
        if (is_define_directive(current)) {
            if (!parse_define_directive(current)) {
                fprintf(stderr, "Error parsing line %d: %s\n", n_line, current);
                fclose(file);
                return false;
            }

            continue;
        }

        if (!replace_defines(current)) {
            fclose(file);
            return false;
        }

        if (is_addr_directive(current)) {
            if (!parse_addr_directive(current, &address)) {
                fprintf(stderr, "Error parsing line %d: %s\n", n_line, current);
                fclose(file);
                return false;
            }

            continue;
        }

        if (is_string_directive(current)) {
            if (!write_string_directive(machine, current, &address)) {
                fprintf(stderr, "Error parsing line %d: %s\n", n_line, current);
                fclose(file);
                return false;
            }

            continue;
        }

        if (isLabel(current))
            continue;

        if (address >= Z33_MEMORY_SIZE) {
            fprintf(stderr, "Error: program exceeds memory size\n");
            fclose(file);
            return false;
        }

        fprintf(stderr,"%d  %s\n", n_line, current);
        fflush(stdout);

        Z33_Instruction inst;

        if (!parse_line(machine, current, &inst, labels, label_count)) {
            fprintf(stderr, "Error parsing line %d: %s\n", n_line, current);
            fclose(file);
            return false;
        }

        inst.line = n_line;

        if (!write_Instruction_to_memory(&machine->memory, inst, address)) {
            fclose(file);
            return false;
        }

        address++;
    }

    fclose(file);
    return true;
}

bool verify_opcode(char *line, Z33_Instruction *inst)
{
    char mnemonic[LENGTH_MAX_OPCODE];

    if (sscanf(line, "%9s", mnemonic) != 1)
        return false;

    size_t count = sizeof(opcode_table) / sizeof(opcode_table[0]);

    for (size_t i = 0; i < count; i++) {
        if (strcmp(mnemonic, opcode_table[i].mnemonic) == 0) {
            inst->opcode = opcode_table[i].opcode;
            inst->n_op = opcode_table[i].n_op;
            return true;
        }
    }
    fprintf(stderr,"Error : unknown instruction \n");
    return false;

}

char * remove_words_separated_space(  char *line ){
    char * operands = strchr(line,' ');
    if(operands==NULL) return NULL;
    while(*operands ==' ') operands++;
    return operands;
}


void to_lowercase(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
        str[i] = (char)tolower((unsigned char)str[i]);
}

char * cut_2_operands(char * text){
    char *second = strchr(text,',');
    if(second==NULL) return NULL;
    
    *second='\0';
    second++;
    
    return trim(second);
}

bool parse_line(Z33_Machine *machine, char *line, Z33_Instruction *instruction, const Z33_Label *labels, size_t label_count) {
    instruction->opcode = OP_INVALID;
    instruction->n_op = 0;

    to_lowercase(line);

    if (!verify_opcode(line, instruction))
        return false;

    char *operandes = remove_words_separated_space(line);

    if (instruction->n_op == 0) {
        if (operandes == NULL)
            return true;

        operandes = trim(operandes);

        if (operandes[0] == '\0')
            return true;

        fprintf(stderr, "Error: unexpected operand\n");
        instruction->opcode = OP_INVALID;
        instruction->n_op = 0;
        return false;
    }

    if (operandes == NULL) {
        fprintf(stderr, "Error: missing operand\n");
        instruction->opcode = OP_INVALID;
        instruction->n_op = 0;
        return false;
    }

    operandes = trim(operandes);

    if (instruction->n_op == 1) {
        if (strchr(operandes, ',') != NULL) {
            fprintf(stderr, "Error: too many operands\n");
            instruction->opcode = OP_INVALID;
            instruction->n_op = 0;
            return false;
        }

        Z33_Operand op1;

        if (!parse_operand(machine, operandes, &op1, labels, label_count)) {
            fprintf(stderr, "Error: invalid operand: %s\n", operandes);
            instruction->opcode = OP_INVALID;
            instruction->n_op = 0;
            return false;
        }

        instruction->op[0] = op1;
        return true;
    }

    if (instruction->n_op == 2) {
        Z33_Operand op1, op2;
        char *second = cut_2_operands(operandes);

        if (second == NULL) {
            fprintf(stderr, "Error: expected two operands separated by a comma\n");
            instruction->opcode = OP_INVALID;
            instruction->n_op = 0;
            return false;
        }

        operandes = trim(operandes);
        second = trim(second);

        if (operandes[0] == '\0' || second[0] == '\0') {
            fprintf(stderr, "Error: missing operand\n");
            instruction->opcode = OP_INVALID;
            instruction->n_op = 0;
            return false;
        }

        if (strchr(second, ',') != NULL) {
            fprintf(stderr, "Error: too many operands\n");
            instruction->opcode = OP_INVALID;
            instruction->n_op = 0;
            return false;
        }

        if (!parse_operand(machine, operandes, &op1, labels, label_count)) {
            fprintf(stderr, "Error: invalid first operand: %s\n", operandes);
            instruction->opcode = OP_INVALID;
            instruction->n_op = 0;
            return false;
        }

        if (!parse_operand(machine, second, &op2, labels, label_count)) {
            fprintf(stderr, "Error: invalid second operand: %s\n", second);
            instruction->opcode = OP_INVALID;
            instruction->n_op = 0;
            return false;
        }

        instruction->op[0] = op1;
        instruction->op[1] = op2;
        return true;
    }

    instruction->opcode = OP_INVALID;
    instruction->n_op = 0;
    return false;
}

bool is_Immediate (char * text){
    char *end;
    strtoll(text, &end, 10);
    if (end!=text && *end == '\0') return true;
    return false;
}

bool parse_operand(Z33_Machine *machine, char *text, Z33_Operand *operand, const Z33_Label *labels, size_t label_count) {
    if (text == NULL || text[0] == '\0')
        return false;

    /* Immediate value */
    if (is_Immediate(text)) {
        errno = 0;
        long long value = strtoll(text, NULL, 10);

        if (errno == ERANGE) {
            fprintf(stderr, "Error: immediate value out of range\n");
            return false;
        }

        operand->type = OPERAND_IMM;
        operand->value.immediate = (Z33_Word)value;
        return true;
    }

    /* Register */
    if (text[0] == '%')
        return parse_register(text, operand);

    /* Memory operand */
    if (text[0] == '[') {
        size_t len = strlen(text);

        if (len < 2 || text[len - 1] != ']') {
            fprintf(stderr, "Error: missing ']'\n");
            return false;
        }

        text++;
        text[strlen(text) - 1] = '\0';
        text = trim(text);

        /* Direct addressing: [500] */
        if (is_Immediate(text)) {
            errno = 0;
            long long address = strtoll(text, NULL, 10);

            if (errno == ERANGE) {
                fprintf(stderr, "Error: address out of range\n");
                return false;
            }

            if (address < 0 || address >= Z33_MEMORY_SIZE) {
                fprintf(stderr, "Error: invalid memory address %lld\n", address);
                z33_raise_exception(machine, EX_INVALID_MEMORY);
                return false;
            }

            operand->type = OPERAND_DIR;
            operand->value.address = (Z33_Address)address;
            return true;
        }

        /* Indexed addressing: [%a+5] / [%a-5] */
        char *plus = strchr(text, '+');
        char *minus = strchr(text, '-');
        char *sign = (plus != NULL) ? plus : minus;

        if (sign != NULL) {
            char sign_char = *sign;
            *sign = '\0';

            char *reg_text = trim(text);
            char *offset_text = trim(sign + 1);
            Z33_Operand reg_operand;

            if (!parse_register(reg_text, &reg_operand))
                return false;

            if (!is_Immediate(offset_text)) {
                fprintf(stderr, "Error: invalid indexed offset\n");
                return false;
            }

            errno = 0;
            long long offset = strtoll(offset_text, NULL, 10);

            if (errno == ERANGE) {
                fprintf(stderr, "Error: offset out of range\n");
                return false;
            }

            if (sign_char == '-')
                offset = -offset;

            if (offset < INT32_MIN || offset > INT32_MAX) {
                fprintf(stderr, "Error: offset out of range\n");
                return false;
            }

            operand->type = OPERAND_IDX;
            operand->value.indexed.reg = reg_operand.value.reg;
            operand->value.indexed.offset = (int32_t)offset;
            return true;
        }

        /* Indirect addressing: [%a] */
        Z33_Operand reg_operand;

        if (parse_register(text, &reg_operand)) {
            operand->type = OPERAND_IND;
            operand->value.reg = reg_operand.value.reg;
            return true;
        }

        return false;
    }

    /* Label */
    for (size_t i = 0; i < label_count; i++) {
        if (strcmp(text, labels[i].name) == 0) {
            operand->type = OPERAND_IMM;
            operand->value.immediate = (Z33_Word)labels[i].address;
            return true;
        }
    }

    fprintf(stderr, "Error: unknown label or invalid operand '%s'\n", text);
    return false;
}

bool parse_register(char *text,Z33_Operand *operand){
    if(strcmp(text,"%a")==0){
        operand->type=OPERAND_REG;
        operand->value.reg=REG_A;
        return true;
    }
    if(strcmp(text,"%b")==0){
        operand->type=OPERAND_REG;
        operand->value.reg=REG_B;
        return true;
    }
    if(strcmp(text,"%pc")==0){
        operand->type=OPERAND_REG;
        operand->value.reg=REG_PC;
        return true;
    }
    if(strcmp(text,"%sp")==0){
        operand->type=OPERAND_REG;
        operand->value.reg=REG_SP;
        return true;
    }
    if(strcmp(text,"%sr")==0){
        operand->type=OPERAND_REG;
        operand->value.reg=REG_SR;
        return true;
    }
    fprintf(stderr,"parse_register : Error wrong register name\n");
    return false;
}
