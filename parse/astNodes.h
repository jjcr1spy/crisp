/*
defines all AST nodes used in the recursive descent parsing
*/

#ifndef ASTNODES_H
#define ASTNODES_H

#include <memory>
#include <vector>
#include "../scan/token.h"
#include "types.h"
#include "symbols.h"

// each printNode method defined in printNodes.cpp
// each finalizeOp method for each op defined in astNodes.cpp
// each codegen method defined in ../emitIR/astEmit.cpp

// forward declare llvm Value to make compiler happy 
namespace llvm {
	class Value;
}

// in ../emitIr/emitter.h
class CodeContext;

// defined below just need to forward declare for compiler
class ASTFunc;
class ASTArgDecl;
class ASTCompoundStmt;
class ASTExpr;

class ASTNode {
public:
	// virtual so subclass deconstructors get called too
    virtual ~ASTNode() noexcept = default;

	// each node should have a printNode method to make sure AST is well formed
	virtual void printNode(std::ostream& output, int depth = 0) const noexcept = 0;	

	// each node should have a codegen method to output LLVM LIR  
	virtual llvm::Value * codegen(CodeContext& context) noexcept = 0; 
protected:
	ASTNode() noexcept = default;
};

class ASTProg : public ASTNode { 
public:
	ASTProg() noexcept = default;
	~ASTProg() noexcept = default;

	// defined in astNodes.cpp
    void addFunction(std::shared_ptr<ASTFunc> func) noexcept;

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override; 
private: 
    std::vector<std::shared_ptr<ASTFunc>> mFuncs;
};

class ASTFunc : public ASTNode {
public:
    ASTFunc(Identifier& ident, Type returnType, ScopeTable& scopeTable) noexcept
    : mIdent {ident}
    , mReturnType {returnType}
    , mScopeTable {scopeTable} { }

	~ASTFunc() noexcept = default;

	// defined in astNodes.cpp
    void addArg(std::shared_ptr<ASTArgDecl> arg) noexcept;
    void setBody(std::shared_ptr<ASTCompoundStmt> body) noexcept;
    bool checkArgType(int argNum, Type type) const noexcept;
    Type getArgType(int argNum) const noexcept;

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;

    Type getReturnType() const noexcept {
        return mReturnType;
    }

    int getNumArgs() const noexcept {
        return mArgs.size();
    }
protected:
    std::vector<std::shared_ptr<ASTArgDecl>> mArgs;
    std::shared_ptr<ASTCompoundStmt> mBody;
private:
    Identifier& mIdent;
    Type mReturnType;
    ScopeTable& mScopeTable;
};

class ASTArgDecl : public ASTNode {
public:
	ASTArgDecl(Identifier& ident) noexcept
	: mIdent {ident} { }
	
	~ASTArgDecl() noexcept = default;

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;

	Type getType() const noexcept {
		return mIdent.getType();
	}
	
	Identifier& getIdent() noexcept {
		return mIdent;
	}	
private:
	Identifier& mIdent;
};

/*
------------------------------------------------------
statements 
*/ 

// use as base class so no need to implement printNode since derived class will implement printNode
class ASTStmt : public ASTNode {
public:
    virtual ~ASTStmt() noexcept = default;
protected:
	ASTStmt() noexcept = default;
};

class ASTDecl : public ASTStmt {
public:
	ASTDecl(Identifier& ident, std::shared_ptr<ASTExpr> expr = nullptr) noexcept
	: mIdent {ident}
	, mExpr {expr} { }

	~ASTDecl() noexcept = default;

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	Identifier& mIdent;
	std::shared_ptr<ASTExpr> mExpr;
};

class ASTCompoundStmt : public ASTStmt {
public:
	ASTCompoundStmt() noexcept = default;
	~ASTCompoundStmt() noexcept = default;

	// defined in astNodes.cpp
	void addStmt(std::shared_ptr<ASTStmt> stmt) noexcept;

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	std::vector<std::shared_ptr<ASTStmt>> mStmts;
};

class ASTIfStmt : public ASTStmt {
public:
	ASTIfStmt(std::shared_ptr<ASTExpr> expr, std::shared_ptr<ASTStmt> thenStmt, std::shared_ptr<ASTStmt> elseStmt = nullptr) noexcept
	: mExpr {expr}
	, mThenStmt {thenStmt}
	, mElseStmt {elseStmt} { }

	~ASTIfStmt() noexcept = default;

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	std::shared_ptr<ASTExpr> mExpr;
	std::shared_ptr<ASTStmt> mThenStmt;
	std::shared_ptr<ASTStmt> mElseStmt;
};

class ASTReturnStmt : public ASTStmt {
public:
	ASTReturnStmt(std::shared_ptr<ASTExpr> expr) noexcept
	: mExpr {expr} { }

	~ASTReturnStmt() noexcept = default;

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	std::shared_ptr<ASTExpr> mExpr;
};

