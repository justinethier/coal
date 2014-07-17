/*
 * vm.h
 * Types used by the virtual machine
 */
#ifndef __VM_H__
#define __VM_H__
 
typedef enum {
  OBJ_INT,
  OBJ_PAIR
} ObjectType;

typedef struct sObject {
  ObjectType type;
  unsigned char marked;

  /* The next object in the linked list of heap allocated objects. */
  struct sObject* next;

  union {
    /* OBJ_INT */
    int value;

    /* OBJ_PAIR */
    struct {
      struct sObject* head;
      struct sObject* tail;
    };
  };
} Object;

enum Instruction {
    // TODO: last top-level statement should have a HALT after it,
    //       to prevent execution from overrunning it
    INST_HALT               = 0xFF
  , INST_LITERAL            = 0x00
  , INST_IO                 = 0x01
  , INST_ADD                = 0x02
  , INST_MUL                = 0x03
  // TODO: CALL addr (push PC on stack, set PC to addr)
  // TODO: ret (pop and set PC to value) optionally push return value?
  // TODO: IF (cmp?)
  // TODO: JUMP addr
};
 
#endif // __VM_H__
