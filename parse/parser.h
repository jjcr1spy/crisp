#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <fstream> 
#include <vector> 

#include "types.h"
#include "../scan/token.h"
#include "../scan/scanner.h"
#include "../error/parseExcept.h"
#include "symbols.h"

class Parser {	
public:
	Parser(Scanner& scanner, std::ostream * errStream, std::ostream * ASTStream);
	~Parser() = default;

    bool isValid() const noexcept {
        return mErrors.size() == 0;
    }

    size_t getNumErrors() const noexcept {
        return mErrors.size();
    }
private:
    // Returns the current token
    Token peekToken() const noexcept;

    // Returns the string for the current token's text
    const std::string& getTokenTxt() const noexcept;
    
    // Consumes the current token
	// Throws an exception if next token is Unknown
	// if unknownIsExcept is true
	void consumeToken(bool unknownIsExcept = true);

    // Sees if the token matches the requested.
	// If it does, it'll consume the token and return true
	// otherwise it'll return false
	//
	// Throws an exception if next token is Unknown
	bool peekAndConsume(TokenType desired);

    // Returns true if the current token matches one of the tokens
	// in the list.
	bool peekIsOneOf(const std::vector<TokenType>& v) noexcept;

    // Matches the current token against the requested token,
	// and consumes it.
	//
	// Throws an exception if there is a mismatch.
	//
	// NOTE: You should ONLY use this for terminals that are always a specific text.
	void matchToken(TokenType desired);

    // Matches the current token against the first element
	// in the initializer_list. Then consumes and verifies all
	// remaining requested elements.
	//
	// Throws an exception if there is a mismatch.
	// Since it throws an exception, it should only be used in instances where a
	// specific token order is the ONLY valid match.
	// It also throws an exception of the next token is Unknown
	//
	// NOTE: You should ONLY use this for terminals that are always a specific text.
	// Don't use it for identifier, constant, or string, because you'll have no way to
	// get the text.
	void matchTokenSeq(const std::vector<TokenType>& v);

    // Consumes tokens until either a match or EOF is found
	void consumeUntil(TokenType desired) noexcept;
	
	// consumeUntil for a list of tokens
	void consumeUntil(const std::vector<TokenType>& v) noexcept;

    struct Error {
        Error(const std::string& msg, int line, int col)
        : mMsg(msg)
        , mLine(line)
        , mCol(col) { }
        
        std::string mMsg;
        int mLine;
        int mCol;
    };

    // Helper functions to report syntax errors
	void reportError(const ParseExcept& except) noexcept;
	void reportError(const std::string& msg) noexcept;
	
	// Helper function to report a semantic error
	void reportSemantError(const std::string& msg, int colOverride = -1, int lineOverride = -1) noexcept;

    // Write an error message to the error stream
	void displayErrorMsg(const std::string& line, std::shared_ptr<Error> error) noexcept;
	
	// Writes out all the error messages
	void displayErrors() noexcept;

    Scanner& mScanner;

    // pointer to the root of our AST tree
	// std::shared_ptr<ASTProg> mRoot;
	
	// used to resolve AssignStmt/Factor ambiguity????
	Identifier * unusedIdent;
	//std::shared_ptr<ASTArrSub> unusedArray;
	
	// SymbolTable corresponding to the parsed file
	SymbolTable mSymbolTable;

	// name of the file we're parsing
	const char * mFileName;
	// ostream exceptions should be output to
	std::ostream * mErrStream;
	// ostream for AST output
	std::ostream * mAstStream;
	
	// tracks the return type of the current function
	Type mCurrReturnType;
	
    // keep track of current token
    Token& mCurrToken;
    unsigned int mTokenIndex;

	// keeps track of the line number for error messages
	unsigned int mLine;
	// keeps track of the column number for error messages
	unsigned int mCol;

    std::vector<std::shared_ptr<Error>> mErrors;

    // Track whether we need printf
	bool mNeedPrintf;
};

#endif