class ASTForStmt : public ASTStmt {
public:
	ASTForStmt(std::shared_ptr<ASTStmt> varDef, std::shared_ptr<ASTStmt> cond, std::shared_ptr<ASTExpr> update, std::shared_ptr<ASTStmt> loopStmt) noexcept
    : mVarDecl {varDef}
    , mExprCond {cond}
    , mUpdateStmt {update}
	, mLoopBody {loopStmt} { }

	~ASTForStmt() noexcept = default;

	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
    std::shared_ptr<ASTStmt> mVarDecl;
	std::shared_ptr<ASTStmt> mExprCond;
    std::shared_ptr<ASTExpr> mUpdateStmt;
	std::shared_ptr<ASTStmt> mLoopBody;
};

class ASTWhileStmt : public ASTStmt {
public:
	ASTWhileStmt(std::shared_ptr<ASTExpr> expr, std::shared_ptr<ASTStmt> loopStmt) noexcept
	: mExpr {expr}
	, mLoopStmt {loopStmt} { }

	~ASTWhileStmt() noexcept = default;

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	std::shared_ptr<ASTExpr> mExpr;
	std::shared_ptr<ASTStmt> mLoopStmt;
};

class ASTExprStmt : public ASTStmt {
public:
	ASTExprStmt(std::shared_ptr<ASTExpr> expr) noexcept
	: mExpr(expr) { }

	~ASTExprStmt() noexcept = default;

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	std::shared_ptr<ASTExpr> mExpr;
};

class ASTNullStmt : public ASTStmt {
public:
	ASTNullStmt() noexcept = default;
	~ASTNullStmt() noexcept = default;

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
};

/*
------------------------------------------------------
expressions
*/ 

// use as base class so no need to implement printNode since derived class will implement printNode
class ASTExpr : public ASTNode {
public:	
	virtual ~ASTExpr() noexcept = default;

	Type getType() const noexcept {
		return mType;
	}
protected:
	ASTExpr() noexcept
	: mType {Type::Void} { }

	// all expressions have a type
	Type mType;
};

// id
class ASTIdentExpr : public ASTExpr {
public:
	ASTIdentExpr(Identifier& ident) noexcept
	: mIdent {ident} {
		mType = mIdent.getType();
	}

	~ASTIdentExpr() noexcept = default;

	llvm::Value * getAddress() const noexcept {
		return mIdent.getAddress();
	}

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	Identifier& mIdent;
};

// id [ Expr ]
class ASTArrayExpr : public ASTExpr {
public:
	ASTArrayExpr(Identifier& ident, std::shared_ptr<ASTExpr> expr) noexcept
	: mExpr {expr}
	, mIdent {ident}
	, mIndexLoc {nullptr} {
		switch (ident.getType()) {
			case Type::CharArray:
				mType = Type::Char;
				break;
			case Type::DoubleArray:
				mType = Type::Double;
				break;
			case Type::IntArray:
				mType = Type::Int;
				break;
			default:
				break;
		}
	}

	~ASTArrayExpr() noexcept = default;

	void setIndexLoc(llvm::Value * val) noexcept {
		mIndexLoc = val;
	}

	llvm::Value * getIndexLoc() const noexcept {
		return mIndexLoc;
	}

	llvm::Value * getAddress() const noexcept {
		return mIdent.getAddress();
	}

	std::shared_ptr<ASTExpr> getExpr() const noexcept {
		return mExpr;
	}

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	std::shared_ptr<ASTExpr> mExpr;
	Identifier& mIdent;
	llvm::Value * mIndexLoc;
};

// lhs (+=, -=, =) rhs
class ASTAssignOp : public ASTExpr {
public:
	ASTAssignOp(TokenType t) noexcept
	: mOp {t} { }

	~ASTAssignOp() noexcept = default;

	void setLHS(std::shared_ptr<ASTExpr> lhs) noexcept {
		mLHS = lhs;
	}

	void setRHS(std::shared_ptr<ASTExpr> rhs) noexcept {
		mRHS = rhs;
	}

	bool finalizeOp() noexcept;	
	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	TokenType mOp;
	std::shared_ptr<ASTExpr> mLHS;
	std::shared_ptr<ASTExpr> mRHS;
};

// id ( FuncCallArgs )
class ASTFuncExpr : public ASTExpr {
public:
	ASTFuncExpr(Identifier& ident) noexcept
	: mIdent {ident} {
		if (mIdent.getFunction()) {
			mType = mIdent.getFunction()->getReturnType();
		} else {
			mType = Type::Void;
		}
	}
	
	~ASTFuncExpr() noexcept = default;
	
	int getNumArgs() const noexcept {
		return mArgs.size();
	}

	void addArg(std::shared_ptr<ASTExpr> arg) noexcept;	
	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	Identifier& mIdent;
	std::vector<std::shared_ptr<ASTExpr>> mArgs;
};

class ASTLogicalAnd : public ASTExpr {
public:
	ASTLogicalAnd() noexcept = default;
	~ASTLogicalAnd() noexcept = default;

	void setLHS(std::shared_ptr<ASTExpr> lhs) noexcept {
		mLHS = lhs;
	}

	void setRHS(std::shared_ptr<ASTExpr> rhs) noexcept {
		mRHS = rhs;
	}

