#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_CAP 0x400;
#define MAX_RUNTIME_EXHAUST 69

#define printfln(...)                                                          \
  do {                                                                         \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  } while (0)

// as the runtime doesn't have any registers as of now the pop
// instruction will just remove the top element from the stack
typedef enum INST_TYPE {
  STACK_PUSH,
  STACK_POP,
  STACK_ADD,
  STACK_INCR,
  STACK_DECR,
  STACK_SUB,
  STACK_MUL,
  STACK_DIV,
  STACK_JMP,
  STACK_DUP,
  HALT,
} INST_TYPE;

typedef struct instruction {
  INST_TYPE inst;
  int64_t data;
} instruction;

typedef struct machine {
  int16_t sp;
  int16_t bp;
  int16_t size;
  int16_t ip;
  bool halt;
  instruction text[0x0ffff];
  int16_t stack[0x0ffff];
} runtime;

typedef enum TRAP {
  TRAP_OK,
  TRAP_STACK_OVERFLOW,
  TRAP_STACK_UNDERFLOW,
  TRAP_ILLEGAL,
  TRAP_EXIT,
} TRAP;
char *inst_to_str(INST_TYPE inst) {
  switch (inst) {
  case STACK_PUSH: {
    return "STACK_PUSH";
  } break;
  case STACK_POP: {
    return "STACK_POP";
  } break;
  case STACK_ADD: {
    return "STACK_ADD";
  } break;
  case STACK_INCR: {
    return "STACK_INCR";
  } break;
  case STACK_DECR: {
    return "STACK_DECR";
  } break;
  case STACK_SUB: {
    return "STACK_SUB";
  } break;
  case STACK_MUL: {
    return "STACK_MUL";
  } break;
  case STACK_DIV: {
    return "STACK_DIV";
  } break;
  case STACK_JMP: {
    return "STACK_JMP";
  } break;
  case HALT: {
    return "HALT ";
  } break;
  default:
    return "INVALID INSTRUCTION";
    break;
  }
  return "INVALID INSTRUCTION";
}
char *trap_to_str(TRAP trap) {
  switch (trap) {
  case TRAP_OK:
    return "TRAP_OK";
  case TRAP_STACK_OVERFLOW:
    return "TRAP_STACK_OVERFLOW";
  case TRAP_STACK_UNDERFLOW:
    return "TRAP_STACK_UNDERFLOW";
  case TRAP_ILLEGAL:
    return "TRAP_ILLEGAL";
  case TRAP_EXIT:
    return "TRAP_EXIT";
  default:
    return "TRAP_UNKOWN";
  }
}
#define MAKE_INST(i, d)                                                        \
  { .inst = i, .data = d }

int is_stack_full(runtime *r) { return r->sp == STACK_CAP; }

int is_stack_empty(runtime *r) { return r->sp == 0; }

TRAP stack_push(runtime *runtime) {
  if (is_stack_full(runtime)) {
    return TRAP_STACK_OVERFLOW;
  }
  runtime->stack[runtime->sp] = runtime->text[runtime->ip].data;
  runtime->sp++;
  return TRAP_OK;
}

