#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include "Tokenizer.h"
#include "Meta.h"

static string ReadText(char* fileName)
{
    FILE* file = fopen(fileName, "r");
    string result = {0};
    if (file)
    {
        fseek(file, 0, SEEK_END);
        result.count = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        result.data = (char*)malloc(result.count);
        fread(result.data, result.count, 1, file);
        
        fclose(file);
    }
    
    return result;
}

inline bool IsEndOfLine(char c)
{
    return ((c == '\n') ||
            (c == '\r'));
}

inline bool IsWhitespace(char c)
{
    return ((c == ' ') ||
            (c == '\t') ||
            (c == '\v') ||
            (c == '\f') ||
            IsEndOfLine(c));
}

inline bool IsAlpha(char c)
{
    return (((c >= 'a') && (c <= 'z')) ||
            ((c >= 'A') && (c <= 'Z')) ||
            (c == '_'));
}

inline bool IsNumber(char c)
{
    return ((c >= '0') && (c <= '9'));
}

inline Token RequireToken(Tokenizer* tokenizer, TokenType type)
{
    Token token = GetToken(tokenizer);
    //if (token.type != type) Error();
    return token;
}

inline bool TokenEquals(Token token, char* match)
{
    char *at = match;
    for(int index = 0; index < token.value.count; ++index, ++at)
    {
        if((*at == 0) ||
           (token.value.data[index] != *at))
        {
            return false;
        }
    }
    
    return *at == 0;
}

bool OptionalToken(Tokenizer* tokenizer, TokenType type, Token* token)
{
    if (PeekToken(tokenizer).type == type)
    {
        if (token)
            *token = GetToken(tokenizer);
        else
            GetToken(tokenizer);
        return true;
    }
    return false;
}

Token RequireIdentifier(Tokenizer* tokenizer, char* match)
{
    Token result = RequireToken(tokenizer, Identifier);
    //if (!TokenEquals(result, match)) Error();
    return result;
}

Token AdvanceTokenizer(Tokenizer* tokenizer, TokenType type)
{
    for (;;)
    {
        Token token = GetToken(tokenizer);
        if ((token.type == type) ||
            (token.type == EndOfStream))
            return token;
    }
}

bool IsStringValid(string s)
{
    if (s.count > 0 && s.data != NULL)
        return true;
    return false;
}

static void Refill(Tokenizer* tokenizer)
{
    if (tokenizer->input.count == 0)
    {
        tokenizer->at[0] = 0;
        tokenizer->at[1] = 0;
    }
    else if (tokenizer->input.count == 1)
    {
        tokenizer->at[0] = tokenizer->input.data[0];
        tokenizer->at[1] = 0;
    }
    else
    {
        tokenizer->at[0] = tokenizer->input.data[0];
        tokenizer->at[1] = tokenizer->input.data[1];
    }
}

static void AdvanceChars(Tokenizer* tokenizer, int count)
{
    if (count > tokenizer->input.count)
        count = tokenizer->input.count;
    
    tokenizer->input.count -= count;
    tokenizer->input.data += count;
    
    if (IsEndOfLine(tokenizer->at[0]))
    {
        ++tokenizer->lineNumber;
        tokenizer->columnNumber = 1;
    }
    else
        tokenizer->columnNumber += count;
    Refill(tokenizer);
}

void EatAllWhitespace(Tokenizer* tokenizer)
{
    for(;;)
    {
        if(IsWhitespace(tokenizer->at[0]))
        {
            AdvanceChars(tokenizer, 1);
        }
        // Ignore comment.
        else if((tokenizer->at[0] == '/') &&
                (tokenizer->at[1] == '/'))
        {
            AdvanceChars(tokenizer, 2);
            while(tokenizer->at[0] && !IsEndOfLine(tokenizer->at[0]))
            {
                AdvanceChars(tokenizer, 1);
            }
        }
        else if((tokenizer->at[0] == '/') &&
                (tokenizer->at[1] == '*'))
        {
            AdvanceChars(tokenizer, 2);
            while(tokenizer->at[0] &&
                  !((tokenizer->at[0] == '*') &&
                    (tokenizer->at[1] == '/')))
            {
                AdvanceChars(tokenizer, 1);
            }
            
            if(tokenizer->at[0] == '*')
            {
                AdvanceChars(tokenizer, 2);
            }
        }
        else
        {
            break;
        }
    }
}

