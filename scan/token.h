/*
defines the types of tokens i.e. enum class TokenType and a wrapper class i.e Token for each token parsed to store other useful info
*/

#ifndef TOKEN_H
#define TOKEN_H 

#include <string>
#include <unordered_map>

enum class TokenType {
    // literals
    Identifier, CharLit, IntLit, DoubleLit, StringLit,
    
    // expression operators
    Assign, Plus, Minus, Mult, Div, Mod, Inc, Dec,
    LBracket, RBracket, EqualTo, NotEqual, Or, And,
    Not, LessThan, GreaterThan, LParen, RParen, Addr,
    IncAssign, DecAssign, MinusAssign, LThanOrEq, GThanOrEq, 

    // keywords
    KeyFor, KeyWhile, KeyIf, KeyElse, KeyVoid, KeyInt, KeyChar, KeyDouble,

    // other
    SemiColon, LBrace, RBrace, Comma, Unknown, EndOfFile
};

class Token {
public:
    // allow Parser to access all of private methods/members 
    friend class Parser;

    // store map from TokenType to the its string name -> map[TokenType::...] = "..."
    static std::unordered_map<TokenType, std::string> mToString;

    Token(TokenType type, std::string&& str, int line, int pos) noexcept;

    // only using STL stuff which has mem management for me
    ~Token() noexcept = default;
private:
    // token type
    TokenType mType;

    // string of TokenType from source file
    std::string mStr; 

    // line number (for error messages down the line)
    int mLine; 

    // column number (for error messages down the line)
    int mCol;
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
EOF       -> EndOfFile
All else fails -> Unknown

*/

#endif