TRAP stack_pop(runtime *runtime) {
  if (is_stack_empty(runtime)) {
    return TRAP_STACK_UNDERFLOW;
  }
  runtime->sp--;
  return TRAP_OK;
}
TRAP stack_add(runtime *runtime) {
  if (runtime->sp < 2) {
    return TRAP_STACK_UNDERFLOW;
  }
  runtime->stack[runtime->sp - 2] += runtime->stack[runtime->sp - 1];
  runtime->sp--;
  return TRAP_OK;
}
TRAP stack_sub(runtime *runtime) {
  if (runtime->sp < 2) {
    return TRAP_STACK_UNDERFLOW;
  }
  runtime->stack[runtime->sp - 2] -= runtime->stack[runtime->sp - 1];
  runtime->sp--;
  return TRAP_OK;
}
TRAP stack_mul(runtime *runtime) {
  if (runtime->sp < 2) {
    return TRAP_STACK_UNDERFLOW;
  }
  runtime->stack[runtime->sp - 2] *= runtime->stack[runtime->sp - 1];
  runtime->sp--;
  return TRAP_OK;
}
TRAP stack_div(runtime *runtime) {
  if (runtime->sp < 2) {
    return TRAP_STACK_UNDERFLOW;
  }
  runtime->stack[runtime->sp - 2] /= runtime->stack[runtime->sp - 1];
  runtime->sp--;
  return TRAP_OK;
}
TRAP stack_jmp(runtime *runtime) {
  runtime->ip = runtime->stack[--runtime->sp];
  return TRAP_OK;
}
void dump_stack(runtime *runtime) {
  printfln("[STACKDUMP]");
  int i = 0;
  while (i < runtime->sp) {
    printf("%d:", i);
    printfln("%d", runtime->stack[i]);
    i++;
  }
  printfln("[END]\n");
}
TRAP stack_addi(runtime *runtime, int64_t data) {
  if (is_stack_empty(runtime)) {
    return TRAP_STACK_UNDERFLOW;
  }
  runtime->stack[runtime->sp - 1] += data;
  return TRAP_OK;
}
TRAP stack_subi(runtime *runtime, int64_t data) {
  if (is_stack_empty(runtime)) {
    return TRAP_STACK_UNDERFLOW;
  }
  runtime->stack[runtime->sp - 1] += data;

  return TRAP_OK;
}
TRAP stack_dup(runtime *runtime) {
  if (is_stack_full(runtime)) {
    return TRAP_STACK_UNDERFLOW;
  }
  runtime->stack[runtime->sp] =
      runtime->stack[runtime->sp - 1 - runtime->text[runtime->ip].data];
  runtime->sp++;
  return TRAP_OK;
}

TRAP exec_inst(runtime *runtime) {
  instruction inst = runtime->text[runtime->ip];
  switch (inst.inst) {
  case STACK_PUSH: {
    TRAP t = stack_push(runtime);
    runtime->ip++;
    return t;
  }
  case STACK_POP: {
    TRAP t = stack_pop(runtime);
    runtime->ip++;
    return t;
  }
  case STACK_ADD: {
    TRAP t = stack_add(runtime);
    runtime->ip++;
    return t;
  }
  case STACK_SUB: {
    TRAP t = stack_sub(runtime);
    runtime->ip++;
    return t;
  }
  case STACK_MUL: {
    TRAP t = stack_mul(runtime);
    runtime->ip++;
    return t;
  }
  case STACK_DIV: {
    TRAP t = stack_div(runtime);
    runtime->ip++;
    return t;
  }
  case STACK_JMP: {
    TRAP t = stack_jmp(runtime);
    return t;
  }
  case STACK_INCR: {
    TRAP t = stack_addi(runtime, 1);
    runtime->ip++;
    return t;
  }
  case STACK_DECR: {
    TRAP t = stack_addi(runtime, 1);
    runtime->ip++;
    return t;
  }
  case STACK_DUP: {
    TRAP t = stack_dup(runtime);
    runtime->ip++;
    return t;
  }
  case HALT:
    runtime->ip++;
    runtime->halt = 1;
    break;
  default:
    return TRAP_ILLEGAL;
  }
  return TRAP_OK;
}
void load_program_from_memory(runtime *runtime, instruction *program,
                              int16_t program_size) {
  memcpy(runtime->text, program, program_size * (sizeof(instruction)));

  return;
}

#define SIZEOF_ARR(A) sizeof(A) / sizeof(A[0])
instruction fibonacci_sequence[] = {
    MAKE_INST(STACK_PUSH, 0), MAKE_INST(STACK_PUSH, 1),
    MAKE_INST(STACK_DUP, 0),  MAKE_INST(STACK_DUP, 2),
    MAKE_INST(STACK_ADD, 0),  MAKE_INST(STACK_PUSH, 2),
    MAKE_INST(STACK_JMP, 0),  MAKE_INST(HALT, 0),
};

int main() {
  runtime runtime = {0};
  load_program_from_memory(&runtime, fibonacci_sequence,
                           SIZEOF_ARR(fibonacci_sequence));
  int i = 0;
  while (!runtime.halt && i < MAX_RUNTIME_EXHAUST) {
    TRAP trap = exec_inst(&runtime);
    if (trap != TRAP_OK && trap != TRAP_EXIT && trap != TRAP_ILLEGAL) {
      fprintf(stderr, "PROGRAM TRAPPED WITH %s\n", trap_to_str(trap));
      runtime.halt = true;
    }
    i++;
  }
  dump_stack(&runtime);
  return 0;
}
