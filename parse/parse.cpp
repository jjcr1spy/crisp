#include <iostream>
#include "../scan/scan.h"
#include "../error/parseExcept.h"
#include "astNodes.h"
#include "symbols.h"
#include "parse.h"

Parser::Parser(Scanner& scanner, SymbolTable& table, StringTable& strings, const char * fileName, std::ostream * errStream, std::ostream * astStream) 
: mScanner {scanner}
, mCurrToken {scanner.mTokens[0]}
, mTokenIndex {0}
, mErrors {}
, mFileName {fileName}
, mErrStream {errStream}
, mAstStream {astStream}
, mSymbolTable {table}
, mStringTable {strings}
, mCurrReturnType {Type::Void}
, mNeedPrintf {false} {
    try {
		mRoot = parseProgram();
    } catch (ParseExcept& e) {
        reportError(e);
    }

    if (!isValid()) {
        displayErrors();
    }
}

/* 
--------------------------------------------------------------------------------------------------------------
methods used for error messages when parsing grammar and performing semantic analysis
*/

void Parser::reportError(const ParseExcept& except) noexcept {
	std::stringstream errStrm;
	except.printException(errStrm);
	mErrors.push_back(std::make_shared<Error>(errStrm.str(), mCurrToken.mLine, mCurrToken.mCol));
}

void Parser::reportError(const std::string& msg) noexcept {
	mErrors.push_back(std::make_shared<Error>(msg, mCurrToken.mLine, mCurrToken.mCol));
}

void Parser::reportSemantError(const std::string& msg, int col) noexcept {
	int c = (col == -1) ? mCurrToken.mCol : col;

    mErrors.push_back(std::make_shared<Error>(msg, mCurrToken.mLine, c));
}

void Parser::displayErrorMsg(const std::string& line, std::shared_ptr<Error> error) noexcept {
    (*mErrStream) << mFileName << ":" << error->mLine << ":" << error->mCol;
	(*mErrStream) << ": error: ";
	(*mErrStream) << error->mMsg << std::endl;
	
	(*mErrStream) << line << std::endl;
	
    // now add the caret
	for (int i = 0; i < error->mCol - 1; ++i) {
		if (line[i] == '\t') (*mErrStream) << '\t';
		else (*mErrStream) << ' ';
	}

	(*mErrStream) << '^' << std::endl;
}

void Parser::displayErrors() noexcept {
	int lineNum = 0;
	std::string lineTxt;
	std::ifstream fileStream(mFileName);
	
	for (auto i = mErrors.begin(); i != mErrors.end(); ++i) {
		while (lineNum < (*i)->mLine) {
			std::getline(fileStream, lineTxt);
			lineNum++;
		}
		
		displayErrorMsg(lineTxt, *i);
	}
}

const char * Parser::getTypeText(Type type) const noexcept {
	switch (type) {
		case Type::Char:
			return "char";
		case Type::Int:
			return "int";
		case Type::Double:
			return "double";
		case Type::Void:
			return "void";
		case Type::CharArray:
			return "char[]";
		case Type::IntArray:
			return "int[]";
		case Type::DoubleArray:
			return "double[]";
		case Type::Function:
			return "function";
	}

	return nullptr;
}

/* 
--------------------------------------------------------------------------------------------------------------
helper methods for parsing grammar 
*/

Identifier * Parser::getVariable(const std::string& name) noexcept {
	Identifier * ident = mSymbolTable.getIdentifier(name);

	if (!ident) {
		reportSemantError("Use of undeclared identifier '" + name + "'");
		return mSymbolTable.getIdentifier("@@variable");
	}
	
	return ident;
}

// returns true if we are past last scanned token in vector
bool Parser::isAtEnd() const noexcept {
	return mCurrToken.mType == TokenType::EndOfFile;
}

// advance to next token in vector
void Parser::advance() noexcept {
	if (isAtEnd()) return;
	
	mCurrToken = mScanner.mTokens[++mTokenIndex];
}

// consumes the current token if arg is set to false then we throw exception for TokenType::Unkown
// throws an exception if next token is Unknown and consumes it
void Parser::consumeToken(bool unknownBad) {
	advance();

	if (unknownBad && mCurrToken.mType == TokenType::Unknown) {
		throw UnknownToken(mCurrToken.mStr, mCurrToken.mCol);
		consumeToken();
	}
}

