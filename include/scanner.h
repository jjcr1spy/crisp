#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include <unordered_map> 

enum class TokenType; // defined in token.h
class Token;          // defined in token.h

class Scanner {
public:
    Scanner(char *);
    ~Scanner();

    void scanTokens(); // scan all tokens from input file by invoking scanToken iteratively
private:
    std::string source; // src file read into this string
    std::unordered_map <std::string, TokenType> keywords; // store the letter only keywords 
    std::vector<Token> tokens; // tokens in order of being read in from input file

    int start; // points to first character of lexeme
    int current; // points to current character being considered
    int line; // line number we are at
    
    char advance(); // consume current char and advance
    bool match(char); // for cases like + and ++, match determines + or ++ to be next token  

    void scanToken(); // scan the next token and call addToken to insert
    void addToken(TokenType, int, int); // insert found token into member vector tokens

    void character(); // for chars: 'a', '1'
    void string(); // for strings: "hello"
    void number(); // for int/double: 1.2, 20, -10, -1.50
    void identifier(); // for identifiers + keywords with letters only

    bool isAtEnd() const; // if at end of file
    char peek() const; // look at source[current] character in file
    char peekNext() const; // look at source[current + 1] character in file
};

#endif