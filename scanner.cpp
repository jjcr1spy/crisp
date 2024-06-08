#include <fstream> 
#include <sstream> 
#include <utility> 
#include <cctype> 
#include <iostream>

#include "include/token.h"
#include "include/scanner.h"

Scanner::Scanner(char * src) : source {}, keywords {
        {"for", TokenType::KeyFor},
        {"while", TokenType::KeyWhile},
        {"if", TokenType::KeyIf},
        {"else", TokenType::KeyElse},
        {"void", TokenType::KeyVoid},
        {"int", TokenType::KeyInt},
        {"float", TokenType::KeyFloat},
        {"char", TokenType::KeyChar},
        {"double", TokenType::KeyDouble},
        {"void", TokenType::KeyVoid},
    }, tokens {}, start {0}, current {0}, line {1} 
{
    std::ifstream in(src); // open src file

    std::stringstream strStream;
    strStream << in.rdbuf(); // read the file
    source = strStream.str(); // source holds the content of the file

    in.close(); // close src file   
}

Scanner::~Scanner() {};

bool Scanner::isAtEnd() const { 
    return current >= static_cast<int>(source.length());
}

char Scanner::advance() { 
    return source[current++];
}

bool Scanner::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
        
    current++;
    return true;
}

char Scanner::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
} 

char Scanner::peekNext() const {
    if (current + 1 >= static_cast<int>(source.length())) return '\0';
    return source[current + 1];
} 

void Scanner::identifier() {
    while (isalnum(peek()) || peek() == '_') {
        advance();
    }

    auto find = keywords.find(source.substr(start, current - start));

    if (find != keywords.end()) addToken(find->second, start, current);
    else addToken(TokenType::Identifier, start, current);
}

void Scanner::character() {
    while (peek() != '\'' && !isAtEnd()) {        
        advance();
    }

    if (isAtEnd()) {
        addToken(TokenType::Unknown, start, current);
        return;
    }

    // the closing '
    advance();

    // trim the surrounding single quotes
    addToken(TokenType::CharLit, start + 1, current - 1);
}

void Scanner::string() { // support multi-line strings
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        
        advance();
    }

    if (isAtEnd()) {
        addToken(TokenType::Unknown, start, current);
        return;
    }

    // the closing "
    advance();

    // trim the surrounding quotes
    addToken(TokenType::StringLit, start + 1, current - 1);
}

void Scanner::number() { // dont allow a leading or trailing decimal point
    while (isdigit(peek())) advance();

    // look for a fractional part
    if (peek() == '.' && isdigit(peekNext())) {
        // consume the .
        advance();

        while (isdigit(peek())) advance();

        addToken(TokenType::DoubleLit, start, current);
    } else {
        addToken(TokenType::IntLit, start, current);
    }
}

void Scanner::addToken(TokenType type, int start, int end) { 
    std::string s {source.substr(start, end - start)}; 
    Token toAdd {type, std::move(s), line}; 
    tokens.push_back(std::move(toAdd));

    std::cout << toAdd << '\n';
}

void Scanner::scanTokens() {
    while (!isAtEnd()) { 
        start = current;
        scanToken();
    }
}

void Scanner::scanToken() {      
    // handle newlines, spaces, and tabs as we dont care about them and check them first as they are common
    while (!isAtEnd() && (source[current] == ' ' || source[current] == '\n' || source[current] == '\t')) {
        if (source[current] == '\n') line++;

        current++;
    }

    if (isAtEnd()) { // if at end of file
        return;
    }

    start = current;
    char c = advance(); 
    
    switch (c) {
        case '/': // handle comments, only support // at the moment
            if (match('/')) {
                while (peek() != '\n' && !isAtEnd()) advance();
            } else {
                addToken(TokenType::Div, start, current);
            }
            break;
        case '"': // for strings
            string();
            break;
        case '\'': // for chars
            character();
            break;
        case '+': 
            if (match('+')) {
                addToken(TokenType::Inc, start, current);
            } else if (match('=')) {
                addToken(TokenType::IncAssign, start, current);
            } else {
                addToken(TokenType::Plus, start, current);
            }
            break;
        case '-':
            if (match('-')) {
                addToken(TokenType::Dec, start, current);
            } else if (match('=')) {
                addToken(TokenType::MinusAssign, start, current);
            } else if (isdigit(peek())) {
                number();
            } else {
                addToken(TokenType::Minus, start, current);
            }
            break;
        case '!':
            if (match('=')) {
                addToken(TokenType::NotEqual, start, current);
            } else {
                addToken(TokenType::Not, start, current);
            }
            break;
        case '<':
            if (match('=')) {
                addToken(TokenType::LThanOrEq, start, current);
            } else {
                addToken(TokenType::LessThan, start, current);
            }
            break;
        case '>':
            if (match('=')) {
                addToken(TokenType::GThanOrEq, start, current);
            } else {
                addToken(TokenType::GreaterThan, start, current);
            }
            break;
        case '&':
            if (match('&')) {
                addToken(TokenType::And, start, current);
            } else {
                addToken(TokenType::Addr, start, current);
            }
            break;
        case '|':
            if (match('|')) {
                addToken(TokenType::Or, start, current);
            } else {
                addToken(TokenType::Unknown, start, current);
            }
            break;
        case '=':
            if (match('=')) {
                addToken(TokenType::EqualTo, start, current);
            } else {
                addToken(TokenType::Assign, start, current);
            }
            break;
        case '[':
            addToken(TokenType::LBracket, start, current);
            break;
        case ']':
            addToken(TokenType::RBracket, start, current);
            break;
        case '*':
            addToken(TokenType::Mult, start, current);
            break;
        case '%':
            addToken(TokenType::Mod, start, current);
            break;
        case ';':
            addToken(TokenType::SemiColon, start, current);
            break;
        case '{':
            addToken(TokenType::LBrace, start, current);
            break;
        case '}':
            addToken(TokenType::RBrace, start, current);
            break;
        case ',':
            addToken(TokenType::Comma, start, current);
            break;
        case '(':
            addToken(TokenType::LParen, start, current);
            break;
        case ')':
            addToken(TokenType::RParen, start, current);
            break;
        default:
            if (isdigit(c)) {
                number();
            } else if (isalpha(c) || c == '_') {
                identifier();
            } else {
                addToken(TokenType::Unknown, start, current);
            }
            break;
    }
}     