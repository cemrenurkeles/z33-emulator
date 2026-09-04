#include "../include/parser.h"
#include "../include/exception.h"
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>

#define MAX_INCLUDE_DEPTH 32
#define MAX_CONDITIONAL_DEPTH 64
#define LENGTH_MAX_PATH 1024

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
static const Z33_Label *expression_labels;
static size_t expression_label_count;

typedef struct {
    bool parent_active;
    bool branch_taken;
    bool active;
    bool else_seen;
} ConditionalBlock;

typedef struct {
    ConditionalBlock blocks[MAX_CONDITIONAL_DEPTH];
    size_t depth;
} ConditionalState;

typedef struct {
    const char *text;
    bool valid;
    bool undefined_is_zero;
} ExpressionParser;

static bool is_directive(const char *line, const char *directive) {
    size_t length = strlen(directive);
    return strncmp(line, directive, length) == 0 &&
           (line[length] == '\0' || isspace((unsigned char)line[length]));
}

static void to_lowercase(char *str);

static const char *find_define_value(const char *name) {
    for (size_t i = 0; i < define_count; i++) {
        if (strcmp(defines[i].name, name) == 0)
            return defines[i].value;
    }
    return NULL;
}

static void skip_expression_spaces(ExpressionParser *parser) {
    while (isspace((unsigned char)*parser->text))
        parser->text++;
}

static long long parse_expression_or(ExpressionParser *parser);

static long long parse_expression_primary(ExpressionParser *parser) {
    skip_expression_spaces(parser);

    if (*parser->text == '(') {
        parser->text++;
        long long value = parse_expression_or(parser);
        skip_expression_spaces(parser);
        if (*parser->text != ')') {
            parser->valid = false;
            return 0;
        }
        parser->text++;
        return value;
    }

    if (isalpha((unsigned char)*parser->text) || *parser->text == '_') {
        char name[LENGTH_MAX_LABEL];
        size_t length = 0;
        while (isalnum((unsigned char)*parser->text) || *parser->text == '_') {
            if (length + 1 >= sizeof(name)) {
                parser->valid = false;
                return 0;
            }
            name[length++] = *parser->text++;
        }
        name[length] = '\0';

        if (strcmp(name, "defined") == 0) {
            skip_expression_spaces(parser);
            bool parentheses = *parser->text == '(';
            if (parentheses)
                parser->text++;
            skip_expression_spaces(parser);
            if (!isalpha((unsigned char)*parser->text) && *parser->text != '_') {
                parser->valid = false;
                return 0;
            }
            length = 0;
            while (isalnum((unsigned char)*parser->text) || *parser->text == '_') {
                if (length + 1 >= sizeof(name)) {
                    parser->valid = false;
                    return 0;
                }
                name[length++] = *parser->text++;
            }
            name[length] = '\0';
            skip_expression_spaces(parser);
            if (parentheses) {
                if (*parser->text != ')') {
                    parser->valid = false;
                    return 0;
                }
                parser->text++;
            }
            return find_define_value(name) != NULL;
        }

        const char *value = find_define_value(name);
        if (value == NULL) {
            for (size_t i = 0; i < expression_label_count; i++) {
                if (strcmp(expression_labels[i].name, name) == 0)
                    return expression_labels[i].address;
            }
            if (!parser->undefined_is_zero)
                parser->valid = false;
            return 0;
        }
        if (*value == '\0')
            return 1;

        char *end;
        errno = 0;
        long long parsed = strtoll(value, &end, 0);
        if (errno == ERANGE || end == value || *trim(end) != '\0') {
            parser->valid = false;
            return 0;
        }
        return parsed;
    }

    errno = 0;
    char *end;
    long long value = strtoll(parser->text, &end, 0);
    if (errno == ERANGE || end == parser->text) {
        parser->valid = false;
        return 0;
    }
    parser->text = end;
    return value;
}

