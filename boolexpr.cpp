// --------------------------------------------------------------------------
// Citadel: BoolExpr.CPP
//
// Boolean expression evaluation code stuff thingies.

#include "ctdl.h"
#pragma hdrstop

#include "boolexpr.h"


// --------------------------------------------------------------------------
// can each type be followed by...
//
//     value|unary|open|close|binary
// val|     |     |    | xxx | xxx
// unr| xxx | xxx | xx |     |
// opn| xxx | xxx | xx |     |
// cls|     |     |    | xxx | xxx
// bnr| xxx | xxx | xx |     |
//
// we initialize with an open...

enum BooleanState
    {
    BS_VALUE,   BS_UNARY,   BS_OPEN,    BS_CLOSE,   BS_BINARY
    };

Bool IsBooleanExpressionValid(const BoolExpr Expression)
    {
    BooleanState LastState = BS_OPEN;
    int NumOpen = 0;

    for (int i = 0; i < BOOL_COMPLEX; i++)
        {
        BooleanState ThisState;

        if (Expression[i] <= 32767)
            {
            ThisState = BS_VALUE;
            }
        else
            {
            switch (Expression[i])
                {
                case BOOLEAN_OPEN:
                    {
                    NumOpen++;
                    ThisState = BS_OPEN;
                    break;
                    }

                case BOOLEAN_CLOSE:
                    {
                    if (--NumOpen < 0)
                        {
                        return (FALSE);
                        }

                    ThisState = BS_CLOSE;
                    break;
                    }

                case BOOLEAN_END:
                    {
                    ThisState = BS_CLOSE;
                    break;
                    }

                case BOOLEAN_AND:
                case BOOLEAN_OR:
                case BOOLEAN_XOR:
                    {
                    ThisState = BS_BINARY;
                    break;
                    }

                case BOOLEAN_NOT:
                    {
                    ThisState = BS_UNARY;
                    break;
                    }

                case BOOLEAN_TRUE:
                case BOOLEAN_FALSE:
                    {
                    ThisState = BS_VALUE;
                    break;
                    }

                default:
                    {
                    return (FALSE);
                    }
                }
            }

        switch (LastState)
            {
            case BS_VALUE:
            case BS_CLOSE:
                {
                if (ThisState == BS_VALUE || ThisState == BS_UNARY ||
                        ThisState == BS_OPEN)
                    {
                    return (FALSE);
                    }

                break;
                }

            case BS_UNARY:
            case BS_OPEN:
            case BS_BINARY:
                {
                if (ThisState == BS_CLOSE || ThisState == BS_BINARY)
                    {
                    return (FALSE);
                    }

                break;
                }

            default:
                {
                assert(FALSE);
                return (FALSE);
                }
            }

        if (Expression[i] == BOOLEAN_END)
            {
            return (NumOpen == 0);
            }

        LastState = ThisState;
        }

    return (FALSE);
    }

static Bool ExpressionHasGroups(const BoolExpr Expression)
    {
    int Index;

    for (Index = 0; Index < BOOL_COMPLEX && Expression[Index] != BOOLEAN_END;
            Index++)
        {
        if (Expression[Index] == BOOLEAN_OPEN)
            {
            return (TRUE);
            }
        }

    return (FALSE);
    }

static int GetIndexOfOneBeyondNotInExpression(const BoolExpr Expression)
    {
    int Index;

    for (Index = 0; Index < BOOL_COMPLEX && Expression[Index] != BOOLEAN_END; Index++)
        {
        if (Expression[Index] == BOOLEAN_NOT)
            {
            return (Index + 1);
            }
        }

    return (0);
    }

static int GetIndexOfAndInExpression(const BoolExpr Expression)
    {
    int Index;

    for (Index = 0; Index < BOOL_COMPLEX && Expression[Index] != BOOLEAN_END; Index++)
        {
        if (Expression[Index] == BOOLEAN_AND)
            {
            return (Index);
            }
        }

    return (0);
    }

static int GetIndexOfOrInExpression(const BoolExpr Expression)
    {
    int Index;

    for (Index = 0; Index < BOOL_COMPLEX && Expression[Index] != BOOLEAN_END; Index++)
        {
        if (Expression[Index] == BOOLEAN_OR)
            {
            return (Index);
            }
        }

    return (0);
    }

