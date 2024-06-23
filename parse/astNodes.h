/*
defines all AST nodes used in the recursive descent parsing
*/

#ifndef ASTNODES_H
#define ASTNODES_H

#include <memory>
#include <vector>
#include "symbols.h"
#include "../scan/token.h"

class ASTFunc;
class ASTArgDecl;
class ASTCompoundStmt;
class ASTExpr;

class ASTNode {
public:
    virtual ~ASTNode() = default;

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept = 0;
protected:
	ASTNode() = default;
};

class ASTProg : public ASTNode { 
public:
    void addFunction(std::shared_ptr<ASTFunc> func) noexcept;
	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private: 
    std::vector<std::shared_ptr<ASTFunc>> mFuncs;
};

class ASTFunc : public ASTNode {
public:
    ASTFunc(Identifier& ident, Type returnType, ScopeTable& scopeTable) noexcept
    : mIdent(ident)
    , mReturnType(returnType)
    , mScopeTable(scopeTable) {}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;

    void addArg(std::shared_ptr<ASTArgDecl> arg) noexcept;
    void setBody(std::shared_ptr<ASTCompoundStmt> body) noexcept;
    bool checkArgType(int argNum, Type type) const noexcept;
    Type getArgType(int argNum) const noexcept;

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
    ScopeTable mScopeTable;
};

class ASTArgDecl : public ASTNode {
public:
	ASTArgDecl(Identifier& ident) noexcept
	: mIdent(ident) { }
	
	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;

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

class ASTStmt : public ASTNode {
	
};

class ASTDecl : public ASTStmt {
public:
	ASTDecl(Identifier& ident, std::shared_ptr<ASTExpr> expr = nullptr) noexcept
	: mIdent(ident)
	, mExpr(expr) { }

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	Identifier& mIdent;
	std::shared_ptr<ASTExpr> mExpr;
};

class ASTCompoundStmt : public ASTStmt {
public:
	void addDecl(std::shared_ptr<ASTDecl> decl) noexcept;
	void addStmt(std::shared_ptr<ASTStmt> stmt) noexcept;

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	std::vector<std::shared_ptr<ASTDecl>> mDecls;
	std::vector<std::shared_ptr<ASTStmt>> mStmts;
};

class ASTIfStmt : public ASTStmt {
public:
	ASTIfStmt(std::shared_ptr<ASTExpr> expr, std::shared_ptr<ASTStmt> thenStmt, std::shared_ptr<ASTStmt> elseStmt = nullptr) noexcept
	: mExpr(expr)
	, mThenStmt(thenStmt)
	, mElseStmt(elseStmt) { }

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	std::shared_ptr<ASTExpr> mExpr;
	std::shared_ptr<ASTStmt> mThenStmt;
	std::shared_ptr<ASTStmt> mElseStmt;
};

class ASTReturnStmt : public ASTStmt {
public:
	ASTReturnStmt(std::shared_ptr<ASTExpr> expr) noexcept
	: mExpr(expr) { }

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	std::shared_ptr<ASTExpr> mExpr;
};

class ASTForStmt : public ASTStmt {
public:
	ASTForStmt(std::shared_ptr<ASTDecl> varDef, std::shared_ptr<ASTExpr> cond, std::shared_ptr<ASTExpr> update, std::shared_ptr<ASTStmt> loopStmt) noexcept
    : mVarDecl(varDef)
    , mExprCond(cond)
    , mUpdateExpr(update)
	, mLoopStmt(loopStmt) { }
private:
    std::shared_ptr<ASTDecl> mVarDecl;
	std::shared_ptr<ASTExpr> mExprCond;
    std::shared_ptr<ASTExpr> mUpdateExpr;
	std::shared_ptr<ASTStmt> mLoopStmt;
};

class ASTWhileStmt : public ASTStmt {
public:
	ASTWhileStmt(std::shared_ptr<ASTExpr> expr, std::shared_ptr<ASTStmt> loopStmt) noexcept
	: mExpr(expr)
	, mLoopStmt(loopStmt) { }

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	std::shared_ptr<ASTExpr> mExpr;
	std::shared_ptr<ASTStmt> mLoopStmt;
};

class ASTExprStmt : public ASTStmt {
public:
	ASTExprStmt(std::shared_ptr<ASTExpr> expr) noexcept
	: mExpr(expr) { }

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	std::shared_ptr<ASTExpr> mExpr;
};

class ASTNullStmt : public ASTStmt {
public:
	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
};

/*
------------------------------------------------------
expressions
*/ 

class ASTExpr : public ASTNode {
public:
	ASTExpr() noexcept
	: mType(Type::Void) { }
	