Token PeekToken(Tokenizer* tokenizer)
{
    Tokenizer temp = *tokenizer;
    Token token = GetToken(&temp);
    return token;
}

Token GetToken(Tokenizer* tokenizer)
{
    EatAllWhitespace(tokenizer);
    
    Token token = {0};
    token.fileName = tokenizer->fileName;
    token.lineNumber = tokenizer->lineNumber;
    token.columnNumber = tokenizer->columnNumber;
    token.value = tokenizer->input;
    if (token.value.count)
    {
        token.value.count = 1;
    }
    
    char c = tokenizer->at[0];
    AdvanceChars(tokenizer, 1);
    switch(c)
    {
        case '\0': {token.type = EndOfStream;} break;
        
        case '(': {token.type = OpenParen;} break;
        case ')': {token.type = CloseParen;} break;
        case ':': {token.type = Colon;} break;
        case ';': {token.type = Semicolon;} break;
        case '*': {token.type = Asterisk;} break;
        case '[': {token.type = OpenBracket;} break;
        case ']': {token.type = CloseBracket;} break;
        case '{': {token.type = OpenBrace;} break;
        case '}': {token.type = CloseBrace;} break;
        case ',': {token.type = Coma;} break;
        
        case '"':
        {
            token.type = String;
            token.value = tokenizer->input;
            
            while(tokenizer->at[0] &&
                  tokenizer->at[0] != '"')
            {
                if((tokenizer->at[0] == '\\') &&
                   tokenizer->at[1])
                {
                    AdvanceChars(tokenizer, 1);
                }                
                AdvanceChars(tokenizer, 1);
            }
            
            token.value.count = (int)(tokenizer->input.data - token.value.data);
            if(tokenizer->at[0] == '"')
            {
                AdvanceChars(tokenizer, 1);
            }
        } break;
        
        default:
        {
            if(IsAlpha(c))
            {
                token.type = Identifier;
                
                while(IsAlpha(tokenizer->at[0]) ||
                      IsNumber(tokenizer->at[0]))
                {
                    AdvanceChars(tokenizer, 1);
                }
                
                token.value.count = (int)(tokenizer->input.data - token.value.data);
            }
            // TODO: Remember to parse number.
            else
            {
                token.type = Unknown;
            }
        } break;        
    }
    
    return token;
}

Tokenizer Tokenize(string input, char* fileName)
{
    Tokenizer result = {0};
    if (IsStringValid(input))
    {
        result.input = input;
        result.parsing = true;
        result.fileName = fileName;
        result.lineNumber = 1;
        result.columnNumber = 1;
        Refill(&result);
    }
    
    return result;
}

static void ParseStructDeclaration(Tokenizer* tokenizer, Token token)
{
    Token nameToken = PeekToken(tokenizer);
    if (IsAlpha(nameToken.value.data[0]))
    {
        printf("typedef %.*s %.*s %.*s;\n",
               token.value.count, token.value.data,
               nameToken.value.count, nameToken.value.data,
               nameToken.value.count, nameToken.value.data);
        printf("#line %d \"%s\"\n", token.lineNumber - 1, token.fileName);
    }
}

static void ParseMember(Tokenizer* tokenizer, Token structTypeToken, Token memberTypeToken)
{
    bool parsing = true;
    bool isPointer = false;
    
    while (parsing)
    {
        Token token = GetToken(tokenizer);
        
        switch (token.type)
        {
            case Asterisk:
            {
                isPointer = true;
            } break;
            // TODO: Handle inner struct case. ex: struct Outer { struct Inter { ... }; ... };
            
            case Coma:
            {
                if (!OptionalToken(tokenizer, Identifier, &token))
                    parsing = false;
            }
            case Identifier:
            {
                printf("    { MetaType_%.*s, \"%.*s\", %d, (size_t)&((%.*s*)0)->%.*s },\n",
                       memberTypeToken.value.count, memberTypeToken.value.data,
                       token.value.count, token.value.data,
                       isPointer ? MetaFlag_IsPointer : 0,
                       structTypeToken.value.count, structTypeToken.value.data,
                       token.value.count, token.value.data);
            } break;
            
            case Semicolon:
            case EndOfStream:
            {
                parsing = false;
            } break;
        }
    }
}