static long long parse_expression_unary(ExpressionParser *parser) {
    skip_expression_spaces(parser);
    if (*parser->text == '!') {
        parser->text++;
        return !parse_expression_unary(parser);
    }
    if (*parser->text == '-') {
        parser->text++;
        return -parse_expression_unary(parser);
    }
    if (*parser->text == '+') {
        parser->text++;
        return parse_expression_unary(parser);
    }
    if (*parser->text == '~') {
        parser->text++;
        return ~parse_expression_unary(parser);
    }
    return parse_expression_primary(parser);
}

static long long parse_expression_product(ExpressionParser *parser) {
    long long value = parse_expression_unary(parser);
    while (parser->valid) {
        skip_expression_spaces(parser);
        char operation = *parser->text;
        if (operation != '*' && operation != '/' && operation != '%')
            break;
        parser->text++;
        long long right = parse_expression_unary(parser);
        if ((operation == '/' || operation == '%') && right == 0) {
            parser->valid = false;
            return 0;
        }
        if (operation == '*') value *= right;
        if (operation == '/') value /= right;
        if (operation == '%') value %= right;
    }
    return value;
}

static long long parse_expression_sum(ExpressionParser *parser) {
    long long value = parse_expression_product(parser);
    while (parser->valid) {
        skip_expression_spaces(parser);
        char operation = *parser->text;
        if (operation != '+' && operation != '-')
            break;
        parser->text++;
        long long right = parse_expression_product(parser);
        value = operation == '+' ? value + right : value - right;
    }
    return value;
}

static long long parse_expression_shift(ExpressionParser *parser) {
    long long value = parse_expression_sum(parser);
    while (parser->valid) {
        skip_expression_spaces(parser);
        bool left = strncmp(parser->text, "<<", 2) == 0;
        bool right = strncmp(parser->text, ">>", 2) == 0;
        if (!left && !right)
            break;
        parser->text += 2;
        long long amount = parse_expression_sum(parser);
        if (amount < 0 || amount >= (long long)(sizeof(value) * CHAR_BIT)) {
            parser->valid = false;
            return 0;
        }
        value = left ? value << amount : value >> amount;
    }
    return value;
}

static long long parse_expression_relation(ExpressionParser *parser) {
    long long value = parse_expression_shift(parser);
    skip_expression_spaces(parser);
    if (strncmp(parser->text, "<=", 2) == 0) {
        parser->text += 2;
        return value <= parse_expression_shift(parser);
    }
    if (strncmp(parser->text, ">=", 2) == 0) {
        parser->text += 2;
        return value >= parse_expression_shift(parser);
    }
    if (*parser->text == '<' || *parser->text == '>') {
        char operation = *parser->text++;
        return operation == '<' ? value < parse_expression_shift(parser)
                                : value > parse_expression_shift(parser);
    }
    return value;
}

static long long parse_expression_equality(ExpressionParser *parser) {
    long long value = parse_expression_relation(parser);
    skip_expression_spaces(parser);
    if (strncmp(parser->text, "==", 2) == 0) {
        parser->text += 2;
        return value == parse_expression_relation(parser);
    }
    if (strncmp(parser->text, "!=", 2) == 0) {
        parser->text += 2;
        return value != parse_expression_relation(parser);
    }
    return value;
}

static long long parse_expression_bitwise_and(ExpressionParser *parser) {
    long long value = parse_expression_equality(parser);
    while (parser->valid) {
        skip_expression_spaces(parser);
        if (*parser->text != '&' || parser->text[1] == '&')
            break;
        parser->text++;
        value &= parse_expression_equality(parser);
    }
    return value;
}

static long long parse_expression_bitwise_xor(ExpressionParser *parser) {
    long long value = parse_expression_bitwise_and(parser);
    while (parser->valid) {
        skip_expression_spaces(parser);
        if (*parser->text != '^')
            break;
        parser->text++;
        value ^= parse_expression_bitwise_and(parser);
    }
    return value;
}

static long long parse_expression_bitwise_or(ExpressionParser *parser) {
    long long value = parse_expression_bitwise_xor(parser);
    while (parser->valid) {
        skip_expression_spaces(parser);
        if (*parser->text != '|' || parser->text[1] == '|')
            break;
        parser->text++;
        value |= parse_expression_bitwise_xor(parser);
    }
    return value;
}

