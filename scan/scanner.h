#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include <unordered_map> 

// in token token.h
enum class TokenType; 
class Token;     

class Scanner {
public:
    // allow Parser to access all of private methods/members to implement recursive descent functions 
    friend class Parser;
    
    // constructor/destructor 
    Scanner(const char *) noexcept;
    // members used only STL stuff which has mem management for me
    ~Scanner() noexcept = default;

    // scan all tokens by invoking scanToken iteratively which calls addToken 
    void scanTokens() noexcept;
private:
    // source file read into this string
    std::string mSource; 
    // store keywords 
    std::unordered_map<std::string, TokenType> mKeywords; 
    // tokens in order of being read in from input file
    std::vector<Token> mTokens; 

    // first character of lexeme
    int mStart; 
    // current character being considered
    int mCurrent; 
    // curr line number 
    int mLine;
    
    // consume current char and advance
    char advance() noexcept; 
    // for cases like + and ++, match determines + or ++ to be next token  
    bool match(char) noexcept; 

    // insert token into member vector tokens
    void addToken(TokenType) noexcept;
    // scan next token by eventually calling addToken
    void scanToken() noexcept; 

    // for chars: 'a', '1'
    void character() noexcept;
    // for strings: "hello" 
    void string() noexcept; 
    // for int or double: 1.2, 20, -10, -1.50
    void number() noexcept; 
    // identifiers + keywords  
    void identifier() noexcept; 

    // EOF?
    bool isAtEnd() const noexcept; 
    // source[current] character in file
    char peek() const noexcept; 
    // source[current + 1] character in file
    char peekNext() const noexcept; 

    // functions below are for parsing functions in Parser
    
};

#endif
