/*

defines exceptions classes that can be thrown during parse 

why inherit virtual: we want to not have the "diamond scenario"

class a;
class b : public a;
class c : public b, public a;
class c inherits a through b and through itself, how can we resolve which method to call?
solution: class c : public b, virtual public a;
now c inherits a from b but only one instance of a is used resolving the ambiguity

*/

#ifndef PARSE_EXCEPT_H
#define PARSE_EXCEPT_H

#include <exception>
#include <sstream>
#include <string>

#include "../scan/token.h"

class ParseExcept : public virtual std::exception {
public:
	virtual const char * what() const noexcept override {
		return "Exception while parsing";
	}

	virtual void printException(std::ostream& output) const noexcept;
};

class ParseExceptMsg : public virtual ParseExcept {
public:
	ParseExceptMsg(const char * msg) noexcept
	: mMsg(msg) { }
	
	virtual const char * what() const noexcept override {
		return "Exception while parsing w/ message";
	}
	
	virtual void printException(std::ostream& output) const noexcept override;
private:
	const char * mMsg;
};

class FileNotFound : public virtual ParseExcept {
public:
	FileNotFound() noexcept = default;
	~FileNotFound() noexcept = default;

	virtual const char * what() const noexcept override {
		return "File not found";
	}
};

class EOFExcept : public virtual ParseExcept {
public:
	EOFExcept() noexcept = default;
	~EOFExcept() noexcept = default;

	virtual const char * what() const noexcept override {
		return "Unexpected end of file";
	}
};

class UnknownToken : public virtual ParseExcept {
public:
	UnknownToken(std::string& str, int& colNum) noexcept
	: mStr(str)
	, mCol(colNum) { }
	
	virtual ~UnknownToken() override {
		mCol++;
	}
	
	virtual const char * what() const noexcept override {
		return "Unknown token";
	}
	
	virtual void printException(std::ostream& output) const noexcept override;
private:
	std::string& mStr;
	int& mCol;
};

class TokenMismatch : public virtual ParseExcept {
public:
	TokenMismatch(TokenType expected, TokenType actual, std::string& str) noexcept
	: mExpected(expected)
	, mActual(actual)
	, mStr(str) { }
	
	~TokenMismatch() noexcept = default;

	virtual const char * what() const noexcept override {
		return "Token mismatch detected";
	}
	
	virtual void printException(std::ostream& output) const noexcept override;
private:
	TokenType mExpected;
	TokenType mActual;
	std::string& mStr;
};

class OperandMissing : public virtual ParseExcept {
public:
	OperandMissing(TokenType op)
	: mOp(op) { }

	~OperandMissing() noexcept = default;
	
	virtual const char* what() const noexcept override {
		return "Missing binary operand";
	}
	
	virtual void printException(std::ostream& output) const noexcept override;
private:
	TokenType mOp;
};

#endif