static long long parse_expression_and(ExpressionParser *parser) {
    long long value = parse_expression_bitwise_or(parser);
    while (parser->valid) {
        skip_expression_spaces(parser);
        if (strncmp(parser->text, "&&", 2) != 0)
            break;
        parser->text += 2;
        value = value && parse_expression_bitwise_or(parser);
    }
    return value;
}

static long long parse_expression_or(ExpressionParser *parser) {
    long long value = parse_expression_and(parser);
    while (parser->valid) {
        skip_expression_spaces(parser);
        if (strncmp(parser->text, "||", 2) != 0)
            break;
        parser->text += 2;
        value = value || parse_expression_and(parser);
    }
    return value;
}

static bool evaluate_expression_mode(char *text, long long *result,
                                     bool undefined_is_zero) {
    ExpressionParser parser = {trim(text), true, undefined_is_zero};
    long long value = parse_expression_or(&parser);
    skip_expression_spaces(&parser);
    if (!parser.valid || *parser.text != '\0') {
        return false;
    }
    *result = value;
    return true;
}

static bool evaluate_expression(char *text, long long *result) {
    return evaluate_expression_mode(text, result, true);
}

static bool evaluate_numeric_expression(char *text, long long *result) {
    return evaluate_expression_mode(text, result, false);
}

static bool evaluate_condition(char *text, bool *result) {
    long long value;
    if (!evaluate_expression(text, &value)) {
        fprintf(stderr, "Error: invalid #if expression\n");
        return false;
    }
    *result = value != 0;
    return true;
}

static bool conditional_is_active(const ConditionalState *state) {
    return state->depth == 0 || state->blocks[state->depth - 1].active;
}

static bool process_conditional_directive(char *line, ConditionalState *state) {
    if (is_directive(line, "#if")) {
        if (state->depth >= MAX_CONDITIONAL_DEPTH) {
            fprintf(stderr, "Error: maximum conditional nesting depth reached\n");
            return false;
        }
        bool parent_active = conditional_is_active(state);
        bool condition = false;
        if (parent_active && !evaluate_condition(line + 3, &condition))
            return false;
        ConditionalBlock *block = &state->blocks[state->depth++];
        block->parent_active = parent_active;
        block->branch_taken = condition;
        block->active = block->parent_active && condition;
        block->else_seen = false;
        return true;
    }
    if (is_directive(line, "#elif")) {
        if (state->depth == 0 || state->blocks[state->depth - 1].else_seen) {
            fprintf(stderr, "Error: #elif without a matching #if\n");
            return false;
        }
        ConditionalBlock *block = &state->blocks[state->depth - 1];
        bool condition = false;
        if (block->parent_active && !block->branch_taken &&
            !evaluate_condition(line + 5, &condition))
            return false;
        block->active = block->parent_active && !block->branch_taken && condition;
        block->branch_taken = block->branch_taken || condition;
        return true;
    }
    if (is_directive(line, "#else")) {
        if (state->depth == 0 || state->blocks[state->depth - 1].else_seen ||
            *trim(line + 5) != '\0') {
            fprintf(stderr, "Error: invalid #else directive\n");
            return false;
        }
        ConditionalBlock *block = &state->blocks[state->depth - 1];
        block->active = block->parent_active && !block->branch_taken;
        block->branch_taken = true;
        block->else_seen = true;
        return true;
    }
    if (is_directive(line, "#endif")) {
        if (state->depth == 0 || *trim(line + 6) != '\0') {
            fprintf(stderr, "Error: invalid #endif directive\n");
            return false;
        }
        state->depth--;
        return true;
    }
    return false;
}

static bool is_conditional_directive(char *line) {
    return is_directive(line, "#if") || is_directive(line, "#elif") ||
           is_directive(line, "#else") || is_directive(line, "#endif");
}