static Token ParseStruct(Tokenizer* tokenizer)
{
    Token nameToken = RequireToken(tokenizer, Identifier);
    
    RequireToken(tokenizer, OpenBrace);
    Tokenizer temp = *tokenizer;
    AdvanceTokenizer(&temp, CloseBrace);
    AdvanceTokenizer(&temp, Semicolon); // Ex: struct A { ... } A;
    printf("struct %.*s\n{%.*s", nameToken.value.count, nameToken.value.data, tokenizer->input.count - temp.input.count, tokenizer->input.data);
    
    printf("\nMemberDefinition memeberOf_%.*s[] = \n{\n", nameToken.value.count, nameToken.value.data);
    for (;;)
    {
        Token memberToken = GetToken(tokenizer);
        if (memberToken.type == CloseBrace)
        {
            printf("};\n#line %d \"%s\"", memberToken.lineNumber, memberToken.fileName);
            return AdvanceTokenizer(tokenizer, Semicolon);
        }
        else
            ParseMember(tokenizer, nameToken, memberToken);
    }
}

static void ParseIntrospectionParams(Tokenizer* tokenizer)
{
    AdvanceTokenizer(tokenizer, CloseParen);
}

static void ParseIntrospectable(Tokenizer* tokenizer, bool definable, Token* lastToken)
{
    RequireToken(tokenizer, OpenParen);
    ParseIntrospectionParams(tokenizer);
    
    Token structToken = RequireIdentifier(tokenizer, "struct");
    if (definable)
        ParseStructDeclaration(tokenizer, structToken);
    *lastToken = ParseStruct(tokenizer);
}

static int ParseFunctionArgs(Tokenizer* tokenizer, Token* argTokens, int argCount)
{
    int i = 0;
    while (i <= argCount)
    {
        Token argToken = PeekToken(tokenizer);
        if ((argToken.type == CloseParen) ||
            (argToken.type == EndOfStream))
            break;
        else if (argToken.type == Identifier)
        {
            assert(i <= argCount);
            argTokens[i] = argToken;
            ++i;
        }
        
        GetToken(tokenizer);
    }
    
    return i;
}

static Token ParseFunction(Tokenizer* tokenizer, Token functionName)
{
    Token result = functionName;
    printf("%.*s", functionName.value.count, functionName.value.data);
    
    char* argsStart = tokenizer->input.data;
    char* argsEnd = NULL;
#define MAX_FUNCTION_ARG 10
    Token argTokens[MAX_FUNCTION_ARG] = {0};
    int argCount = ParseFunctionArgs(tokenizer, argTokens, MAX_FUNCTION_ARG);
    argsEnd = tokenizer->input.data;
    result = GetToken(tokenizer);
    assert(result.type == CloseParen);
    
    char* funcStart = tokenizer->input.data;
    bool needCallerFile = false, needCallerLine = false;
    if (OptionalToken(tokenizer, OpenBrace, 0))
    {
        int braceCount = 0;
        Token token;
        for (;;)
        {
            token = GetToken(tokenizer);
            if (token.type == CloseBrace)
            {
                if (braceCount)
                    --braceCount;
                else
                    break;
            }
            else if (token.type == OpenBrace)
                ++braceCount;
            else if (token.type == Identifier)
            {
                if (TokenEquals(token, "_CALLER_FILE_"))
                    needCallerFile = true;
                if (TokenEquals(token, "_CALLER_LINE_"))
                    needCallerLine = true;
            }
        }
        
        result = token;
    }
    
    if (!(needCallerFile || needCallerLine))
    {
        if (argCount == 0)
            printf("(void)");
        else
            printf("(%.*s)", (int)(argsEnd - argsStart), argsStart);
        printf("%.*s", (int)(tokenizer->input.data - funcStart), funcStart);
        return result;
    }
    
    char* funcArgs = "int _CALLER_LINE_";
    char* macroArgs = "__LINE__";
    if (needCallerFile && needCallerLine)
    {
        funcArgs = "char* _CALLER_FILE_, int _CALLER_LINE_";
        macroArgs = "__FILE__, __LINE__";
    }
    else if (needCallerFile)
    {
        funcArgs = "char* _CALLER_FILE_";
        macroArgs = "__FILE__";
    }
    
    if (((argCount == 1) && TokenEquals(argTokens[0], "void")) || (argCount == 0))
        printf("(");
    else
        printf("(%.*s, ", (int)(argsEnd - argsStart), argsStart);
    printf("%s)%.*s", funcArgs, (int)(tokenizer->input.data - funcStart), funcStart);
    
    printf("\n#define %.*s(", functionName.value.count, functionName.value.data);
    for(int i = 1; i < argCount; i += 2) // odd indexs are arg's names, even indexs are arg's types
        printf("%.*s%s", argTokens[i].value.count, argTokens[i].value.data, (i != argCount - 1) ? ", " : "");
    printf(")");
    
    printf(" %.*s(", functionName.value.count, functionName.value.data);
    for (int i = 1; i < argCount; i += 2)
    {
        printf("%.*s, ", argTokens[i].value.count, argTokens[i].value.data);
    }
    printf("%s)\n#line %d \"%s\"", macroArgs, result.lineNumber, result.fileName);
    
    return result;
}

