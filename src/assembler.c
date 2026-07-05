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
    uint32_t (*emitter)(char *, int, int, uint32_t *);
} Builder;


// gives offset of first non-space in str
// returns -1 if no nonspace found
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


int read_register(char *str, int start) {
    if (start == -1) {
        printf("assembly failed for %s: no register - invalid start offset\n", str);
        exit(-1);
    }
    int reg_start = nonspace(str, start);
    bool reference = str[reg_start] == '[';
    if (reference) {
        reg_start++;
    }
    if (str[reg_start] != 'R') {
        printf("assembly failed for %s: expected register\n", str);
        exit(-1);
    }
    reg_start++;
    long num = strtol(str+reg_start, NULL, 10);
    if (reference) {
        return num | 0x80;
    }
    return num;
}


uint16_t read_offset(char *str, int start) {
    if (start == -1) {
        printf("assembly failed for %s: expected offset at %d\n", str, start);
        exit(-1);
    }
    int offset_start = nonspace(str, start);
    long num = strtoul(str+offset_start, NULL, 10);
    return (uint16_t)num;
}

int read_cond(char *line)
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
    int opcode, uint32_t (*emitter)(char *, int, int, uint32_t *)) {
    Builder *builder = malloc(sizeof(Builder));
    builder->name = name;
    builder->opcode = opcode;
    builder->emitter = emitter;
    put_map(map, name, builder);
}

uint32_t op_no_reg(char *line, int op_end, int opcode, uint32_t *dest) {
    int cond = read_cond(line);
    *dest = opcode << 22 | cond << 16;
    return 1;
}


uint32_t op_with_offset(char *line, int op_end, int opcode, uint32_t *dest) {
    uint16_t offset = read_offset(line, op_end);
    int cond = read_cond(line);
    *dest = opcode << 22 | cond << 16 | offset;
    return 1;
}

uint32_t op_one_reg(char *line, int op_end, int opcode, uint32_t *dest) {
    int reg1 = read_register(line, op_end);
    int cond = read_cond(line);
    *dest = opcode << 22 | cond << 16 | reg1 << 8;
    return 1;
}


uint32_t op_reg_reg(char *line, int op_end, int opcode, uint32_t *dest) {
    int reg1 = read_register(line, op_end);
    int comma = find(line, ',', op_end);
    int reg2 = read_register(line, comma + 1);
    int cond = read_cond(line);
//    printf("op_reg_reg %d %d %d: %s\n", opcode, reg1, reg2, line);
    *dest = opcode << 22 | cond << 16 | reg1 << 8 | reg2;
    return 1;
}


uint8_t read_const8(char *line, int offset) {
    char *const_start = line + nonspace(line, offset);
    char *endptr;
    uint8_t value = strtoul(const_start, &endptr, 10);
//        printf("reading from %s gives %d\n", const_start, value);
    return value;
}

uint32_t read_const32(char *line, int offset) {
    char *const_start = line + nonspace(line, offset);
    char *endptr;
    uint32_t value;
    if (const_start[0] == '-') {
        int32_t val = (int32_t)strtol(const_start, &endptr, 10);
        memcpy(&value, &val, sizeof(val));
    }
    else {
        value = strtoul(const_start, &endptr, 10);
//        printf("reading from %s gives %d\n", const_start, value);
    }
    return value;
}

uint64_t read_const64(char *line, int offset) {
    char *const_start = line + nonspace(line, offset);
    char *endptr;
    uint64_t value;
    if (const_start[0] == '-') {
        int64_t val = (int64_t)strtoll(const_start, &endptr, 10);
        memcpy(&value, &val, sizeof(val));
    }
    else {
        value = strtoull(const_start, &endptr, 10);
    }
    return value;
}


double read_const_double(char *line, int offset) {
    char *const_start = line + nonspace(line, offset);
    char *endptr;
    double value;
    value = strtod(const_start, &endptr);
//    printf("double %s, parsed to %lf\n", const_start, value);
    return value;
}

uint32_t op_reg_const(char *line, int op_end, int opcode, uint32_t *dest) {
        int reg1 = read_register(line, op_end);
        int comma = find(line, ',', op_end);
        int cond = read_cond(line);
        int size = GET_SIZE(opcode);
        *dest = opcode << 22 | cond << 16 | reg1 << 8;
        if (size == 2) {
            uint32_t value = read_const32(line, comma + 1);
//            printf("got size %d, value %d\n", size, value);
            *(dest + 1) = value;
        }
        else if (size == 3) {
            uint64_t value = read_const64(line, comma + 1);
//            printf("got size %d, value %lld\n", size, value);
            *((uint64_t *)(dest + 1)) = value;
        }
        return size;
}


