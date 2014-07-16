#include <stdio.h>
#include <stdlib.h>
#include "vm.h"

#define STACK_MAX 256

typedef struct {
  Object* stack[STACK_MAX];
  int stackSize;

  /* The first object in the linked list of all objects on the heap. */
  Object* firstObject;

  /* The total number of currently allocated objects. */
  int numObjects;

  /* The number of objects required to trigger a GC. */
  int maxObjects;
} VM;

void assert(int condition, const char* message) {
  if (!condition) {
    printf("%s\n", message);
    exit(1);
  }
}

VM* newVM() {
  VM* vm = malloc(sizeof(VM));
  vm->stackSize = 0;
  vm->firstObject = NULL;
  vm->numObjects = 0;
  vm->maxObjects = 8;
  return vm;
}

void push(VM* vm, Object* value) {
  assert(vm->stackSize < STACK_MAX, "Stack overflow!");
  vm->stack[vm->stackSize++] = value;
}


Object* pop(VM* vm) {
  assert(vm->stackSize > 0, "Stack underflow!");
  return vm->stack[--vm->stackSize];
}

void mark(Object* object) {
  /* If already marked, we're done. Check this first to avoid recursing
     on cycles in the object graph. */
  if (object->marked) return;

  object->marked = 1;

  if (object->type == OBJ_PAIR) {
    mark(object->head);
    mark(object->tail);
  }
}

void markAll(VM* vm)
{
  for (int i = 0; i < vm->stackSize; i++) {
    mark(vm->stack[i]);
  }
}

void sweep(VM* vm)
{
  Object** object = &vm->firstObject;
  while (*object) {
    if (!(*object)->marked) {
      /* This object wasn't reached, so remove it from the list and free it. */
      Object* unreached = *object;

      *object = unreached->next;
      free(unreached);

      vm->numObjects--;
    } else {
      /* This object was reached, so unmark it (for the next GC) and move on to
       the next. */
      (*object)->marked = 0;
      object = &(*object)->next;
    }
  }
}

void gc(VM* vm) {
  int numObjects = vm->numObjects;

  markAll(vm);
  sweep(vm);

  vm->maxObjects = vm->numObjects * 2;

  printf("Collected %d objects, %d remaining.\n", numObjects - vm->numObjects,
         vm->numObjects);
}

Object* newObject(VM* vm, ObjectType type) {
  if (vm->numObjects == vm->maxObjects) gc(vm);

  Object* object = malloc(sizeof(Object));
  object->type = type;
  object->next = vm->firstObject;
  vm->firstObject = object;
  object->marked = 0;

  vm->numObjects++;

  return object;
}

void pushInt(VM* vm, int intValue) {
  Object* object = newObject(vm, OBJ_INT);
  object->value = intValue;

  push(vm, object);
}

Object* pushPair(VM* vm) {
  Object* object = newObject(vm, OBJ_PAIR);
  object->tail = pop(vm);
  object->head = pop(vm);

  push(vm, object);
  return object;
}

void objectPrint(Object* object) {
  switch (object->type) {
    case OBJ_INT:
      printf("%d", object->value);
      break;

    case OBJ_PAIR:
      printf("(");
      objectPrint(object->head);
      printf(", ");
      objectPrint(object->tail);
      printf(")");
      break;

    default:
      printf("Unknown object type %d\n", object->type);
  }
}

void test1() {
  printf("Test 1: Objects on stack are preserved.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);

  gc(vm);
  assert(vm->numObjects == 2, "Should have preserved objects.");
}

void test2() {
  printf("Test 2: Unreached objects are collected.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
  pop(vm);
  pop(vm);

  gc(vm);
  assert(vm->numObjects == 0, "Should have collected objects.");
}

void test3() {
  printf("Test 3: Reach nested objects.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
  pushPair(vm);
  pushInt(vm, 3);
  pushInt(vm, 4);
  pushPair(vm);
  pushPair(vm);

  gc(vm);
  assert(vm->numObjects == 7, "Should have reached objects.");
}

void test4() {
  printf("Test 4: Handle cycles.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
  Object* a = pushPair(vm);
  pushInt(vm, 3);
  pushInt(vm, 4);
  Object* b = pushPair(vm);

  a->tail = b;
  b->tail = a;

  gc(vm);
  assert(vm->numObjects == 4, "Should have collected objects.");
}

void perfTest() {
  printf("Performance Test.\n");
  VM* vm = newVM();

  for (int i = 0; i < 1000; i++) {
    for (int j = 0; j < 20; j++) {
      pushInt(vm, i);
    }

    for (int k = 0; k < 20; k++) {
      pop(vm);
    }
  }
}

void evalAdd(VM* vm) {
  Object* a = pop(vm);
  Object* b = pop(vm);
  assert(a->type == OBJ_INT, "a is not an integer");
  assert(b->type == OBJ_INT, "b is not an integer");
  pushInt(vm, a->value + b->value);
}

void evalMul(VM* vm) {
  Object* a = pop(vm);
  Object* b = pop(vm);
  assert(a->type == OBJ_INT, "a is not an integer");
  assert(b->type == OBJ_INT, "b is not an integer");
  pushInt(vm, a->value * b->value);
}

void eval(VM* vm, unsigned char bytecode[], int size) {
  int i = 0;
  Object* acc;

  while (i < size) {
    unsigned char instruction = bytecode[i];

    printf("DEBUG OP: %d\n", instruction);
    switch (instruction) {
      case INST_LITERAL:
        pushInt(vm, (int) bytecode[++i]);
        break;

      case INST_IO:
        acc = pop(vm);
        objectPrint(acc);
        break;

      case INST_ADD:
        evalAdd(vm);
        break;

      case INST_MUL:
        evalMul(vm);
        break;
    }

    i++;
  }
} 

// TODO: bytecode will change too much for this test to be stable.
// better to create a simple compiler and move the test there
int jaetest(){
  FILE *bin = fopen("out.bin", "rb");
  unsigned char bytecode[1024]; // TODO: use a different data structure
  VM* vm = newVM();
  //unsigned char bytecode[] = {INST_LITERAL, 1, INST_LITERAL, 2, INST_ADD, INST_IO};

  int num = fread(bytecode, sizeof(unsigned char), 1024, bin);
  fclose(bin);
printf("DEBUG num = %d\n", num);
  eval(vm, bytecode, num); 
}

int main(int argc, const char * argv[]) {
// TODO: wrap in a while(1) and see if memory usage is constant
  test1();
  test2();
  test3();
  test4();

  perfTest();

  jaetest();
  return 0;
}
