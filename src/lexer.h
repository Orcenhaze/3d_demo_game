#ifndef LEXER_H
#define LEXER_H

enum Token_Type
{
    TokenType_ERROR,
    
    TokenType_HYPHEN,
    
    TokenType_STRING,
    TokenType_IDENTIFIER,
    TokenType_INTEGER,
    TokenType_FLOAT,
    
    TokenType_SPACING,
    TokenType_END_OF_LINE,
    TokenType_COMMENT,
    
    TokenType_END_OF_FILE
};

b32 should_negate;

struct Token
{
    String file_name;
    s32    column;
    s32    line;
    
    Token_Type type;
    
    String string_value;
    f32    f32_value;
    s32    s32_value;
};

struct Lexer
{
    String file_name;
    s32    column;
    s32    line;
    
    String input;
    char   at[2]; // The current and next character in input.
    
    b32 error;
};

#endif //LEXER_H