uint32_t op_reg_float(char *line, int op_end, int opcode, uint32_t *dest) {
        int reg1 = read_register(line, op_end);
        int comma = find(line, ',', op_end);
        int cond = read_cond(line);
        int size = GET_SIZE(opcode);
        if (size != 3) {
            printf("Malformed opcode\n");
            exit(-1);
        }
        *dest = opcode << 22 | cond << 16 | reg1 << 8;
        double value = read_const_double(line, comma + 1);
//            printf("got size %d, value %f\n", size, value);
        *((double *)(dest + 1)) = value;

        return size;
}

// Nooo... we are wasting 16 bits / two registers
uint32_t op_const(char *line, int op_end, int opcode, uint32_t *dest) {
        int cond = read_cond(line);
        int size = GET_SIZE(opcode);
        int offset = nonspace(line, op_end);
        if (size == 1) {
            uint8_t value = read_const8(line, offset);
            *dest = opcode << 22 | cond << 16 | value;
        }
        else
        {
            *dest = opcode << 22 | cond << 16;
            if (size == 2) {
                uint32_t value = read_const32(line, offset);
//             printf("got size %d, value %d\n", size, value);
                *(dest + 1) = value;
            }
            else if (size == 3) {
                uint64_t value = read_const64(line, offset);
//             printf("got size %d, value %lld\n", size, value);
                *((uint64_t *)(dest + 1)) = value;
            }
        }
        return size;
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

HashMap *create_instructions() {
    HashMap *map = create_map(256);
    add_instr(map, "HALT", HALT, &op_no_reg);
    add_instr(map, "AHEAD", AHEAD, &op_with_offset);
    add_instr(map, "BACK", BACK, &op_with_offset);
    add_instr(map, "JUMP", JUMP, &op_one_reg);
    add_instr(map, "JUMP32", JUMP32, &op_const);
    add_instr(map, "JUMP64", JUMP64, &op_const);
    add_instr(map, "JUMPNEGATIVE", JUMPZERO, &op_reg_reg);
    add_instr(map, "JUMPZERO32", JUMPZERO32, &op_reg_const);
    add_instr(map, "JUMPZERO64", JUMPZERO64, &op_reg_const);
    add_instr(map, "JUMPNOTZERO", JUMPNOTZERO, &op_reg_reg);
    add_instr(map, "JUMPNOTZERO32", JUMPNOTZERO32, &op_reg_const);
    add_instr(map, "JUMPNOTZERO64", JUMPNOTZERO64, &op_reg_const);
    add_instr(map, "JUMPPOSITIVE", JUMPPOSITIVE, &op_reg_reg);
    add_instr(map, "JUMPPOSITIVE32", JUMPPOSITIVE32, &op_reg_const);
    add_instr(map, "JUMPPOSITIVE64", JUMPPOSITIVE64, &op_reg_const);
    add_instr(map, "JUMPNOTPOSITIVE", JUMPNOTPOSITIVE, &op_reg_reg);
    add_instr(map, "JUMPNOTPOSITIVE32", JUMPNOTPOSITIVE32, &op_reg_const);
    add_instr(map, "JUMPNOTPOSITIVE64", JUMPNOTPOSITIVE64, &op_reg_const);
    add_instr(map, "JUMPNEGATIVE", JUMPNEGATIVE, &op_reg_reg);
    add_instr(map, "JUMPNEGATIVE32", JUMPNEGATIVE32, &op_reg_const);
    add_instr(map, "JUMPNEGATIVE64", JUMPNEGATIVE64, &op_reg_const);
    add_instr(map, "JUMPNOTNEGATIVE", JUMPNOTNEGATIVE, &op_reg_reg);
    add_instr(map, "JUMPNOTNEGATIVE32", JUMPNOTNEGATIVE32, &op_reg_const);
    add_instr(map, "JUMPNOTNEGATIVE64", JUMPNOTNEGATIVE64, &op_reg_const);
    add_instr(map, "CALL", CALL, &op_reg_reg);
    add_instr(map, "CALL32", CALL32, &op_reg_const);
    add_instr(map, "CALL64", CALL64, &op_reg_const);
    add_instr(map, "LOAD", LOAD, &op_reg_reg);
    add_instr(map, "LOAD32", LOAD32, &op_reg_const);
    add_instr(map, "LOAD64", LOAD64, &op_reg_const);
    add_instr(map, "INTTOFLOAT", INTTOFLOAT, &op_reg_reg);
    add_instr(map, "FLOATTOINT", FLOATTOINT, &op_reg_reg);
    add_instr(map, "SHIFTLEFT", SHIFTLEFT, &op_reg_reg);
    add_instr(map, "SHIFTRIGHT", SHIFTRIGHT, &op_reg_reg);
    add_instr(map, "ROTLEFT", ROTLEFT, &op_reg_reg);
    add_instr(map, "ROTRIGHT", ROTRIGHT, &op_reg_reg);
    add_instr(map, "COMPARE", COMPARE, &op_reg_reg);
    add_instr(map, "LOADBYTE", LOADBYTE, &op_reg_reg);
    add_instr(map, "LOADCONSTBYTE", LOADCONSTBYTE, &op_reg_const);


    add_instr(map, "NEGATE", NEGATE, &op_reg_reg);
    add_instr(map, "ADD", ADD, &op_reg_reg);
    add_instr(map, "ADD32", ADD32, &op_reg_const);
    add_instr(map, "ADD64", ADD64, &op_reg_const);
    add_instr(map, "SUBTRACT", SUBTRACT, &op_reg_reg);
    add_instr(map, "SUBTRACT32", SUBTRACT32, &op_reg_const);
    add_instr(map, "SUBTRACT64", SUBTRACT64, &op_reg_const);
    add_instr(map, "MULTIPLY", MULTIPLY, &op_reg_reg);
    add_instr(map, "MULTIPLY32", MULTIPLY32, &op_reg_const);
    add_instr(map, "MULTIPLY64", MULTIPLY64, &op_reg_const);
    add_instr(map, "DIVIDE", SUBTRACT, &op_reg_reg);
    add_instr(map, "DIVIDE32", DIVIDE32, &op_reg_const);
    add_instr(map, "DIVIDE64", DIVIDE64, &op_reg_const);

    add_instr(map, "LOADFLOAT", LOADFLOAT, &op_reg_float);
    add_instr(map, "NEGATEFLOAT", NEGATEFLOAT, &op_reg_float);
    add_instr(map, "ADDFLOAT", ADDFLOAT, &op_reg_reg);
    add_instr(map, "ADDFLOAT64", ADDFLOAT64, &op_reg_float);
    add_instr(map, "SUBTRACTFLOAT", SUBTRACTFLOAT, &op_reg_reg);
    add_instr(map, "SUBTRACTFLOAT64", SUBTRACTFLOAT, &op_reg_float);
    add_instr(map, "MULTIPLYFLOAT", MULTIPLYFLOAT, &op_reg_reg);
    add_instr(map, "MULTIPLYFLOAT64", MULTIPLYFLOAT64, &op_reg_float);
    add_instr(map, "DIVIDEFLOAT", DIVIDEFLOAT, &op_reg_reg);
    add_instr(map, "DIVIDEFLOAT64", DIVIDEFLOAT64, &op_reg_float);

    add_instr(map, "NOT", NOT, &op_reg_reg);
    add_instr(map, "AND", AND, &op_reg_reg);
    add_instr(map, "AND32", AND32, &op_reg_const);
    add_instr(map, "AND64", AND64, &op_reg_const);
    add_instr(map, "NAND", NAND, &op_reg_reg);
    add_instr(map, "NAND32", NAND32, &op_reg_const);
    add_instr(map, "NAND64", NAND64, &op_reg_const);
    add_instr(map, "OR", OR, &op_reg_reg);
    add_instr(map, "OR32", OR32, &op_reg_const);
    add_instr(map, "OR64", OR64, &op_reg_const);
    add_instr(map, "NOR", NOR, &op_reg_reg);
    add_instr(map, "NOR32", NOR32, &op_reg_const);
    add_instr(map, "NOR64", NOR64, &op_reg_const);
    add_instr(map, "XOR", XOR, &op_reg_reg);
    add_instr(map, "XOR32", XOR32, &op_reg_const);
    add_instr(map, "XOR64", XOR64, &op_reg_const);
    add_instr(map, "XNOR", XNOR, &op_reg_reg);
    add_instr(map, "XNOR32", XNOR32, &op_reg_const);
    add_instr(map, "XNOR64", XNOR64, &op_reg_const);

    return map;
}


uint32_t assemble(HashMap *codes, char *line, uint32_t *dest) {
    char opcode[25];
    int op_start = nonspace(line, 0);
    int op_end = space(line, op_start);
    memcpy(opcode, line + op_start, op_end);
    opcode[op_end - op_start] = '\0';
//    printf("got opcode %s\n", opcode);
    Builder *builder = get_map(codes, opcode);
    if (builder == NULL) {
        printf("Unknown opcode %s\n", opcode);
    }
    int size = builder->emitter(line, op_end, builder->opcode, dest);
    printf("parsed: %s\n", line);
    for(int i = 0; i < size; i++) {
        printf("emitted %d of %d: %x\n", i, size, *(dest + i));
    }
    return size;
}