bool is_include_directive(char *line) {
    return strncmp(line, "#include", 8) == 0 &&
           (line[8] == '\0' || isspace((unsigned char)line[8]));
}

bool parse_include_directive(char *line, char *filename) {
    char *text = trim(line + 8);

    if (*text != '"') {
        fprintf(stderr, "Error: #include expects a quoted filename\n");
        return false;
    }

    text++;

    char *end = strchr(text, '"');

    if (end == NULL) {
        fprintf(stderr, "Error: missing closing quote in #include\n");
        return false;
    }

    *end = '\0';
    strcpy(filename, text);

    if (*trim(end + 1) != '\0') {
        fprintf(stderr, "Error: unexpected text after #include\n");
        return false;
    }

    return true;
}

bool is_undefine_directive(char *line) {
    return strncmp(line, "#undefine", 9) == 0 &&
           (line[9] == '\0' || isspace((unsigned char)line[9]));
}

bool parse_undefine_directive(char *line) {
    char *name = trim(line + 9);
    if (name[0] == '\0' || !isalpha((unsigned char)name[0])) {
        fprintf(stderr, "Error: #undefine expects a symbol\n");
        return false;
    }

    char *end = name;
    while (isalnum((unsigned char)*end) || *end == '_')
        end++;
    if (*trim(end) != '\0') {
        fprintf(stderr, "Error: invalid symbol in #undefine\n");
        return false;
    }

    *end = '\0';
    for (size_t i = 0; i < define_count; i++) {
        if (strcmp(defines[i].name, name) == 0) {
            memmove(&defines[i], &defines[i + 1],
                    (define_count - i - 1) * sizeof(defines[0]));
            define_count--;
            break;
        }
    }
    return true;
}

bool is_error_directive(char *line) {
    return strncmp(line, "#error", 6) == 0 &&
           (line[6] == '\0' || isspace((unsigned char)line[6]));
}

bool parse_error_directive(char *line) {
    char *message = trim(line + 6);
    if (*message != '"') {
        fprintf(stderr, "Error: #error expects a quoted message\n");
        return false;
    }

    message++;
    char *end = strchr(message, '"');
    if (end == NULL || *trim(end + 1) != '\0') {
        fprintf(stderr, "Error: invalid #error directive\n");
        return false;
    }

    *end = '\0';
    fprintf(stderr, "Error: %s\n", message);
    return false;
}

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

    long long parsed;
    if (!evaluate_numeric_expression(text, &parsed)) {
        fprintf(stderr, "Error: invalid .word value\n");
        return false;
    }

    *value = (Z33_Word)parsed;
    return true;
}

bool is_space_directive(char *line) {
    return strncmp(line, ".space", 6) == 0 &&
           (line[6] == '\0' || isspace((unsigned char)line[6]));
}

