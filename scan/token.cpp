#include <iostream> 
#include "include/token.h"

const std::unordered_map<TokenType, std::string> Token::tokenToString = {
    // Literals
    {TokenType::Identifier, "Identifier"},
    {TokenType::CharLit, "CharLit"},
    {TokenType::IntLit, "IntLit"},
    {TokenType::DoubleLit, "DoubleLit"},
    {TokenType::StringLit, "StringLit"},

    // Expression Operators
    {TokenType::Assign, "Assign"},
    {TokenType::Plus, "Plus"},
    {TokenType::Minus, "Minus"},
    {TokenType::Mult, "Mult"},
    {TokenType::Div, "Div"},
    {TokenType::Mod, "Mod"},
    {TokenType::Inc, "Inc"},
    {TokenType::Dec, "Dec"},
    {TokenType::LBracket, "LBracket"},
    {TokenType::RBracket, "RBracket"},
    {TokenType::EqualTo, "EqualTo"},
    {TokenType::NotEqual, "NotEqual"},
    {TokenType::Or, "Or"},
    {TokenType::And, "And"},
    {TokenType::Not, "Not"},
    {TokenType::LessThan, "LessThan"},
    {TokenType::GreaterThan, "GreaterThan"},
    {TokenType::LParen, "LParen"},
    {TokenType::RParen, "RParen"},
    {TokenType::Addr, "Addr"},
    {TokenType::IncAssign, "IncAssign"},
    {TokenType::DecAssign, "DecAssign"},
    {TokenType::MinusAssign, "MinusAssign"},
    {TokenType::LThanOrEq, "LThanOrEq"},
    {TokenType::GThanOrEq, "GThanOrEq"},

    // Keywords
    {TokenType::KeyFor, "KeyFor"},
    {TokenType::KeyWhile, "KeyWhile"},
    {TokenType::KeyIf, "KeyIf"},
    {TokenType::KeyElse, "KeyElse"},
    {TokenType::KeyVoid, "KeyVoid"},
    {TokenType::KeyInt, "KeyInt"},
    {TokenType::KeyChar, "KeyChar"},
    {TokenType::KeyDouble, "KeyDouble"},

    // Other
    {TokenType::SemiColon, "SemiColon"},
    {TokenType::LBrace, "LBrace"},
    {TokenType::RBrace, "RBrace"},
    {TokenType::Comma, "Comma"},
    {TokenType::Unknown, "Unknown"}
};

Token::Token(TokenType t, std::string s, int l) : type {t}, lexeme {s}, line {l} {};

std::ostream& operator<<(std::ostream& out, const Token& token) {
    std::cout << "Token (" <<  token.tokenToString.at(token.type) << ", \"" << token.lexeme << "\")";

    return out;
}


