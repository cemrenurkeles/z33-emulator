#include "../include/parser.h"
#include "../include/exception.h"

static const Z33_OpcodeEntry opcode_table[] = {
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

bool parse_file( Z33_Machine *machine, const char *filename){
    FILE *file = fopen(filename,"r");
    if(file==NULL){
        fprintf(stderr,"Error opening the file\n");
        return false;
    }
    char line[LENGTH_MAX_LINE];
    while (fgets(line, sizeof(line),file)!=NULL){
        Z33_Instruction inst;
        if(parse_line(machine,line,&inst)==false){
            fclose(file);
            fprintf(stderr,"Error parsing the line\n");
            return false;
        };
        z33_execute(machine,&inst);
        // Écrire à la mémoire manquant, sauté pour un test rapide
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
    return false;

}

char * remove_words_separated_space(const char *line ){
    char * operands = strchr(line,' ');
    if(operands==NULL) return NULL;
    while(*operands ==' ') operands++;
    return operands;
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

bool parse_line(Z33_Machine *machine,char *line, Z33_Instruction *instruction){
    instruction->opcode=OP_INVALID;
    instruction->n_op=0;

    to_lowercase(line);
    if(verify_opcode(line,instruction)==true){
        if(instruction->n_op==0) return true;
        char * operandes = remove_words_separated_space(line);
        if(operandes!=NULL&&instruction->n_op==2){
            Z33_Operand op1, op2;
            char * second = cut_2_operands(operandes);
            if(second==NULL){
                fprintf(stderr,"parse_line : Error operand number incorrect");
                return false;
            }
            operandes = trim(operandes);
            if(parse_operand(machine,operandes,&op1)==true){
                if(parse_operand(machine,second,&op2)==true){
                    instruction->op[0]=op1;
                    instruction->op[1]=op2;
                    return true;
                }
            }   
        }
    }
    instruction->opcode=OP_INVALID;
    instruction->n_op=0;

    return false;
}

bool is_Immediate (char * text){
    char *end;
    strtoll(text, &end, 10);
    if (end!=text && *end == '\0') return true;
    return false;
}
bool parse_operand(Z33_Machine *machine,char *text, Z33_Operand *operand){
    if (text == NULL || text[0] == '\0')
        return false;

    if(is_Immediate(text)){
        operand->type = OPERAND_IMM;
        long long value_imm = strtoll(text,NULL,10);
        if (value_imm>UINT32_MAX) {
            Z33_Exception exception = EX_INT_OUT_OF_RANGE;
            z33_raise_exception(machine,exception);
        }
        operand->value.immediate = (Z33_Word)strtoll(text,NULL,10);
        return true;
    }

    if(text[0]=='%'){
        if(parse_register(text,operand))
            return true;
        else
            return false;
    }

    if(text[0]=='['){
        if(text[strlen(text)-1]==']'){
            text = text + 1;
            text[strlen(text)-1] = '\0';

            if(is_Immediate(text)){
                operand->type = OPERAND_IMM;
                long long value_imm = strtoll(text,NULL,10);
                if (value_imm>UINT32_MAX) {
                    Z33_Exception exception = EX_INT_OUT_OF_RANGE;
                    z33_raise_exception(machine,exception);
                }
                operand->type = OPERAND_DIR;
                operand->value.address =
                    (Z33_Address)strtoll(text,NULL,10);
                return true;
            }

            char *plus = strchr(text,'+');
            char *minus = strchr(text,'-');
            char *sign = plus != NULL ? plus : minus;

            if(sign != NULL){
                char sign_char = *sign;
                *sign = '\0';

                char *reg_text = trim(text);
                char *offset_text = trim(sign + 1);

                Z33_Operand reg_operand;

                if(!parse_register(reg_text, &reg_operand))
                    return false;

                if(!is_Immediate(offset_text))
                    return false;

                Z33_Word offset =
                    (Z33_Word)strtoll(offset_text,NULL,10);

                if(sign_char == '-')
                    offset = -offset;

                operand->type = OPERAND_IDX;
                operand->value.indexed.reg =
                    reg_operand.value.reg;
                operand->value.indexed.offset =
                    (int32_t)offset;

                return true;
            }

            Z33_Operand reg_operand;

            if(parse_register(text, &reg_operand)){
                operand->type = OPERAND_IND;
                operand->value.reg =
                    reg_operand.value.reg;

                return true;
            }

            return false;
        }
        else{
            fprintf(stderr,"Error missing ']'\n");
            return false;
        }
    }

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
