// @Note: From Handmade Hero.

FUNCTION void update_lexer_at(Lexer *lexer)
{
    // @Note: Our strings are null terminated.
    
    if(lexer->input.count == 0)
    {
        lexer->at[0] = 0;
        lexer->at[1] = 0;
    }
    else
    {
        lexer->at[0] = lexer->input.data[0];
        lexer->at[1] = lexer->input.data[1];
    }
}

FUNCTION void lexer_advance_bytes(Lexer *lexer, u32 count)
{
    lexer->column += count;
    advance_bytes(&lexer->input, count);
    update_lexer_at(lexer);
}

FUNCTION b32 token_match(Token token, char *match)
{
    b32 result = string_match(token.string_value, make_string_from_cstring(match));
    return result;
}

FUNCTION Token get_token_raw(Lexer *lexer)
{
    Token token        = {};
    token.file_name    = lexer->file_name;
    token.column       = lexer->column;
    token.line         = lexer->line;
    token.string_value = lexer->input;
    
    char c = lexer->at[0];
    lexer_advance_bytes(lexer, 1);
    
    switch(c)
    {
        case '\0':
        {
            token.type = TokenType_END_OF_FILE;
        } break;
        
        case '-':
        {
            token.type = TokenType_HYPHEN;
            
            if(is_numeric(lexer->at[0]))
            {
                should_negate = true;
            }
        } break;
        
        case '"':
        {
            token.type = TokenType_STRING;
            
            while((lexer->at[0]) && 
                  (lexer->at[0] != '"'))
            {
                if((lexer->at[0] == '\\') &&
                   (lexer->at[1]))
                    lexer_advance_bytes(lexer, 1);
                
                lexer_advance_bytes(lexer, 1);
            }
            
            if(lexer->at[0] == '"')
            {
                lexer_advance_bytes(lexer, 1);
            }
        } break;
        
        default:
        {
            if(is_spacing(c))
            {
                token.type = TokenType_SPACING;
                
                while(is_spacing(lexer->at[0]))
                    lexer_advance_bytes(lexer, 1);
            }
            else if(is_end_of_line(c))
            {
                token.type = TokenType_END_OF_LINE;
                
                if(((c == '\r') && (lexer->at[0] == '\n')) ||
                   ((c == '\n') && (lexer->at[0] == '\r')))
                    lexer_advance_bytes(lexer, 1);
                
                lexer->column = 1;
                lexer->line++;
            }
            else if((c == '/') && (lexer->at[0] == '/'))
            {
                token.type = TokenType_COMMENT;
                
                lexer_advance_bytes(lexer, 2);
                
                while((lexer->at[0]) && !is_end_of_line(lexer->at[0]))
                    lexer_advance_bytes(lexer, 1);
            }
            else if((c == '/') && (lexer->at[0] == '*'))
            {
                // @Note: Multi-line comment, so we account for new-lines.
                
                token.type = TokenType_COMMENT;
                
                lexer_advance_bytes(lexer, 2);
                while((lexer->at[0]) && 
                      !((lexer->at[0] == '*') && 
                        (lexer->at[1] == '/')))
                {                    
                    if(((lexer->at[0] == '\r') && (lexer->at[1] == '\n')) ||
                       ((lexer->at[0] == '\n') && (lexer->at[1] == '\r')) )
                        lexer_advance_bytes(lexer, 1);
                    
                    if(is_end_of_line(lexer->at[0]))
                        lexer->line++;
                    
                    lexer_advance_bytes(lexer, 1);
                }
                
                if(lexer->at[0] == '*')
                    lexer_advance_bytes(lexer, 2);
            }
            else if(is_alpha(c))
            {
                token.type = TokenType_IDENTIFIER;
                
                while(is_alphanumeric(lexer->at[0]) || (lexer->at[0] == '_'))
                    lexer_advance_bytes(lexer, 1);
            }
            else if(is_numeric(c))
            {
                token.type = TokenType_INTEGER;
                
                f32 number = (f32)(c - '0');
                
                while(is_numeric(lexer->at[0]))
                {
                    f32 digit = (f32)(lexer->at[0] - '0');
                    
                    number    = 10.0f*number + digit;
                    
                    lexer_advance_bytes(lexer, 1);
                }
                
                if(lexer->at[0] == '.')
                {
                    token.type = TokenType_FLOAT;
                    
                    lexer_advance_bytes(lexer, 1);
                    f32 coef = 0.1f;
                    while(is_numeric(lexer->at[0]))
                    {
                        f32 digit = (f32)(lexer->at[0] - '0');
                        
                        number   += coef*digit;
                        coef     *= 0.1f;
                        
                        lexer_advance_bytes(lexer, 1);
                    }
                }
                
                if(should_negate) 
                {
                    number        = -number;
                    should_negate = false;
                }
                
                token.f32_value = number; 
                token.s32_value = _round_f32_to_s32(number);
            }
            else
            {
                token.type = TokenType_ERROR;
            }
        } break;
    }
    
    token.string_value.count = (lexer->input.data - token.string_value.data);
    
    //token.string_value.data[token.string_value.count] = 0;
    //lexer_advance_bytes(lexer, 1);
    
    return token;
}

