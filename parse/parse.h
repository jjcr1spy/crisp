/*
defines the parser i.e. class Parser which is used for recursive descent parsing
*/

#ifndef PARSE_H
#define PARSE_H

#include <memory>
#include <fstream> 
#include <vector> 

#include "symbols.h"
#include "astNodes.h"
#include "../error/parseExcept.h"
#include "../scan/token.h"
#include "../scan/scan.h"

class Parser {	
public:
	// start parsing by calling parseProgram()
	// catch errors that reach constructor 
	// after parsing call displayErrors() to send error messages to stderr
	Parser(Scanner& scanner, const char * fileName, std::ostream * errStream, std::ostream * ASTStream);

	~Parser() noexcept = default;

    bool isValid() const noexcept {
        return mErrors.size() == 0;
    }

    int getNumErrors() const noexcept {
        return mErrors.size();
    }
protected: 
	// these are all the mutually recursive parse functions
	
	// entry point for parser (in parser.cpp)
	std::shared_ptr<ASTProg> parseProgram();
	
	// function definitions and/or forward declarations (in parser.cpp)
	std::shared_ptr<ASTFunc> parseFunction();
	std::shared_ptr<ASTArgDecl> parseArgDecl();

	// declarations (in parseStmt.cpp)
	std::shared_ptr<ASTDecl> parseDecl();

	// statements (in parseStmt.cpp)
	std::shared_ptr<ASTStmt> parseStmt();

	// types of statements that parseStmt considers when parsing (in parseStmt.cpp)
	std::shared_ptr<ASTIfStmt> parseIfStmt();
	std::shared_ptr<ASTForStmt> parseForStmt();
	std::shared_ptr<ASTWhileStmt> parseWhileStmt();
	std::shared_ptr<ASTReturnStmt> parseReturnStmt();
	std::shared_ptr<ASTExprStmt> parseExprStmt();
	std::shared_ptr<ASTNullStmt> parseNullStmt();
	// if the compound statement is a function body then the scope
	// change will happen at a higher level so it should not happen in
	// parseCompoundStmt
	std::shared_ptr<ASTCompoundStmt> parseCompoundStmt(bool isFuncBody = false);

	// expressions (in parseExpr.cpp)
	std::shared_ptr<ASTExpr> parseExpr();
	
	// assignExpr (int parseExpr.cpp)
	std::shared_ptr<ASTExpr> parseAssignExpr();
	std::shared_ptr<ASTAssignOp> parseAssignExprPrime(std::shared_ptr<ASTExpr> lhs);

	// orTerm (int parseExpr.cpp)
	std::shared_ptr<ASTExpr> parseOrTerm();
	std::shared_ptr<ASTLogicalOr> parseOrTermPrime(std::shared_ptr<ASTExpr> lhs);

	// andTerm (in parseExpr.cpp)
	std::shared_ptr<ASTExpr> parseAndTerm();
	std::shared_ptr<ASTLogicalAnd> parseAndTermPrime(std::shared_ptr<ASTExpr> lhs);

	// relExpr (in parseExpr.cpp)
	std::shared_ptr<ASTExpr> parseRelExpr();
	std::shared_ptr<ASTBinaryCmpOp> parseRelExprPrime(std::shared_ptr<ASTExpr> lhs);
	
	// numExpr (in parseExpr.cpp)
	std::shared_ptr<ASTExpr> parseNumExpr();
	std::shared_ptr<ASTBinaryMathOp> parseNumExprPrime(std::shared_ptr<ASTExpr> lhs);
	
	// term (in parseExpr.cpp)
	std::shared_ptr<ASTExpr> parseTerm();
	std::shared_ptr<ASTBinaryMathOp> parseTermPrime(std::shared_ptr<ASTExpr> lhs);
	
	// value (in parseExpr.cpp)
	std::shared_ptr<ASTExpr> parseValue();
	
