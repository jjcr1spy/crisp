/*
defines all AST nodes used in the recursive descent parsing
*/

#ifndef ASTNODES_H
#define ASTNODES_H

#include <memory>
#include <vector>

// for semantic analysis i.e classes SymbolTable ScopeTable Identifier
#include "symbols.h"

class ASTFunc;
class ASTArgDecl;
class ASTCompoundStmt;
class ASTExpr;

class ASTNode {
public:
    virtual ~ASTNode() = default;
protected:
	ASTNode() = default;
private:
};

class ASTProg : public ASTNode { 
public:
    void addFunction(std::shared_ptr<ASTFunc> func) noexcept;
private: 
    std::vector<std::shared_ptr<ASTFunc>> mFuncs;
};

class ASTFunc : public ASTNode {
public:
    ASTFunc(Identifier& ident, Type returnType, ScopeTable& scopeTable) noexcept
    : mIdent(ident)
    , mReturnType(returnType)
    , mScopeTable(scopeTable) {}

    void addArg(std::shared_ptr<ASTArgDecl> arg) noexcept;
    void setBody(std::shared_ptr<ASTCompoundStmt> body) noexcept;
    bool checkArgType(unsigned int argNum, Type type) const noexcept;
    Type getArgType(unsigned int argNum) const noexcept;
	
    Type getReturnType() const noexcept {
        return mReturnType;
    }

    size_t getNumArgs() const noexcept {
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
	
	Type getType() const noexcept {
		return mIdent.getType();
	}
	
	Identifier& getIdent() noexcept {
		return mIdent;
	}	
private:
	Identifier& mIdent;
};

// array subscript helper node
class ASTArraySub : public ASTNode {
public:
	ASTArraySub(Identifier& ident, std::shared_ptr<ASTExpr> expr) noexcept
	: mIdent(ident)
	, mExpr(expr) { }
	
	Type getType() const noexcept {
		return mIdent.getType();
	}	
private:
	Identifier& mIdent;
	std::shared_ptr<ASTExpr> mExpr;
};

// ------------------------------------------------------
// below are statements in crisp

// Statement AST Nodes
class ASTStmt : public ASTNode {
	
};

class ASTDecl : public ASTStmt {
public:
	ASTDecl(Identifier& ident, std::shared_ptr<ASTExpr> expr = nullptr) noexcept
	: mIdent(ident)
	, mExpr(expr) { }
private:
	Identifier& mIdent;
	std::shared_ptr<ASTExpr> mExpr;
};

class ASTCompoundStmt : public ASTStmt {
public:
	void addDecl(std::shared_ptr<ASTDecl> decl) noexcept;
	void addStmt(std::shared_ptr<ASTStmt> stmt) noexcept;
	std::shared_ptr<ASTStmt> getLastStmt() noexcept;
private:
	std::vector<std::shared_ptr<ASTDecl>> mDecls;
	std::vector<std::shared_ptr<ASTStmt>> mStmts;
};

class ASTAssignStmt : public ASTStmt {
public:
	ASTAssignStmt(Identifier& ident, std::shared_ptr<ASTExpr> expr) noexcept
	: mIdent(ident)
	, mExpr(expr) { }
private:
	Identifier& mIdent;
	std::shared_ptr<ASTExpr> mExpr;
};

class ASTAssignArrayStmt : public ASTStmt {
public:
	ASTAssignArrayStmt(std::shared_ptr<ASTArraySub> array, std::shared_ptr<ASTExpr> expr) noexcept
	: mArray(array)
	, mExpr(expr) { }
private:
	std::shared_ptr<ASTArraySub> mArray;
	std::shared_ptr<ASTExpr> mExpr;
};

class ASTIfStmt : public ASTStmt {
public:
	ASTIfStmt(std::shared_ptr<ASTExpr> expr, std::shared_ptr<ASTStmt> thenStmt, std::shared_ptr<ASTStmt> elseStmt = nullptr) noexcept
	: mExpr(expr)
	, mThenStmt(thenStmt)
	, mElseStmt(elseStmt) { }
private:
	std::shared_ptr<ASTExpr> mExpr;
	std::shared_ptr<ASTStmt> mThenStmt;
	std::shared_ptr<ASTStmt> mElseStmt;
};

class ASTReturnStmt : public ASTStmt {
public:
	ASTReturnStmt(std::shared_ptr<ASTExpr> expr) noexcept
	: mExpr(expr) { }
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
private:
	std::shared_ptr<ASTExpr> mExpr;
	std::shared_ptr<ASTStmt> mLoopStmt;
};

class ASTExprStmt : public ASTStmt {
public:
	ASTExprStmt(std::shared_ptr<ASTExpr> expr) noexcept
	: mExpr(expr) { }
private:
	std::shared_ptr<ASTExpr> mExpr;
};

class ASTNullStmt : public ASTStmt {

};

// ------------------------------------------------------
// below are expressions 

// expression AST Nodes
class ASTExpr : public ASTNode {
public:
	ASTExpr() noexcept
	: mType(Type::Void) { }
	
	Type getType() const noexcept {
		return mType;
	}
protected:
	// all expressions have a type (used for semantic evaluation)
	Type mType;
};

// bad expr is returned if a () subexpr fails so at least
// further ops will recognize it as a potentially valid op
class ASTBadExpr : public ASTExpr {
public:

};

class ASTConstantExpr : public ASTExpr {
public:
	ASTConstantExpr(const std::string& constStr);

	int getValue() const noexcept {
		return mValue;
	}
	
	void changeToInt() noexcept {
		mType = Type::Int;
	}
	
	void changeToChar() noexcept {
		mType = Type::Char;
	}	
private:
	int mValue;
};

class ASTStringExpr : public ASTExpr {
public:
	ASTStringExpr(const std::string& str);

	size_t getLength() const noexcept {
		return mString->getText().size();
	}	
private:
	ConstStr* mString;
};

// class ASTIdentExpr : public ASTExpr {
// // a
// public:
// };

// class ASTArrExpr : public ASTExpr {
// // id [ Expr ]
// public:
// };

// class ASTAssignExpr : public ASTExpr {
// // =, +=, -= ex: x = 5; or x[4] += 1; 
// public:
// };

// class ASTFuncExpr : public ASTExpr {
// // id ( FuncCallArgs )

// public:
// };

// class ASTAddrOfArray : public ASTExpr {
// // &arr
// public:
// };

// class ASTUnaryArithOp : public ASTExpr { 
// // ++, -- 
// public:
// };

// class ASTUnaryLogicalOp : public ASTExpr {
// // !
// public:
// };

// class ASTBinaryLogicalOp : public ASTExpr { 
// // &&, ||
// public:
// };

// class ASTBinaryRelOp : public ASTExpr { 
// // ==, !=, <, <=, >, >=
// public:
// };

// class ASTBinaryArithOp : public ASTExpr { 
// // +, -, *, /, %, 
// public:
// };

// class ASTDoubleLit : public ASTExpr {
//     double Val;

// public:
//     ASTDoubleLit(double Val) : Val(Val) {}
// };

// class ASTCharLit : public ASTExpr {
//     char Val;

// public:
//     ASTCharLit(char Val) : Val(Val) {}
// };

// class ASTIntLit : public ASTExpr {
//     int Val;

// public:
//     ASTIntLit(int Val) : Val(Val) {}
// };

#endif