FUNCTION Token get_token(Lexer *lexer)
{
    Token token;
    
    for(;;)
    {
        token = get_token_raw(lexer);
        
        if((token.type == TokenType_SPACING)     ||
           (token.type == TokenType_END_OF_LINE) ||
           (token.type == TokenType_COMMENT)     ||
           (token.type == TokenType_HYPHEN))
        {
            
        }
        else
        {
            if(token.type == TokenType_STRING)
            {
                if(token.string_value.count &&
                   token.string_value.data[0] == '"')
                {
                    token.string_value.data++;
                    token.string_value.count--;
                }
                
                if(token.string_value.count &&
                   token.string_value.data[token.string_value.count - 1] == '"')
                {
                    token.string_value.count--;
                }
            }
            
            break;
        }
    }
    
    return token;
}

FUNCTION String get_token_type_name(Token_Type type)
{
    switch(type)
    {
        case TokenType_HYPHEN:      {return STRING_LITERAL("hyphen");}
        case TokenType_STRING:      {return STRING_LITERAL("string");}
        case TokenType_IDENTIFIER:  {return STRING_LITERAL("identifier");}
        case TokenType_INTEGER:     {return STRING_LITERAL("integer");}
        case TokenType_FLOAT:       {return STRING_LITERAL("float");}
        case TokenType_SPACING:     {return STRING_LITERAL("spacing");}
        case TokenType_END_OF_LINE: {return STRING_LITERAL("end of line");}
        case TokenType_COMMENT:     {return STRING_LITERAL("comment");}
        case TokenType_END_OF_FILE: {return STRING_LITERAL("end of file");}
    }
    
    return STRING_LITERAL("unknown");
}

FUNCTION Token peek_token(Lexer *lexer)
{
    Lexer temp   = *lexer;
    Token result = get_token(&temp);
    
    return result;
}

FUNCTION void eat_token(Lexer *lexer)
{
    get_token(lexer);
}

FUNCTION Token require_token(Lexer *lexer, Token_Type desired_type)
{
    Token result = get_token(lexer);
    
    if(result.type != desired_type)
    {
        PRINT("Unexpected token in %S::%d::%d! Wanted type \"%S\" but instead got \"%S\"\n", result.file_name, result.line, result.column, get_token_type_name(desired_type), get_token_type_name(result.type));
        
        lexer->error = true;
    }
    
    return result;
}

FUNCTION Lexer make_lexer(String file_name, String input)
{
    Lexer result = {};
    
    result.file_name = file_name;
    result.column    = 1;
    result.line      = 1;
    result.input     = input;
    update_lexer_at(&result);
    
    return result;
}