// sees if the token matches the requested
// if it does it will consume the token and return true otherwise it will return false
// throws an exception if next token is Unknown
bool Parser::peekAndConsume(TokenType desired) noexcept {
	if (mCurrToken.mType == desired) {
		consumeToken();
		return true;
	}

	return false;
}

// returns true if the current token matches one of the tokens in the list
bool Parser::peekIsOneOf(const std::vector<TokenType>& v) const noexcept {	
	for (TokenType t : v) {
		if (mCurrToken.mType == t) return true;
	}

	return false;
}

// matches the current token against the requested token and consumes it
// throws an exception if there is a mismatch 
// should only use this for terminals that are always a specific text
void Parser::matchToken(TokenType desired) {
	if (!peekAndConsume(desired)) {
		throw TokenMismatch(desired, mCurrToken.mType, mCurrToken.mStr);
	}
}

// matches the current token against the first element
// in the vector and then consumes and verifies all
// remaining requested elements
//
// throws an exception if there is a mismatch
// since it throws an exception it should only be used in instances where a
// specific token order is the only valid match
// it also throws an exception if the next token is Unknown
void Parser::matchTokenSeq(const std::vector<TokenType>& v) {	
	for (TokenType t : v) {
		if (!peekAndConsume(t)) {
			throw TokenMismatch(t, mCurrToken.mType, mCurrToken.mStr);
		}
	}
}

// consumes tokens until either a match or EOF is found
void Parser::consumeUntil(TokenType desired) noexcept {	
	while (mCurrToken.mType != TokenType::EndOfFile && mCurrToken.mType != desired) {
		consumeToken(false);
	}
}

// same thing as above but matches w a list of TokenTypes
void Parser::consumeUntil(const std::vector<TokenType>& desired) noexcept {
	if (mCurrToken.mType == TokenType::EndOfFile) return;
	
	while (mCurrToken.mType != TokenType::EndOfFile) {
		for (TokenType t : desired) {
			if (mCurrToken.mType == t) {
				return;
			}
		}

		consumeToken(false);
	}
}

/* 
--------------------------------------------------------------------------------------------------------------
methods for recursive descent parsing other methods are in parse.cpp
*/

std::shared_ptr<ASTProg> Parser::parseProgram() {
	// create our base program node
	std::shared_ptr<ASTProg> retVal = std::make_shared<ASTProg>();
	
	// parse our function 
	std::shared_ptr<ASTFunc> func = parseFunction();
	
	while (func) {
		retVal->addFunction(func);
		func = parseFunction();
	}
	
	if (mCurrToken.mType != TokenType::EndOfFile) {
		reportError("Expected end of file");
	}
	
	// if (isValid()) {
	// 	retVal->printNode((*mAstStream));
	// 	*mAstStream << std::endl; 

	// 	mSymbolTable.print((*mAstStream));
	// 	*mAstStream << std::endl; 
	// }
	
	return retVal;
}