	Type getType() const noexcept {
		return mType;
	}
protected:
	Type mType;
};

class ASTIdentExpr : public ASTExpr {
public:
	ASTIdentExpr(Identifier& ident) noexcept
	: mIdent(ident) {
		mType = mIdent.getType();
	}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	Identifier& mIdent;
};

// id [ Expr ]
class ASTArrayExpr : public ASTExpr {
public:
	ASTArrayExpr(Identifier& ident, std::shared_ptr<ASTExpr> expr) noexcept
	: mExpr {expr}
	, mIdent {ident} {
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

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	std::shared_ptr<ASTExpr> mExpr;
	Identifier& mIdent;
};

class ASTAssignOp : public ASTExpr {
public:
	ASTAssignOp(TokenType t) noexcept
	: mOp {t} { }

	// we need to be able to manually set the lhs/rhs
	void setLHS(std::shared_ptr<ASTExpr> lhs) noexcept {
		mLHS = lhs;
	}

	void setRHS(std::shared_ptr<ASTExpr> rhs) noexcept {
		mRHS = rhs;
	}

	// finalize the op
	// call this after both lhs/rhs are set and
	// it will evaluate the type of the expression
	// returns false if this is an invalid operation
	bool finalizeOp() noexcept;	

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	TokenType mOp;
	std::shared_ptr<ASTExpr> mLHS;
	std::shared_ptr<ASTExpr> mRHS;
};

// id ( FuncCallArgs )
class ASTFuncExpr : public ASTExpr {
public:
	ASTFuncExpr(Identifier& ident) noexcept
	: mIdent(ident) {
		if (mIdent.getFunction()) {
			mType = mIdent.getFunction()->getReturnType();
		} else {
			mType = Type::Void;
		}
	}
	
	int getNumArgs() const noexcept {
		return mArgs.size();
	}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;

	void addArg(std::shared_ptr<ASTExpr> arg) noexcept;	
private:
	Identifier& mIdent;
	std::vector<std::shared_ptr<ASTExpr>> mArgs;
};

class ASTLogicalAnd : public ASTExpr {
public:
	// we need to be able to manually set the lhs/rhs
	void setLHS(std::shared_ptr<ASTExpr> lhs) noexcept {
		mLHS = lhs;
	}

	void setRHS(std::shared_ptr<ASTExpr> rhs) noexcept {
		mRHS = rhs;
	}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
	
	// finalize the op
	// call this after both lhs/rhs are set and
	// it will evaluate the type of the expression
	// returns false if this is an invalid operation
	bool finalizeOp() noexcept;	
private:
	std::shared_ptr<ASTExpr> mLHS;
	std::shared_ptr<ASTExpr> mRHS;
};

class ASTLogicalOr : public ASTExpr {
public:
	// we need to be able to manually set the lhs/rhs
	void setLHS(std::shared_ptr<ASTExpr> lhs) noexcept {
		mLHS = lhs;
	}

	void setRHS(std::shared_ptr<ASTExpr> rhs) noexcept {
		mRHS = rhs;
	}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
	