bool parse_space_directive(char *line, size_t *count) {
    long long value;
    if (!evaluate_numeric_expression(line + 6, &value) || value < 0 ||
        (unsigned long long)value > Z33_MEMORY_SIZE) {
        fprintf(stderr, "Error: .space expects a non-negative cell count\n");
        return false;
    }

    *count = (size_t)value;
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

    long long value;
    if (!evaluate_numeric_expression(value_text, &value)) {
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

    *length = count + 1; /* terminating null cell */
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

/* Remove // comments without treating // inside a string literal as a comment. */
static void strip_comment(char *line) {
    bool escaped = false;
    bool in_string = false;
    for (char *p = line; *p != '\0'; p++) {
        if (in_string && escaped) {
            escaped = false;
        } else if (in_string && *p == '\\') {
            escaped = true;
        } else if (*p == '"') {
            in_string = !in_string;
        } else if (!in_string && p[0] == '/' && p[1] == '/') {
            *p = '\0';
            return;
        }
    }
}

bool isLabel(char * text){
    trim(text);
    if(text[0] != '\0' && text[strlen(text)-1]==':'){
        if(strlen(text)<LENGTH_MAX_LABEL){
            if(isalpha((unsigned char)text[0]) || text[0] == '_')
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

    if (*address >= Z33_MEMORY_SIZE ||
        !write_Word_to_memory(&machine->memory, 0, *address)) {
        fprintf(stderr, "Error: string exceeds memory size\n");
        return false;
    }
    (*address)++;

    return true;
}

static bool resolve_include_path(const char *including_file, const char *included_file,
                                 char *path, size_t path_size) {
    if (included_file[0] == '/')
        return snprintf(path, path_size, "%s", included_file) < (int)path_size;

    const char *slash = strrchr(including_file, '/');
    if (slash == NULL)
        return snprintf(path, path_size, "%s", included_file) < (int)path_size;

    return snprintf(path, path_size, "%.*s/%s", (int)(slash - including_file),
                    including_file, included_file) < (int)path_size;
}

static bool add_label(Z33_Label *labels, size_t *label_count, char *name,
                      Z33_Address address) {
    name = trim(name);
    if (name[0] == '\0' ||
        (!isalpha((unsigned char)name[0]) && name[0] != '_')) {
        fprintf(stderr, "Error: label must start with a letter or underscore\n");
        return false;
    }
    for (char *p = name + 1; *p != '\0'; p++) {
        if (!isalnum((unsigned char)*p) && *p != '_') {
            fprintf(stderr, "Error: invalid character in label '%s'\n", name);
            return false;
        }
    }
    if (strlen(name) >= LENGTH_MAX_LABEL) {
        fprintf(stderr, "Error: label cannot exceed 256 characters\n");
        return false;
    }
    if (*label_count >= MAX_LABELS) {
        fprintf(stderr, "Error: maximum number of labels reached\n");
        return false;
    }
    for (size_t i = 0; i < *label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            fprintf(stderr, "Error: label '%s' already defined\n", name);
            return false;
        }
    }
    strcpy(labels[*label_count].name, name);
    labels[*label_count].address = address;
    (*label_count)++;
    return true;
}

static char *extract_inline_label(char *line, char **label) {
    char *end = line;
    while (*end != '\0' && !isspace((unsigned char)*end))
        end++;
    if (end == line || end[-1] != ':')
        return line;

    end[-1] = '\0';
    *label = line;
    return trim(end);
}

static bool collect_labels_from_file(const char *filename, Z33_Label *labels,
                                     size_t *label_count, Z33_Address *address,
                                     int *n_line, ConditionalState *conditions,
                                     unsigned int depth) {
    if (depth >= MAX_INCLUDE_DEPTH) {
        fprintf(stderr, "Error: maximum #include depth reached\n");
        return false;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file '%s'\n", filename);
        return false;
    }

    char line[LENGTH_MAX_LINE];
    while (fgets(line, sizeof(line), file) != NULL) {
        (*n_line)++;
        line[strcspn(line, "\r\n")] = '\0';
        strip_comment(line);
        char *current = trim(line);

        if (current[0] == '\0')
            continue;
        if (is_conditional_directive(current)) {
            if (!process_conditional_directive(current, conditions)) {
                fclose(file);
                return false;
            }
            continue;
        }
        if (!conditional_is_active(conditions))
            continue;
        while (true) {
            char *label = NULL;
            char *after_label = extract_inline_label(current, &label);
            if (label == NULL)
                break;
            if (!add_label(labels, label_count, label, *address)) {
                fclose(file);
                return false;
            }
            expression_label_count = *label_count;
            current = after_label;
        }
        if (current[0] == '\0')
            continue;
        if (is_error_directive(current)) {
            parse_error_directive(current);
            fclose(file);
            return false;
        }
        if (is_include_directive(current)) {
            char included_file[LENGTH_MAX_PATH];
            char included_path[LENGTH_MAX_PATH];
            if (!parse_include_directive(current, included_file) ||
                !resolve_include_path(filename, included_file, included_path,
                                      sizeof(included_path)) ||
                !collect_labels_from_file(included_path, labels, label_count, address,
                                          n_line, conditions, depth + 1)) {
                fclose(file);
                return false;
            }
            continue;
        }
        if (is_word_directive(current)) {
            if (*address >= Z33_MEMORY_SIZE) {
                fprintf(stderr, "Error: .word exceeds memory\n");
                fclose(file);
                return false;
            }
            (*address)++;
            continue;
        }
        if (is_define_directive(current)) {
            if (!parse_define_directive(current)) {
                fprintf(stderr, "Error parsing line %d: %s\n", *n_line, current);
                fclose(file);
                return false;
            }
            continue;
        }
        if (is_undefine_directive(current)) {
            if (!parse_undefine_directive(current)) {
                fprintf(stderr, "Error parsing line %d: %s\n", *n_line, current);
                fclose(file);
                return false;
            }
            continue;
        }
        if (!replace_defines(current)) {
            fclose(file);
            return false;
        }
        if (is_space_directive(current)) {
            size_t count;
            if (!parse_space_directive(current, &count)) {
                fclose(file);
                return false;
            }
            if ((size_t)*address + count > Z33_MEMORY_SIZE) {
                fprintf(stderr, "Error: .space exceeds memory size\n");
                fclose(file);
                return false;
            }
            *address += (Z33_Address)count;
            continue;
        }
        if (is_addr_directive(current)) {
            if (!parse_addr_directive(current, address)) {
                fprintf(stderr, "Error parsing line %d: %s\n", *n_line, current);
                fclose(file);
                return false;
            }
            continue;
        }
        if (is_string_directive(current)) {
            size_t length;
            if (!get_string_length(current, &length) ||
                (size_t)*address + length > Z33_MEMORY_SIZE) {
                fprintf(stderr, "Error parsing line %d: %s\n", *n_line, current);
                fclose(file);
                return false;
            }
            *address += (Z33_Address)length;
            continue;
        }
        if (isLabel(current)) {
            current[strlen(current) - 1] = '\0';
            if (!add_label(labels, label_count, current, *address)) {
                fclose(file);
                return false;
            }
            continue;
        }
        if (*address >= Z33_MEMORY_SIZE) {
            fprintf(stderr, "Error: program exceeds memory size\n");
            fclose(file);
            return false;
        }
        (*address)++;
    }
    fclose(file);
    return true;
}

static bool load_file_contents(Z33_Machine *machine, const char *filename,
                               const Z33_Label *labels, size_t label_count,
                               Z33_Address *address, int *n_line,
                               ConditionalState *conditions, unsigned int depth) {
    if (depth >= MAX_INCLUDE_DEPTH) {
        fprintf(stderr, "Error: maximum #include depth reached\n");
        return false;
    }
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file '%s'\n", filename);
        return false;
    }

    char line[LENGTH_MAX_LINE];
    while (fgets(line, sizeof(line), file) != NULL) {
        (*n_line)++;
        line[strcspn(line, "\r\n")] = '\0';
        strip_comment(line);
        char *current = trim(line);
        if (current[0] == '\0')
            continue;
        if (is_conditional_directive(current)) {
            if (!process_conditional_directive(current, conditions)) {
                fclose(file);
                return false;
            }
            continue;
        }
        if (!conditional_is_active(conditions))
            continue;
        while (true) {
            char *label = NULL;
            char *after_label = extract_inline_label(current, &label);
            if (label == NULL)
                break;
            current = after_label;
        }
        if (current[0] == '\0')
            continue;
        if (is_error_directive(current)) {
            parse_error_directive(current);
            fclose(file);
            return false;
        }
        if (is_include_directive(current)) {
            char included_file[LENGTH_MAX_PATH];
            char included_path[LENGTH_MAX_PATH];
            if (!parse_include_directive(current, included_file) ||
                !resolve_include_path(filename, included_file, included_path,
                                      sizeof(included_path)) ||
                !load_file_contents(machine, included_path, labels, label_count,
                                    address, n_line, conditions, depth + 1)) {
                fclose(file);
                return false;
            }
            continue;
        }
        if (is_word_directive(current)) {
            Z33_Word value;
            if (!parse_word_directive(current, &value) ||
                !write_Word_to_memory(&machine->memory, value, *address)) {
                fclose(file);
                return false;
            }
            (*address)++;
            continue;
        }
        if (is_define_directive(current)) {
            if (!parse_define_directive(current)) {
                fprintf(stderr, "Error parsing line %d: %s\n", *n_line, current);
                fclose(file);
                return false;
            }
            continue;
        }
        if (is_undefine_directive(current)) {
            if (!parse_undefine_directive(current)) {
                fprintf(stderr, "Error parsing line %d: %s\n", *n_line, current);
                fclose(file);
                return false;
            }
            continue;
        }
        if (!replace_defines(current)) {
            fclose(file);
            return false;
        }
        if (is_space_directive(current)) {
            size_t count;
            if (!parse_space_directive(current, &count)) {
                fclose(file);
                return false;
            }
            if ((size_t)*address + count > Z33_MEMORY_SIZE) {
                fprintf(stderr, "Error: .space exceeds memory size\n");
                fclose(file);
                return false;
            }
            for (size_t i = 0; i < count; i++)
                machine->memory.cells[(size_t)*address + i].type = Cell_Empty;
            *address += (Z33_Address)count;
            continue;
        }
        if (is_addr_directive(current)) {
            if (!parse_addr_directive(current, address)) {
                fprintf(stderr, "Error parsing line %d: %s\n", *n_line, current);
                fclose(file);
                return false;
            }
            continue;
        }
        if (is_string_directive(current)) {
            if (!write_string_directive(machine, current, address)) {
                fprintf(stderr, "Error parsing line %d: %s\n", *n_line, current);
                fclose(file);
                return false;
            }
            continue;
        }
        if (isLabel(current))
            continue;
        if (*address >= Z33_MEMORY_SIZE) {
            fprintf(stderr, "Error: program exceeds memory size\n");
            fclose(file);
            return false;
        }
        fprintf(stderr, "%d  %s\n", *n_line, current);
        fflush(stdout);
        Z33_Instruction inst;
        if (!parse_line(machine, current, &inst, labels, label_count)) {
            fprintf(stderr, "Error parsing line %d: %s\n", *n_line, current);
            fclose(file);
            return false;
        }
        inst.line = *n_line;
        if (!write_Instruction_to_memory(&machine->memory, inst, *address)) {
            fclose(file);
            return false;
        }
        (*address)++;
    }
    fclose(file);
    return true;
}

bool parse_file(Z33_Machine *machine, char *filename) {
    Z33_Label labels[MAX_LABELS];
    size_t label_count = 0;
    Z33_Address address = 1000;
    int n_line = 0;
    ConditionalState conditions = {0};

    define_count = 0;
    expression_labels = labels;
    expression_label_count = 0;
    if (!collect_labels_from_file(filename, labels, &label_count, &address, &n_line,
                                  &conditions, 0))
        return false;
    if (conditions.depth != 0) {
        fprintf(stderr, "Error: missing #endif\n");
        return false;
    }

    address = 1000;
    n_line = 0;
    define_count = 0;
    expression_labels = labels;
    expression_label_count = label_count;
    memset(&conditions, 0, sizeof(conditions));
    if (!load_file_contents(machine, filename, labels, label_count, &address, &n_line,
                            &conditions, 0))
        return false;
    if (conditions.depth != 0) {
        fprintf(stderr, "Error: missing #endif\n");
        return false;
    }
    return true;
}

bool verify_opcode(char *line, Z33_Instruction *inst)
{
    char mnemonic[LENGTH_MAX_OPCODE];

    if (sscanf(line, "%9s", mnemonic) != 1)
        return false;

    to_lowercase(mnemonic);
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


static void to_lowercase(char *str)
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
    long long value;
    if (evaluate_numeric_expression(text, &value)) {
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
        long long address;
        if (evaluate_numeric_expression(text, &address)) {

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

            long long offset;
            if (!evaluate_numeric_expression(offset_text, &offset)) {
                fprintf(stderr, "Error: invalid indexed offset\n");
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
