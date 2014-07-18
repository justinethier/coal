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
  , INST_LITERAL            = 0x00 // EG: LOAD literal
  , INST_LOAD       = 0x10 // EG: LOAD from memory, push to stack
  , INST_STORE      = 0x11
// TODO: COPY (???)
// can variables be stored in the same stack, at the
// "front" of the function? each variable then becomes a
// memory address (translated by compiler). I think that
// would work since the GC will keep track of object references
// even if an object is not stored on the stack (right?)
//
// might be hard to make this work though, since stack location
// is dynamic (relative to activation frame). offset in LD/MOV
// instruction would need to be that relative from the act. frame

// TODO: Since the stack is the key means of data manipulation in a stack-oriented programming language, often these languages provide some sort of stack manipulation operators. Commonly provided are dup, to duplicate the element at the top of the stack, exch (or swap), to exchange elements at the top of the stack (the first becomes the second and the second becomes the first), roll, to cyclically permute elements in the stack or on part of the stack, pop (or drop), to discard the element at the top of the stack (push is implicit), and others. These become key in studying procedures.

// read var/mem
// write var/mem
  , INST_IO                 = 0x01
  , INST_ADD                = 0x02
  , INST_MUL                = 0x03
  // TODO: CALL addr (push PC on stack, set PC to addr)
  //   could push vars on stack, too. would have to pop them back off when
  //   the function returns, though may have to keep track of pc and #vars in
  //   a separate stack/place
  //
  // TODO: ret (pop and set PC to value) optionally push return value?
  // TODO: IF (cmp?)
  // TODO: JUMP addr
};
 
#endif // __VM_H__