	// finalize the op
	// call this after both lhs/rhs are set and
	// it will evaluate the type of the expression
	// returns false if this is an invalid operation
	bool finalizeOp() noexcept;	
private:
	std::shared_ptr<ASTExpr> mLHS;
	std::shared_ptr<ASTExpr> mRHS;
};

class ASTBinaryCmpOp : public ASTExpr {
public:
	ASTBinaryCmpOp(TokenType op) noexcept
	: mOp(op) { }

	// we need to be able to manually set the lhs/rhs
	void setLHS(std::shared_ptr<ASTExpr> lhs) noexcept {
		mLHS = lhs;
	}

	void setRHS(std::shared_ptr<ASTExpr> rhs) noexcept {
		mRHS = rhs;
	}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
	
	// finalize the op
	// call this after both lhs/rhs are set and
	// it will evaluate the type of the expression
	// returns false if this is an invalid operation
	bool finalizeOp() noexcept;	
private:
	TokenType mOp;
	std::shared_ptr<ASTExpr> mLHS;
	std::shared_ptr<ASTExpr> mRHS;
};

class ASTBinaryMathOp : public ASTExpr {
public:
	ASTBinaryMathOp(TokenType op) noexcept
	: mOp(op) { }
	
	// we need to be able to manually set the lhs/rhs
	void setLHS(std::shared_ptr<ASTExpr> lhs) noexcept {
		mLHS = lhs;
	}

	void setRHS(std::shared_ptr<ASTExpr> rhs) noexcept {
		mRHS = rhs;
	}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
	
	// finalize the op
	// call this after both lhs/rhs are set and
	// it will evaluate the type of the expression
	// returns false if this is an invalid operation
	bool finalizeOp() noexcept;	
private:
	TokenType mOp;
	std::shared_ptr<ASTExpr> mLHS;
	std::shared_ptr<ASTExpr> mRHS;
};

class ASTNotExpr : public ASTExpr {
public:
	ASTNotExpr(std::shared_ptr<ASTExpr> expr) noexcept
	: mExpr(expr) {
		mType = mExpr->getType();
	}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	std::shared_ptr<ASTExpr> mExpr;
};

// ++id
class ASTIncExpr : public ASTExpr {
public:
	ASTIncExpr(Identifier& ident) noexcept
	: mIdent(ident) {
		mType = mIdent.getType();
	}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	Identifier& mIdent;
};

// --id
class ASTDecExpr : public ASTExpr {
public:
	ASTDecExpr(Identifier& ident) noexcept
	: mIdent(ident) {
		mType = mIdent.getType();
	}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	Identifier& mIdent;
};

class ASTAddrOfArray : public ASTExpr {
public:
	ASTAddrOfArray(std::shared_ptr<ASTArrayExpr> array) noexcept
	: mArray(array) {
		mType = mArray->getType();
	}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	std::shared_ptr<ASTArrayExpr> mArray;
};

class ASTStringExpr : public ASTExpr {
public:
	ASTStringExpr(std::string& str, StringTable& tbl) noexcept
	: mString {tbl.getString(str)} {
		mType = Type::CharArray;
	}

	int getLength() const noexcept {
		return mString->getText().size();
	}	

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	ConstStr * mString;
};

class ASTConstantExpr : public ASTExpr {
public:
	ASTConstantExpr(const std::string& constStr) noexcept
    : mValue {std::stoi(constStr)} {
		mType = Type::Int;
	}
	
	int getValue() const noexcept {
		return mValue;
	}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	int mValue;
};

class ASTDoubleExpr : public ASTExpr {
public:
	ASTDoubleExpr(const std::string& constStr)
	: mValue {std::stod(constStr)} {
		mType = Type::Double;
	}
	
	double getValue() const noexcept {
		return mValue;
	}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	double mValue;
};

class ASTCharExpr : public ASTExpr {
public:
	ASTCharExpr(const std::string& constStr) 
	: mValue {constStr[0]}{
		mType = Type::Char;
	}
	
	char getValue() const noexcept {
		return mValue;
	}

	virtual void printNode(std::ostream& output, int depth = 0) const noexcept override;
private:
	char mValue;
};

#endif