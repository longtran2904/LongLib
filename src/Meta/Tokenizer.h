/* date = November 14th 2021 11:38 pm */

#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#define REGISTER_ENUM(x) x,
typedef enum
{
#include "TokenNames.h"
    TokenTypeCount
} TokenType;
#undef REGISTER_ENUM

#define REGISTER_ENUM(x) #x,
static char* tokenTypeName[] = 
{
#include "TokenNames.h"
    "Invalid"
};
#undef REGISTER_ENUM

typedef struct string string;
struct string
{
    int count;
    char* data;
};

typedef struct Token Token;
struct Token
{
    char* fileName;
    int lineNumber;
    int columnNumber;
    
    TokenType type;
    string value;
};

typedef struct Tokenizer Tokenizer;
struct Tokenizer
{
    char* fileName;
    int lineNumber;
    int columnNumber;
    
    bool parsing;
    string input;
    char at[2];
};

void Error(Tokenizer* tokenizer, Token token, char* error);

Token RequireToken(Tokenizer* tokenizer, TokenType type);
bool TokenEquals(Token token, char* match);
Token GetToken(Tokenizer* tokenizer);
Token PeekToken(Tokenizer* tokenizer);
bool OptionalToken(Tokenizer* tokenizer, TokenType type, Token* token);
Token AdvanceTokenizer(Tokenizer* tokenizer, TokenType type);
Token RequireIdentifier(Tokenizer* tokenizer, char* match);
Tokenizer Tokenize(string input, char* fileName);

#endif //_TOKENIZER_H
