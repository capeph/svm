#include <_string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/_types/_u_int32_t.h>
#include "hashmap.h"
#include "opcodes.h"
#include "assembler.h"

typedef struct {
    char *name;
    int opcode;
    uint32_t (*emitter)(char *, int, int);
} Builder;

// gives offset of first non-space in str
int nonspace(char *str, int start) {
    int i = start;
    while (str[i] == ' ' || str[i] == '\t') {
        i++;
    }
    if (str[i] == 0) {
        return -1;
    }
    return i;
}

int find(char *str, char target, int start) {
    int i = start;
    while (str[i] != target) {
        if (str[i] == 0) {
            return -1;
        }
        i++;
    }
    return i;
}

int space(char *str, int start) {
    int i = start;
    while (str[i] != ' ' && str[i] != '\t') {
        if (str[i] == 0) {
            return -1;
        }
        i++;
    }
    return i;
}

bool match(char *match, char *str, int start, int end)
{
    int matchlen = strlen(match);
    if (matchlen != end-start) {
        return false;
    }
    for(int i = 0; i < matchlen; i++) {
        if (match[i] != str[start+i]) {
            return false;
        }
    }
    return true;
}

int get_register(char *str, int start) {
    if (start == -1) {
        return -1;
    }
    int reg_start = nonspace(str, start);
    bool reference = str[reg_start] == '[';
    int reg_end = reg_start;
    if (reference) {
        reg_start++;
        reg_end = find(str, ']', reg_start);
    }
    else {
        reg_end = find(str, ',', reg_start);
        if (reg_end == -1) {
            reg_end = space(str, reg_start);
        }
        if (reg_end == -1) {
            reg_end = strlen(str);
        }
    }
    if (str[reg_start] != 'R') {
        return -1;
    }
    reg_start++;
    long num = strtol(str+reg_start, NULL, 10);
    if (reference) {
        return num | 0x80;
    }
    return num;
}

int get_cond(char *line)
{
    int len = strlen(line);
    int startpos = -1;
    for (int i = len; i > 5; i--) {
        if (strncmp(line + i - 3, " IF", 3)) {
            startpos = i;
        }
    }
    if (startpos == -1) {
        return 0;
    }
    int flags = 0;
    for (int i = startpos; i < len; i++)
    {
        if (line[i] == 'Z') {
            flags |= 1;
        }
        else if (line[i] == 'N') {
            flags |= 2;
        }
        else if (line[i] == 'P') {
            flags |= 4;
        }
        else {
            break;
        }
    }
    return flags;
}

void add_instr(HashMap *map, char *name,
    int opcode, uint32_t (*emitter)(char *, int, int)) {
    Builder *builder = malloc(sizeof(Builder));
    builder->name = name;
    builder->opcode = opcode;
    builder->emitter = emitter;
    put_map(map, name, builder);
}

uint32_t op_reg_reg(char *line, int op_end, int opcode) {
    int reg1 = get_register(line, op_end);
    int comma = find(line, ',', op_end);
    int reg2 = get_register(line, comma + 1);
    int cond = get_cond(line);
    return opcode << 22 | cond << 16 | reg1 << 8 | reg2;
}

uint32_t op_reg_const(char *line, int op_end, int opcode) {
        int reg1 = get_register(line, op_end);
        int cond = get_cond(line);
        return LOADHALF << 22 | cond << 16 | reg1 << 8;
}


HashMap *create_instructions() {
    HashMap *map = create_map(256);
    add_instr(map, "LOAD", LOAD, &op_reg_reg);
    add_instr(map, "LOADHALF", LOADHALF, &op_reg_const);
    add_instr(map, "LOADCONST", LOADCONST, &op_reg_const);
    add_instr(map, "ADD", ADD, &op_reg_reg);
    add_instr(map, "SUBTRACT", SUBTRACT, &op_reg_reg);
    return map;
}

void destroy_instructions(HashMap *map) {
    for(int i = 0; i < map->size; i++) {
        Node *node = map->entries[i];
        while (node != NULL) {
            Node *next = node->next;
            Builder *builder = node->data;
            free(builder);
            free(node);
            node = next;
        };
    }

}


uint32_t assemble(HashMap *codes, char *line) {
    char opcode[25];
    int op_start = nonspace(line, 0);
    int op_end = space(line, op_start);
    memcpy(opcode, line + op_start, op_end);
    opcode[op_end - op_start] = '\0';
    Builder *builder = get_map(codes, opcode);
    if (builder == NULL) {
        printf("Unknown opcode %s\n", opcode);
    }
    return builder->emitter(line, op_end, builder->opcode);
}