	bool finalizeOp() noexcept;	
	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	std::shared_ptr<ASTExpr> mLHS;
	std::shared_ptr<ASTExpr> mRHS;
};

class ASTLogicalOr : public ASTExpr {
public:
	ASTLogicalOr() noexcept = default;
	~ASTLogicalOr() noexcept = default;

	void setLHS(std::shared_ptr<ASTExpr> lhs) noexcept {
		mLHS = lhs;
	}

	void setRHS(std::shared_ptr<ASTExpr> rhs) noexcept {
		mRHS = rhs;
	}

	bool finalizeOp() noexcept;	
	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	std::shared_ptr<ASTExpr> mLHS;
	std::shared_ptr<ASTExpr> mRHS;
};

class ASTBinaryCmpOp : public ASTExpr {
public:
	ASTBinaryCmpOp(TokenType op) noexcept
	: mOp {op} { }

	~ASTBinaryCmpOp() noexcept = default; 

	void setLHS(std::shared_ptr<ASTExpr> lhs) noexcept {
		mLHS = lhs;
	}

	void setRHS(std::shared_ptr<ASTExpr> rhs) noexcept {
		mRHS = rhs;
	}

	bool finalizeOp() noexcept;	
	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	TokenType mOp;
	std::shared_ptr<ASTExpr> mLHS;
	std::shared_ptr<ASTExpr> mRHS;
};

class ASTBinaryMathOp : public ASTExpr {
public:
	ASTBinaryMathOp(TokenType op) noexcept
	: mOp {op} { }

	~ASTBinaryMathOp() noexcept = default; 
	
	void setLHS(std::shared_ptr<ASTExpr> lhs) noexcept {
		mLHS = lhs;
	}

	void setRHS(std::shared_ptr<ASTExpr> rhs) noexcept {
		mRHS = rhs;
	}

	bool finalizeOp() noexcept;	
	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	TokenType mOp;
	std::shared_ptr<ASTExpr> mLHS;
	std::shared_ptr<ASTExpr> mRHS;
};

// !expr
class ASTNotExpr : public ASTExpr {
public:
	ASTNotExpr(std::shared_ptr<ASTExpr> expr) noexcept
	: mExpr {expr} {
		mType = mExpr->getType();
	}

	~ASTNotExpr() noexcept = default; 

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	std::shared_ptr<ASTExpr> mExpr;
};

// ++id
class ASTIncExpr : public ASTExpr {
public:
	ASTIncExpr(Identifier& ident, std::shared_ptr<ASTExpr> expr) noexcept 
	: mIdent {ident}
	, mExpr {expr} {
		mType = ident.getType();
	}

	~ASTIncExpr() noexcept = default; 

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	Identifier& mIdent;
	std::shared_ptr<ASTExpr> mExpr;
};

// --id
class ASTDecExpr : public ASTExpr {
public:
	ASTDecExpr(Identifier& ident, std::shared_ptr<ASTExpr> expr) noexcept 
	: mIdent {ident}
	, mExpr {expr} {
		mType = ident.getType();
	}

	~ASTDecExpr() noexcept = default; 

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	Identifier& mIdent;	
	std::shared_ptr<ASTExpr> mExpr;
};

class ASTAddrOfArray : public ASTExpr {
public:
	ASTAddrOfArray(std::shared_ptr<ASTArrayExpr> array) noexcept
	: mArray {array} {
		mType = mArray->getType();
	}

	~ASTAddrOfArray() noexcept = default; 

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	std::shared_ptr<ASTArrayExpr> mArray;
};

class ASTStringExpr : public ASTExpr {
public:
	ASTStringExpr(std::string& str, StringTable& tbl) noexcept
	: mString {tbl.getString(str)} {
		mType = Type::CharArray;
	}

	~ASTStringExpr() noexcept = default; 

	int getLength() const noexcept {
		return mString->getText().size();
	}	

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	ConstStr * mString;
};

class ASTConstantExpr : public ASTExpr {
public:
	ASTConstantExpr(const std::string& constStr) noexcept
    : mValue {std::stoi(constStr)} {
		mType = Type::Int;
	}

	~ASTConstantExpr() noexcept = default;
	
	int getValue() const noexcept {
		return mValue;
	}

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	int mValue;
};

class ASTDoubleExpr : public ASTExpr {
public:
	ASTDoubleExpr(const std::string& constStr)
	: mValue {std::stod(constStr)} {
		mType = Type::Double;
	}

	~ASTDoubleExpr() noexcept = default;

	double getValue() const noexcept {
		return mValue;
	}

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	double mValue;
};

class ASTCharExpr : public ASTExpr {
public:
	ASTCharExpr(const std::string& constStr) 
	: mValue {constStr[0]}{
		mType = Type::Char;
	}

	~ASTCharExpr() noexcept = default;
	
	char getValue() const noexcept {
		return mValue;
	}

	void printNode(std::ostream& output, int depth = 0) const noexcept override;
	llvm::Value * codegen(CodeContext& context) noexcept override;
private:
	char mValue;
};

#endif