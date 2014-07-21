/*
 * Expression.c
 * Implementation of functions used to build the syntax tree.
 */
 
#include "Expression.h"
 
#include <stdlib.h>
 
/**
 * @brief Allocates space for expression
 * @return The expression or NULL if not enough memory
 */
static SExpression *allocateExpression()
{
    SExpression *b = (SExpression *)malloc(sizeof(SExpression));
 
    if (b == NULL)
        return NULL;
 
    b->type = eVALUE;
    b->value = 0;
 
    b->left = NULL;
    b->right = NULL;
 
    return b;
}
 
SExpression *createNumber(int value)
{
    SExpression *b = allocateExpression();
 
    if (b == NULL)
        return NULL;
 
    b->type = eVALUE;
    b->value = value;
 
    return b;
}

SExpression *createIdentifier(char *ident) {
  SExpression *b = allocateExpression();
  if (b == NULL) return NULL;

  b->type = eIDENT;
  b->ident = ident;

  return b;
}
 
SExpression *createOperation(EOperationType type, SExpression *left, SExpression *right)
{
    SExpression *b = allocateExpression();
 
    if (b == NULL)
        return NULL;
 
    b->type = type;
    b->left = left;
    b->right = right;
 
    return b;
}
 
void deleteExpression(SExpression *b)
{
    if (b == NULL)
        return;
 
    deleteExpression(b->left);
    deleteExpression(b->right);
 
    free(b);
}

SStatements *initStmts(SStatement *s) {
    SStatements *ss = (SStatements *)malloc(sizeof(SStatements));
    ss->head = ss->tail = s;
    return ss;
}

SStatements *addStmt(SStatements *ss, SStatement *s) {
  if (ss->head == NULL) {
    ss->head = ss->tail = s;
  } else {
    ss->tail->next = s;
    ss->tail = s;
  }

  return ss;
}

int numStmts(SStatements *ss) {
  int i = 0;
  if (ss != NULL) {
    for (SStatement *s = ss->head; s; s = s->next) {
      i++;
    }
  }
  return i;
}

SStatement *newStmt(EStatementType type, SExpression *expr) {
  SStatement *s = (SStatement *)malloc(sizeof(SStatement));
  s->type = type;
  s->expr = expr;
  s->identifier = NULL;
  s->next = NULL;
  return s;
}

void deleteStmts(SStatements *ss) {
  SStatement *next;
  if (ss == NULL) return;

  for (SStatement *s = ss->head; s; s = next) {
    next = s->next;
    deleteExpression(s->expr);
    free(s);
  }

  free(ss);
}
