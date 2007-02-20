#define BOOLEAN_OPEN    ((ushort) 32768u)
#define BOOLEAN_CLOSE   ((ushort) 32769u)
#define BOOLEAN_AND     ((ushort) 32770u)
#define BOOLEAN_OR      ((ushort) 32771u)
#define BOOLEAN_NOT     ((ushort) 32772u)
#define BOOLEAN_XOR     ((ushort) 32773u)
#define BOOLEAN_TRUE    ((ushort) 32774u)
#define BOOLEAN_FALSE   ((ushort) 32775u)

#define BOOLEAN_END     ((ushort) 65535u)

Bool IsBooleanExpressionValid(const BoolExpr Expression);
Bool EvaluateBooleanExpression(const BoolExpr Expression, Bool (*Tester)(short));
#ifdef WINCIT
Bool CreateBooleanExpression(char *InputString, short (*Tester)(const char *, TermWindowC *TW), BoolExpr Expression, TermWindowC *TW);
#else
Bool CreateBooleanExpression(char *InputString, short (*Tester)(const char *), BoolExpr Expression);
#endif
Bool IsInBooleanExpression(const BoolExpr Expression, ushort ToTest);
