#include <fstream> 
#include <sstream> 
#include <utility> 
#include <cctype> 
#include <iostream>

#include "token.h"
#include "scan.h"

Scanner::Scanner(const char * src) noexcept
: mSource {}
, mKeywords {
    {"for", TokenType::KeyFor},
    {"while", TokenType::KeyWhile},
    {"if", TokenType::KeyIf},
    {"else", TokenType::KeyElse},
    {"void", TokenType::KeyVoid},
    {"int", TokenType::KeyInt},
    {"char", TokenType::KeyChar},
    {"double", TokenType::KeyDouble},
    {"void", TokenType::KeyVoid}}
, mStart {0}
, mCurrent {0}
, mLine {1} 
, mCol {1} {
    std::ifstream in(src); // open src file

    std::stringstream strStream;
    strStream << in.rdbuf(); // read the file into buffer

    mSource = std::move(strStream.str()); // member source holds the content of the file now

    in.close(); // close src file   
}

bool Scanner::isAtEnd() const noexcept { 
    return mCurrent >= static_cast<int>(mSource.length());
}

char Scanner::advance() noexcept { 
    ++mCol;

    return mSource[mCurrent++];
}

bool Scanner::match(char expected) noexcept {
    if (isAtEnd()) return false;
    if (mSource[mCurrent] != expected) return false;
    
    ++mCol;
    ++mCurrent;

    return true;
}

char Scanner::peek() const noexcept {
    if (isAtEnd()) return '\0';
    return mSource[mCurrent];
} 

char Scanner::peekNext() const noexcept {
    if (mCurrent + 1 >= static_cast<int>(mSource.length())) return '\0';
    return mSource[mCurrent + 1];
} 

void Scanner::identifier() noexcept {
    while (isalnum(peek()) || peek() == '_') {
        advance();
    }

    auto find = mKeywords.find(mSource.substr(mStart, mCurrent - mStart));

    if (find != mKeywords.end()) addToken(find->second);
    else addToken(TokenType::Identifier);
}

void Scanner::character() noexcept {
    while (peek() != '\'' && !isAtEnd()) {        
        advance();
    }

    if (isAtEnd()) {
        addToken(TokenType::Unknown);
        return;
    }

    // the closing '
    advance();

    // trim the surrounding single quotes
    addToken(TokenType::CharLit);
}

void Scanner::string() noexcept { // no support for multi-line strings
    while (peek() != '"' && peek() != '\n' && !isAtEnd()) {    
        advance();
    }

    if (isAtEnd() || peek() == '\n') {
        addToken(TokenType::Unknown);
        return;
    }

    // the closing "
    advance();

    // trim the surrounding quotes
    addToken(TokenType::StringLit);
}

void Scanner::number() noexcept { // dont allow a leading or trailing decimal point
    while (isdigit(peek())) advance();

    // look for a fractional part
    if (peek() == '.' && isdigit(peekNext())) {
        // consume the .
        advance();

        while (isdigit(peek())) advance();

        addToken(TokenType::DoubleLit);
    } else {
        addToken(TokenType::IntLit);
    }
}

void Scanner::addToken(TokenType type) noexcept {
    // want column of start of token so need to subtract length of token str
    mTokens.emplace_back(type, std::move(mSource.substr(mStart, mCurrent - mStart)), mLine, mCol - mCurrent - mStart);
}

void Scanner::scanTokens() noexcept {
    while (!isAtEnd()) { 
        mStart = mCurrent;
        scanToken();
    }

    mTokens.push_back({TokenType::EndOfFile, "", mLine, mStart});
}

void Scanner::scanToken() noexcept {      
    // handle newlines, spaces, and tabs as we dont care about them and check them first as they are common
    while (!isAtEnd() && (mSource[mCurrent] == ' ' || mSource[mCurrent] == '\n' || mSource[mCurrent] == '\t')) {
        ++mCol;
        
        if (mSource[mCurrent] == '\n') {
            ++mLine;
            mCol = 1;
        }

        ++mCurrent;
    }

    // EOF?
    if (isAtEnd()) {
        return;
    }

    mStart = mCurrent;
    char c = advance(); 
    
    switch (c) {
        case '/': // comments
            if (match('/')) {
                while (peek() != '\n' && !isAtEnd()) advance();
            } else {
                addToken(TokenType::Div);
            }
            return;
        case '"': // strings
            string();
            return;
        case '\'': // chars
            character();
            return;
        case '+': // +, ++, +=
            if (match('+')) {
                addToken(TokenType::Inc);
            } else if (match('=')) {
                addToken(TokenType::IncAssign);
            } else {
                addToken(TokenType::Plus);
            }
            return;
        case '-': // -, --, -=, -digit
            if (match('-')) {
                addToken(TokenType::Dec);
            } else if (match('=')) {
                addToken(TokenType::MinusAssign);
            } else if (isdigit(peek())) {
                number();
            } else {
                addToken(TokenType::Minus);
            }
            return;
        default: // positive digit, identifier or keyword -> otherwise unknown
            if (isdigit(c)) {
                number();
            } else if (isalpha(c) || c == '_') {
                identifier();
            } else {
                addToken(TokenType::Unknown);
            }
            return;
    }
}