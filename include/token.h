#ifndef TOKEN_H
#define TOKEN_H 

#include <string>
#include <unordered_map>

enum class TokenType {
    // Literals
    Identifier, CharLit, IntLit, DoubleLit, StringLit,
    
    // Expression Operators
    Assign, Plus, Minus, Mult, Div, Mod, Inc, Dec,
    LBracket, RBracket, EqualTo, NotEqual, Or, And,
    Not, LessThan, GreaterThan, LParen, RParen, Addr,
    IncAssign, DecAssign, MinusAssign, LThanOrEq, GThanOrEq, 

    // Keywords
    KeyFor, KeyWhile, KeyIf, KeyElse, KeyVoid, KeyInt, KeyFloat, KeyChar, KeyDouble,

    // Other
    SemiColon, LBrace, RBrace, Comma, Unknown,
};

class Token {
public:
    Token(TokenType, std::string, int);
    ~Token() {};
    
    friend std::ostream& operator<<(std::ostream& out, const Token& token); // for debugging lexing added cout operator for Token type
private:
    static const std::unordered_map<TokenType, std::string> tokenToString; // store map from TokenType to the its string name -> map[TokenType::...] = "..."

    TokenType type; 
    std::string lexeme; 
    int line; 
};

/*

Lexeme: a sequence of characters in program that matches a pattern
Token: a pair of lexeme and its type

-------------------------------------------------

Literals:

[a-zA-Z_][a-zA-Z0-9_]* -> Identifier
'a'                    -> CharLit
"dsafsdf"              -> StringLit
-123, 2099             -> IntLit
-1000.23, 0.023        -> DoubleLit

-------------------------------------------------

Expression Operators:

"="     -> Assign
"+"     -> Plus
"-"     -> Minus
"*"     -> Mult
"/"     -> Div
"%"     -> Mod
"++"    -> Inc
"--"    -> Dec
"["     -> LBracket
"]"     -> RBracket
"=="    -> EqualTo
"!="    -> NotEqual
"||"    -> Or
"&&"    -> And
"!"     -> Not
"<"     -> LessThan
">"     -> GreaterThan
"("     -> LParen
")"     -> RParen
"&"     -> Addr
"+="    -> IncAssign
"-="    -> MinusAssign
"<="    -> LThanOrEq
">="    -> GThanOrEq

-------------------------------------------------

Keywords:

"for"    -> KeyFor
"while"  -> KeyWhile
"if"     -> KeyIf
"else"   -> KeyElse
"void"   -> KeyVoid
"int"    -> KeyInt
"char"   -> KeyChar
"double" -> KeyDouble

-------------------------------------------------

Other:

";"       -> SemiColon
"{"       -> LBrace
"}"       -> RBrace
","       -> Comma
All else fails -> Unknown

*/

#endif