std::shared_ptr<ASTFunc> Parser::parseFunction() {
	std::shared_ptr<ASTFunc> retVal;

	if (peekIsOneOf({TokenType::KeyVoid, TokenType::KeyDouble, TokenType::KeyInt, TokenType::KeyChar})) {		
		Type retType;

		switch (mCurrToken.mType) {
			case TokenType::KeyInt:
				retType = Type::Int;
				break;
			case TokenType::KeyDouble:
				retType = Type::Double;
				break;
			case TokenType::KeyChar:
				retType = Type::Char;
				break;
			case TokenType::KeyVoid:
				retType = Type::Void;
				break;
			default:
				break;
		}

		mCurrReturnType = retType;		

		consumeToken();

		// add a useful message if they are trying to return an array which crisp doesnt allow
		if (peekAndConsume(TokenType::LBracket)) {
			reportSemantError("crisp does not allow return of array types");

			consumeUntil(TokenType::RBracket);

			if (mCurrToken.mType == TokenType::EndOfFile) {
				throw EOFExcept();
			}
			
			matchToken(TokenType::RBracket);
		}

		Identifier * ident = nullptr;
		if (mCurrToken.mType != TokenType::Identifier) { 
			std::string err = "Function name ";
			err += mCurrToken.mStr;
			err += " is invalid";

			reportError(err);

			// set to a bogus debug symbol so the parse continues
			ident = mSymbolTable.getIdentifier("@@function");

			// skip until the open parenthesis
			consumeUntil(TokenType::LParen);

			if (mCurrToken.mType == TokenType::EndOfFile) {
				throw EOFExcept();
			}
		} else {
			if (mSymbolTable.isDeclaredInScope(mCurrToken.mStr)) {
				// invalid redeclaration
				std::string err = "Invalid redeclaration of function '";
				err += mCurrToken.mStr;
				err += '\'';

				reportSemantError(err);

				ident = mSymbolTable.getIdentifier("@@function");
			} else {
				ident = mSymbolTable.createIdentifier(mCurrToken.mStr);
				ident->setType(Type::Function);
				
				if (ident->getName() == "main" && retType != Type::Int) {
					reportSemantError("Function 'main' must return an int");
				}
			}

			consumeToken();
		}

		// once we are here it is time to enter the scope of the function
		// since arguments count as the functions main body scope
		ScopeTable * table = mSymbolTable.enterScope();

		retVal = std::make_shared<ASTFunc>(*ident, retType, *table);
		
		if (!ident->isDummy()) {
			ident->setFunction(retVal);
		}

		if (peekAndConsume(TokenType::LParen)) {
			try {
				std::shared_ptr<ASTArgDecl> arg = parseArgDecl();

				while (arg) {
					retVal->addArg(arg);

					if (peekAndConsume(TokenType::Comma)) {
						arg = parseArgDecl();

						if (!arg) throw ParseExceptMsg("Additional function argument must follow a comma");
					} else {
						break;
					}
				}
			} catch (ParseExcept& e) {
				reportError(e);

				consumeUntil(TokenType::RParen);

				if (mCurrToken.mType == TokenType::EndOfFile) {
					throw EOFExcept();
				}
			}
			
			matchToken(TokenType::RParen);

			if (ident->getName() == "main" && retVal->getNumArgs() != 0) {
				reportSemantError("Function 'main' cannot take any arguments");
			}
		} else {
			std::string err = "Missing argument declaration for function ";
			err += ident->getName();

			reportError(err);

			// skip until the compound stmt
			consumeUntil(TokenType::LBrace);

			if (mCurrToken.mType == TokenType::EndOfFile) {
				throw EOFExcept();
			}
		}

		// Grab the compound statement for this function
		std::shared_ptr<ASTCompoundStmt> funcCompoundStmt;
		try {
			funcCompoundStmt = parseCompoundStmt(true);
		} catch (ParseExcept& e) {
			// something bad happened here
			reportError(e);

			// skip all the tokens until the } brace
			consumeUntil(TokenType::RBrace);

			if (mCurrToken.mType == TokenType::EndOfFile) {
				throw EOFExcept();
			}

			consumeToken();
		}

		// exit the scope but before we potentially throw out of this function for a non-EOF message
		mSymbolTable.exitScope();

		if (!funcCompoundStmt) {
			throw ParseExceptMsg("Function implementation missing");
		}

		// add the compound statement to this function
		retVal->setBody(funcCompoundStmt);
	}

	return retVal;
}

std::shared_ptr<ASTArgDecl> Parser::parseArgDecl() {
	std::shared_ptr<ASTArgDecl> retVal;
	
	if (peekIsOneOf({TokenType::KeyDouble, TokenType::KeyInt, TokenType::KeyChar})) {
		Type varType = Type::Void;

		switch (mCurrToken.mType) {
			case TokenType::KeyDouble:
				varType = Type::Double;
				break;
			case TokenType::KeyInt:
				varType = Type::Int;
				break;
			case TokenType::KeyChar:
				varType = Type::Char;
				break;
			default:
				break;
		}
		
		consumeToken();
		
		if (mCurrToken.mType != TokenType::Identifier) {
			throw ParseExceptMsg("Unnamed function parameters are not allowed");
		}
		
		// set it to the default "error" until we see if this is a new identifier
		Identifier * ident = mSymbolTable.getIdentifier("@@variable");

		if (mSymbolTable.isDeclaredInScope(mCurrToken.mStr)) {
			std::string errMsg("Invalid redeclaration of argument '");
			errMsg += mCurrToken.mStr;
			errMsg += '\'';

			// leave at @@variable
		} else {
			ident = mSymbolTable.createIdentifier(mCurrToken.mStr);
		}
		
		consumeToken();
		
		// is this an array type?
		if (peekAndConsume(TokenType::LBracket)) {
			matchToken(TokenType::RBracket);

			if (varType == Type::Int) varType = Type::IntArray;
			else if (varType == Type::Char) varType = Type::CharArray;
			else varType = Type::DoubleArray;
		}

		ident->setType(varType);
		
		retVal = std::make_shared<ASTArgDecl>(*ident);
	}
	
	return retVal;
}


