#include <iostream> 
#include "token.h"

std::unordered_map<TokenType, std::string> Token::mToString = {
    // literals
    {TokenType::Identifier, "identifier"},
    {TokenType::CharLit, "char"},
    {TokenType::IntLit, "int"},
    {TokenType::DoubleLit, "double"},
    {TokenType::StringLit, "string"},

    // expression operators
    {TokenType::Assign, "="},
    {TokenType::Plus, "+"},
    {TokenType::Minus, "-"},
    {TokenType::Mult, "*"},
    {TokenType::Div, "/"},
    {TokenType::Mod, "%"},
    {TokenType::Inc, "++"},
    {TokenType::Dec, "--"},
    {TokenType::LBracket, "["},
    {TokenType::RBracket, "]"},
    {TokenType::EqualTo, "=="},
    {TokenType::NotEqual, "!="},
    {TokenType::Or, "||"},
    {TokenType::And, "&&"},
    {TokenType::Not, "!"},
    {TokenType::LessThan, "<"},
    {TokenType::GreaterThan, ">"},
    {TokenType::LParen, "("},
    {TokenType::RParen, ")"},
    {TokenType::Addr, "&"},
    {TokenType::IncAssign, "+="},
    {TokenType::DecAssign, "-="},
    {TokenType::MinusAssign, "*="},
    {TokenType::LThanOrEq, "<="},
    {TokenType::GThanOrEq, ">="},

    // keywords
    {TokenType::KeyFor, "for"},
    {TokenType::KeyWhile, "while"},
    {TokenType::KeyIf, "if"},
    {TokenType::KeyElse, "else"},
    {TokenType::KeyVoid, "void"},
    {TokenType::KeyInt, "int"},
    {TokenType::KeyChar, "char"},
    {TokenType::KeyDouble, "double"},
    {TokenType::KeyReturn, "return"},
    
    // other
    {TokenType::SemiColon, ";"},
    {TokenType::LBrace, "{"},
    {TokenType::RBrace, "}"},
    {TokenType::Comma, ","},
    {TokenType::Unknown, "Unknown"},
    {TokenType::EndOfFile, "EOF"},
};

Token::Token(TokenType t, std::string&& s, int l, int c) noexcept
: mType {t}
, mStr {std::move(s)}
, mLine {l}
, mCol {c} { }



