#if 0
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

static char* ReadEntireFile(char* fileName)
{
    FILE* file = fopen(fileName, "r");
    char* result = 0;
    if (file)
    {
        fseek(file, 0, SEEK_END);
        size_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        result = (char*)malloc(fileSize);
        fread(result, fileSize, 1, file);
        result[fileSize] = 0;
        
        fclose(file);
    }
    
    return result;
}

static void ParseIntrospectionParams(Tokenizer* tokenizer)
{
    for (;;)
    {
        Token token = GetToken(tokenizer);
        if ((token.type == CloseParen) ||
            (token.type == EndOfStream))
            break;
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
            case Identifier:
            {
                printf("    { Type_%.*s, \"%.*s\", (u32)&((%.*s*)0)->%.*s },\n",
                       memberTypeToken.text.count, memberTypeToken.text.data,
                       token.text.count, token.text.data,
                       structTypeToken.text.count, structTypeToken.text.data,
                       token.text.count, token.text.data);
            } break;
            
            case Semicolon:
            case EndOfStream:
            {
                parsing = false;
            }break;
        }
    }
}

static void ParseStruct(Tokenizer* tokenizer)
{
    Token nameToken = GetToken(tokenizer);
    RequireToken(tokenizer, OpenBraces);
    
    printf("\nMemberDefinition memberOf%.*s[] = \n", nameToken.text.count, nameToken.text.data);
    printf("{\n");
    
    for (;;)
    {
        Token memberToken = GetToken(tokenizer);
        if (memberToken.type == CloseBraces)
            break;
        else
            ParseMember(tokenizer, nameToken, memberToken);
    }
    
    printf("};\n");
}

static void ParseIntrospectable(Tokenizer* tokenizer)
{
    RequireToken(tokenizer, OpenParen);
    
    ParseIntrospectionParams(tokenizer);
    
    Token typeToken = GetToken(tokenizer);
    if (TokenEquals(typeToken, "struct"))
    {
        ParseStruct(tokenizer);
    }
    else
    {
        fprintf(stderr, "ERROR: Introspection is only supported for structs right now :(.\n");
    }
}

int main (int argCount, char** args)
{
    char* fileContents = ReadEntireFile("LongApp_Windows.c");
    
    Tokenizer tokenizer = {0};
    tokenizer.at = fileContents;
    char* at = tokenizer.at;
    
    bool parsing = true;
    Token prevToken  = {0};
    while (parsing)
    {
        Token token = GetToken(&tokenizer);
        switch (token.type)
        {
            case EndOfStream:
            {
                parsing = false;
            } break;
            case Unknown:
            {
            } break;
            
            case Identifier:
            {
#if 0
                if (TokenEquals(token, "introspect"))
                {
                    ParseIntrospectable(&tokenizer);
                }
#endif
                if ((TokenEquals(token, "struct") || TokenEquals(token, "enum")) )//&& !TokenEquals(prevToken, "typedef"))
                {
                    Token nameToken = GetToken(&tokenizer);
                    printf("\ntypedef %.*s %.*s %.*s;\n",
                           token.text.count, token.text.data,
                           nameToken.text.count, nameToken.text.data,
                           nameToken.text.count, nameToken.text.data);
                }
            }
            
            default:
            {
                printf("%.*s", (int)(tokenizer.at - at), at);
                prevToken = token;
                at = tokenizer.at;
            } break;
        }
    }
}
#endif