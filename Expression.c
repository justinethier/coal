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

void addStmt(SStatements *ss, SStatement *s) {
  if (ss->head == NULL) {
    ss->head = ss->tail = s;
  } else {
    ss->tail->next = ss->tail = s;
  }
}

SStatement *newStmt(EStatementType type, SExpression *expr) {
  SStatement *s = (SStatement *)malloc(sizeof(SStatement));
  s->type = type;
  s->expr = expr;
  s->next = NULL;
  return s;
}
