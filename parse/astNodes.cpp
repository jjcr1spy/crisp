#include "astNodes.h"

// add a function to the program
void ASTProg::addFunction(std::shared_ptr<ASTFunc> func) noexcept {
	mFuncs.push_back(func);
}

// add an argument to said function
void ASTFunc::addArg(std::shared_ptr<ASTArgDecl> arg) noexcept {
	mArgs.push_back(arg);
}

// returns true if the type passed in matches the argument
// declaration for that particular argument
bool ASTFunc::checkArgType(int argNum, Type type) const noexcept {
	if (argNum > 0 && argNum <= static_cast<int>(mArgs.size())) return mArgs[argNum - 1]->getType() == type;
	else return false;
}

Type ASTFunc::getArgType(int argNum) const noexcept {
	if (argNum > 0 && argNum <= static_cast<int>(mArgs.size())) return mArgs[argNum - 1]->getType();
	else return Type::Void;
}

void ASTFunc::setBody(std::shared_ptr<ASTCompoundStmt> body) noexcept {
	mBody = body;
}

void ASTFuncExpr::addArg(std::shared_ptr<ASTExpr> arg) noexcept {
	mArgs.push_back(arg);
}

void ASTCompoundStmt::addDecl(std::shared_ptr<ASTDecl> decl) noexcept {
	mDecls.push_back(decl);
}

void ASTCompoundStmt::addStmt(std::shared_ptr<ASTStmt> stmt) noexcept {
	mStmts.push_back(stmt);
}

/* 
finalize each op
call this after both lhs/rhs are set and
it will evaluate the type of the expression
returns false if this is an invalid operation
*/

bool ASTAssignOp::finalizeOp() noexcept {
	return mLHS->getType() == mRHS->getType();
}

bool ASTLogicalAnd::finalizeOp() noexcept {
	mType = Type::Int;

	if (mRHS->getType() == Type::Int && mLHS->getType() == Type::Int) return true;
	else return false;
}

bool ASTLogicalOr::finalizeOp() noexcept {
	mType = Type::Int;

	if (mRHS->getType() == Type::Int && mLHS->getType() == Type::Int) return true;
	else return false;
}

bool ASTBinaryCmpOp::finalizeOp() noexcept {
	mType = Type::Int;

	if (mRHS->getType() == Type::Int && mLHS->getType() == Type::Int) return true;
	else return false;
}

bool ASTBinaryMathOp::finalizeOp() noexcept {
	mType = Type::Int;
	
	if (mRHS->getType() == Type::Int && mLHS->getType() == Type::Int) return true;
	else return false;
}