#define MAX_TOKENS 100000
Token tokens[MAX_TOKENS];

void PrintTokenSpaces(Token token, Token lastToken)
{
    if (token.lineNumber > lastToken.lineNumber)
    {
        for (int i = 0; i < token.lineNumber - lastToken.lineNumber; ++i)
            printf("\n");
        printf("%*.s", token.columnNumber - 1, " ");
    }
    else
        printf("%*.s", token.columnNumber - (lastToken.columnNumber + lastToken.value.count), " ");
}

void PrintString(Token token)
{
    printf("\"%.*s\"", token.value.count, token.value.data);
    token.columnNumber++;
    token.value.count++;
}

inline void PrintToken(Token token)
{
    printf("%.*s", token.value.count, token.value.data);
}

int main(int argc, char** argv)
{
    char* fileName = "LongApp_Windows.c";
    Tokenizer tokenizer = Tokenize(ReadText(fileName), fileName);
    
    Token lastToken = {0};
    lastToken.lineNumber = 1;
    lastToken.columnNumber = 1;
    
    Tokenizer introspectTokenizer = {0};
    printf("#define META_GENERATED\n");
    
#if 0
    int numberOfTokens = 0;
    for (int i = 1;i < MAX_TOKENS; ++i)
    {
        tokens[i] = GetToken(&tokenizer);
        ++numberOfTokens;
        if (tokens[i].type == EndOfStream)
            break;
    }
    
    for (int i = 1; i < numberOfTokens - 1; ++i)
    {
        PrintTokenSpaces(tokens[i], lastToken);
        
        switch (tokens[i].type)
        {
            case String:
            {
                PrintString(tokens[i]);
                lastToken = tokens[i];
            } break;
            
            case Identifier:
            {
            }
            default:
            {
                PrintToken(tokens[i]);
                lastToken = tokens[i];
            } break;
        }
    }
#else
    while (tokenizer.parsing)
    {
        Token token = GetToken(&tokenizer);
        PrintTokenSpaces(token, lastToken);
        
        switch (token.type)
        {
            case EndOfStream:
            {
                tokenizer.parsing = false;
            } break;
            case String:
            {
                PrintString(token);
                lastToken = token;
            } break;
            
            case Identifier:
            {
                if (TokenEquals(token, "introspect"))
                {
                    ParseIntrospectable(&tokenizer, !TokenEquals(lastToken, "typedef"), &lastToken);
                    break;
                }
                else if (!TokenEquals(lastToken, "typedef") && !TokenEquals(lastToken, "define"))
                {
                    if (TokenEquals(token, "struct") || TokenEquals(token, "enum"))
                        ParseStructDeclaration(&tokenizer, token);
                    else if (OptionalToken(&tokenizer, OpenParen, 0))
                    {
                        lastToken = ParseFunction(&tokenizer, token);
                        break;
                    }
                }
            }
            default:
            {
                PrintToken(token);
                lastToken = token;
            } break;
        }
    }
#endif
    
    return 0;
}