static int GetIndexOfXorInExpression(const BoolExpr Expression)
    {
    int Index;

    for (Index = 0; Index < BOOL_COMPLEX && Expression[Index] != BOOLEAN_END; Index++)
        {
        if (Expression[Index] == BOOLEAN_XOR)
            {
            return (Index);
            }
        }

    return (0);
    }

static Bool TestValue(ushort Code, Bool (*Tester)(short))
    {
    // I want to use this, not < BOOL_OPEN
    if (Code <= 32767)
        {
        return ((*Tester)(Code));
        }

    if (Code == BOOLEAN_TRUE)
        {
        return (TRUE);
        }

    // BOOLEAN_FALSE or bad value
    return (FALSE);
    }

static void ExtractInnerGroup(BoolExpr Original, BoolExpr Extraction, int *Place)
    {
    int Index, LastFound = -1;

    for (Index = 0; Index < BOOL_COMPLEX && Original[Index] != BOOLEAN_END; Index++)
        {
        if (Original[Index] == BOOLEAN_OPEN)
            {
            LastFound = Index;
            }
        }

    assert(LastFound != -1);

    for (Index = LastFound + 1; Index < BOOL_COMPLEX && Original[Index] != BOOLEAN_CLOSE; Index++)
        {
        Extraction[Index - LastFound - 1] = Original[Index];
        }

    Extraction[Index - LastFound - 1] = BOOLEAN_END;

    memcpy(Original + LastFound, Original + Index, BOOL_COMPLEX - Index - 1);

    *Place = LastFound;
    }

#if !defined(NDEBUG)
#ifdef WINCIT
static void ShowInt(short Wow, TermWindowC *TW)
    {
    TW->DebugOut(pcthd, Wow);
    }
#else
static void ShowInt(short Wow)
    {
    DebugOut(pcthd, Wow);
    }
#endif
#endif

Bool EvaluateBooleanExpression(const BoolExpr InpExpression, Bool (*Tester)(short))
    {
    if (!IsBooleanExpressionValid(InpExpression))
        {
        return (FALSE);
        }

    BoolExpr Expression;

    memcpy(Expression, InpExpression, sizeof(BoolExpr));

    // evaluate groups first
    while (ExpressionHasGroups(Expression))
        {
        BoolExpr ExtractedGroup;
        int Index;

        ExtractInnerGroup(Expression, ExtractedGroup, &Index);

        Expression[Index] = (short) EvaluateBooleanExpression(ExtractedGroup, Tester) ? BOOLEAN_TRUE : BOOLEAN_FALSE;
        }

    // then "not"s
    while (GetIndexOfOneBeyondNotInExpression(Expression))
        {
        int Index = GetIndexOfOneBeyondNotInExpression(Expression);
        Bool Value = TestValue(Expression[Index], Tester);

        memcpy(Expression + Index - 1, Expression + Index, sizeof(ushort) * (BOOL_COMPLEX - Index));

        Expression[Index - 1] = Value ? BOOLEAN_FALSE : BOOLEAN_TRUE;
        }

    // then "and"s
    while (GetIndexOfAndInExpression(Expression))
        {
        int Index = GetIndexOfAndInExpression(Expression);

        Bool Value1 = TestValue(Expression[Index - 1], Tester);
        Bool Value2 = TestValue(Expression[Index + 1], Tester);

        memcpy(Expression + Index - 1, Expression + Index + 1, sizeof(ushort) * (BOOL_COMPLEX - Index - 1));

        Expression[Index - 1] = (Value1 && Value2) ? BOOLEAN_TRUE : BOOLEAN_FALSE;
        }

    // then "or"s
    while (GetIndexOfOrInExpression(Expression))
        {
        int Index = GetIndexOfOrInExpression(Expression);

        Bool Value1 = TestValue(Expression[Index - 1], Tester);
        Bool Value2 = TestValue(Expression[Index + 1], Tester);

        memcpy(Expression + Index - 1, Expression + Index + 1, sizeof(ushort) * (BOOL_COMPLEX - Index - 1));

        Expression[Index - 1] = (Value1 || Value2) ? BOOLEAN_TRUE : BOOLEAN_FALSE;
        }

    // then "xor"s
    while (GetIndexOfXorInExpression(Expression))
        {
        int Index = GetIndexOfXorInExpression(Expression);

        Bool Value1 = TestValue(Expression[Index - 1], Tester);
        Bool Value2 = TestValue(Expression[Index + 1], Tester);

        memcpy(Expression + Index - 1, Expression + Index + 1, sizeof(ushort) * (BOOL_COMPLEX - Index - 1));

        Expression[Index - 1] = (Value1 ^ Value2) ? BOOLEAN_TRUE : BOOLEAN_FALSE;
        }

    return (TestValue(Expression[0], Tester));
    }

