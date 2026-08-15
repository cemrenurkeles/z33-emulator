#include "../include/parser.h"

bool parse_file( Z33_Machine *machine, const char *filename){
    FILE *file = fopen(filename,"r");
    if(file==NULL){
        fprintf(stderr,"Error opening the file\n");
        return false;
    }
    char line[LENGTH_MAX_LINE];
    while (fgets(line, sizeof(line),file)!=NULL){
        Z33_Instruction inst;
        if(parse_line(line,&inst)==false){
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

    sscanf(line, "%9s", mnemonic);

    if (strcmp(mnemonic, "ld") == 0) {
        inst->opcode = OP_LD;
        inst->n_op = 2;
        return true;
    }

    // Add other opcodes later

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

char * cut_2_operands(char * text){
    char *second = strchr(text,',');
    if(second==NULL) return NULL;
    
    *second='\0';
    second++;
    
    return trim(second);
}

bool parse_line(char *line, Z33_Instruction *instruction){
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
            if(parse_operand(operandes,&op1)==true){
                if(parse_operand(second,&op2)==true){
                    instruction->op[0]=op1;
                    instruction->op[1]=op2;
                    return true;
                }
            }   
        }
    }
    
    return false;
}

bool parse_operand(char *text,Z33_Operand *operand){
    char *end;
    long long imm_value = strtoll(text, &end, 10);
    if (end!=text && *end == '\0') {
        operand->type = OPERAND_IMM;
        operand->value.immediate = (Z33_Word)imm_value;
        return true;
    }
    if(text[0]=='%'){
        if(parse_register(text,operand)) return true;
    }
    return false;
}

bool parse_register(char *text,Z33_Operand *operand){
    if(strcmp(text,"%a")==0){
        operand->type=OPERAND_REG;
        operand->value.reg=REG_A;
        return true;
    }
    return false;
}