	// factor (in parseExpr.cpp)
	std::shared_ptr<ASTExpr> parseFactor();
	std::shared_ptr<ASTExpr> parseParenFactor();
	std::shared_ptr<ASTConstantExpr> parseConstantFactor();
	std::shared_ptr<ASTCharExpr> parseCharFactor();
	std::shared_ptr<ASTStringExpr> parseStringFactor();
	std::shared_ptr<ASTDoubleExpr> parseDoubleFactor();

	// parseIdentFactor parses id, id [Expr], id (FunCallArgs), id [Expr] (+=, -=, =) Expr, id (+=, -=, =) Expr
	std::shared_ptr<ASTExpr> parseIdentFactor();
	std::shared_ptr<ASTExpr> parseIncFactor();
	std::shared_ptr<ASTExpr> parseDecFactor();
	std::shared_ptr<ASTExpr> parseAddrOfArrayFactor();
private:
	// helper struct for displaying error messages
	struct Error {
        Error(const std::string& msg, int line, int col)
        : mMsg(msg)
        , mLine(line)
        , mCol(col) { }
        
        std::string mMsg;
        int mLine;
        int mCol;
    };

	// scanner that stores the vector of tokens to parse
	Scanner& mScanner;

	// current token to be considered
	Token& mCurrToken;

	// current index into the scanner vector of tokens
    int mTokenIndex;

	// stores error messages as parsing occurs and is used for outputting after
    std::vector<std::shared_ptr<Error>> mErrors;

	// name of the file we're parsing
	const char * mFileName;

	// ostream exceptions should be output to
	std::ostream * mErrStream;

	// ostream for AST/LLVM bitcode output
	std::ostream * mAstStream;
	
	// SymbolTable corresponding to the parsed file
	SymbolTable mSymbolTable;

	// StringTable for this file
	StringTable mStringTable;

	// tracks the return type of the current function
	Type mCurrReturnType;

    // track whether we need printf
	bool mNeedPrintf;

	// pointer to root node of our program
	std::shared_ptr<ASTProg> mRoot;
    
	// returns true if we are past last scanned token in vector
	bool isAtEnd() const noexcept;

	// advance to next token in vector
	void advance() noexcept;

    // consumes the current token and throws an exception if next token is Unknown 
	void consumeToken(bool unknownBad = true);

    // sees if the token matches the requested
	// if it does it will consume the token and return true otherwise it will return false
	// throws an exception if next token is Unknown
	bool peekAndConsume(TokenType desired) noexcept;

    // returns true if the current token matches one of the tokens in the list.
	bool peekIsOneOf(const std::vector<TokenType>& v) const noexcept;

    // matches the current token against the requested token and consumes it
	// throws an exception if there is a mismatch 
	// should only use this for terminals that are always a specific text
	void matchToken(TokenType desired);

    // matches the current token against the first element
	// in the vector and then consumes and verifies all
	// remaining requested elements
	//
	// throws an exception if there is a mismatch
	// since it throws an exception it should only be used in instances where a
	// specific token order is the only valid match
	// it also throws an exception if the next token is Unknown
	void matchTokenSeq(const std::vector<TokenType>& v);

	// consumes tokens until either a match or EOF is found
	void consumeUntil(TokenType desired) noexcept;

	// consumes tokens until either a match of one in the vector or EOF is found
	void consumeUntil(const std::vector<TokenType>& desired) noexcept;

	// Gets the variable, if it exists. Otherwise
	// reports a semant error and returns @@variable
	Identifier * getVariable(const std::string& name) noexcept;

	// returns a char * that contains the type name
	const char * getTypeText(Type type) const noexcept;

    // helper functions to report syntax errors
	void reportError(const ParseExcept& except) noexcept;

	// helper functions to report syntax errors
	void reportError(const std::string& msg) noexcept;
	
	// helper function to report semantic errors
	void reportSemantError(const std::string& msg, int col = -1) noexcept;

    // write an error message to the error stream
	void displayErrorMsg(const std::string& line, std::shared_ptr<Error> error) noexcept;
	
	// writes out all the error messages
	void displayErrors() noexcept;
};

#endif