#ifdef WINCIT
void TERMWINDOWMEMBER ShowBooleanExpression(const BoolExpr Expression, void (*Shower)(short, TermWindowC *TW))
#else
void TERMWINDOWMEMBER ShowBooleanExpression(const BoolExpr Expression, void (*Shower)(short))
#endif
    {
    int Index;

    for (Index = 0; Index < BOOL_COMPLEX && Expression[Index] != BOOLEAN_END; Index++)
        {
        if (Expression[Index] <= 32767)
            {
#ifdef WINCIT
            (*Shower)(Expression[Index], this);
#else
            (*Shower)(Expression[Index]);
#endif
            }
        else
            {
            switch (Expression[Index])
                {
                case BOOLEAN_OPEN:      mPrintf(getmsg(436));   break;
                case BOOLEAN_CLOSE:     mPrintf(getmsg(437));   break;
                case BOOLEAN_AND:       mPrintf(getmsg(438));   break;
                case BOOLEAN_OR:        mPrintf(getmsg(439));   break;
                case BOOLEAN_NOT:       mPrintf(getmsg(440));   break;
                case BOOLEAN_XOR:       mPrintf(getmsg(441));   break;
                case BOOLEAN_TRUE:      mPrintf(getmsg(626));   break;
                case BOOLEAN_FALSE:     mPrintf(getmsg(627));   break;
                default:                mPrintf(getmsg(442));   break;
                }
            }

        mPrintf(spc);
        }
    }

#ifdef WINCIT
Bool CreateBooleanExpression(char *InputString, short (*Tester)(const char *, TermWindowC *TW), BoolExpr Expression, TermWindowC *TW)
#else
Bool CreateBooleanExpression(char *InputString, short (*Tester)(const char *), BoolExpr Expression)
#endif
    {
    char *words[256];
    int count;

    count = parse_it(words, InputString);

    if (count <= BOOL_COMPLEX)
        {
        int i;

        for (i = 0; i < count; i++)
            {
            if (SameString(words[i], getmsg(436)))
                {
                Expression[i] = BOOLEAN_OPEN;
                }
            else if (SameString(words[i], getmsg(437)))
                {
                Expression[i] = BOOLEAN_CLOSE;
                }
            else if (SameString(words[i], getmsg(438)))
                {
                Expression[i] = BOOLEAN_AND;
                }
            else if (SameString(words[i], getmsg(439)))
                {
                Expression[i] = BOOLEAN_OR;
                }
            else if (SameString(words[i], getmsg(440)))
                {
                Expression[i] = BOOLEAN_NOT;
                }
            else if (SameString(words[i], getmsg(441)))
                {
                Expression[i] = BOOLEAN_XOR;
                }
            else if (SameString(words[i], getmsg(626)))
                {
                Expression[i] = BOOLEAN_TRUE;
                }
            else if (SameString(words[i], getmsg(627)))
                {
                Expression[i] = BOOLEAN_FALSE;
                }
            else
                {
#ifdef WINCIT
                short Value = (*Tester)(words[i], TW);
#else
                short Value = (*Tester)(words[i]);
#endif

                if (Value != CERROR)
                    {
                    Expression[i] = Value;
                    }
                else
                    {
                    return (FALSE);
                    }
                }
            }

        Expression[i] = BOOLEAN_END;

        if (IsBooleanExpressionValid(Expression))
            {
            return (TRUE);
            }
        }

    return (FALSE);
    }

Bool IsInBooleanExpression(const BoolExpr Expression, ushort ToTest)
    {
    for (int i = 0; i < BOOL_COMPLEX; i++)
        {
        if (Expression[i] == ToTest)
            {
            return (TRUE);
            }

        if (Expression[i] == BOOLEAN_END)
            {
            break;
            }
        }

    return (FALSE);
    }
