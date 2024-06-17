#include "astNodes.h"
#include "../error/parseExcept.h"

void ASTProg::addFunction(std::shared_ptr<ASTFunc> func) noexcept {
	mFuncs.push_back(func);
}

// add an argument to this function
void ASTFunc::addArg(std::shared_ptr<ASTArgDecl> arg) noexcept {
	mArgs.push_back(arg);
}

// returns true if the type passed in matches the argument
// declaration for that particular argument
bool ASTFunc::checkArgType(unsigned int argNum, Type type) const noexcept {
	if (argNum > 0 && argNum <= mArgs.size()) {
		return mArgs[argNum - 1]->getType() == type;
	} else {
		return false;
	}
}

Type ASTFunc::getArgType(unsigned int argNum) const noexcept {
	if (argNum > 0 && argNum <= mArgs.size()) {
		return mArgs[argNum - 1]->getType();
	} else {
		return Type::Void;
	}
}

// set the compound statement body
void ASTFunc::setBody(std::shared_ptr<ASTCompoundStmt> body) noexcept {
	mBody = body;
}

/*
-----------------------------------------------------------------------
expression ast nodes
*/

// Finalize the op.
// Call this after both lhs/rhs are set, and
// it will evaluate the type of the expression.
// Returns false if this is an invalid operation.
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

ASTConstantExpr::ASTConstantExpr(const std::string& constStr) {
	// ConstExpr is always evaluated as a 32-bit integer
	// it can later be converted to a char at assignment
	mType = Type::Int;
	
	// Is this a character in ''?
	if (constStr[0] == '\'') {
		if (constStr == "\'\\t\'") {
			mValue = '\t';
		} else if (constStr == "\'\\n\'") {
			mValue = '\n';
		} else {
			mValue = constStr[1];
		}
	} else {
		// NOTE: This WILL throw if the value is out of bounds
		std::istringstream ss(constStr);
		ss >> mValue;

		if (ss.fail()) {
			throw std::invalid_argument(constStr);
		}
	}
}

ASTStringExpr::ASTStringExpr(const std::string& str, StringTable& tbl) {
	// This function can only be called if this is a valid string
	std::string actStr = str.substr(1, str.length() - 2);
	mType = Type::CharArray;
	
	// Replace valid escape sequences
	size_t pos = actStr.find("\\n");
	while (pos != std::string::npos) {
		actStr.replace(pos, 2, "\n");
		pos = actStr.find("\\n");
	}
	
	pos = actStr.find("\\t");
	while (pos != std::string::npos)
	{
		actStr.replace(pos, 2, "\t");
		pos = actStr.find("\\t");
	}
	
	// Now grab this from the StringTable
	mString = tbl.getString(actStr);
}

void ASTFuncExpr::addArg(std::shared_ptr<ASTExpr> arg) noexcept {
	mArgs.push_back(arg);
}

/*
-----------------------------------------------------------------------
statement ast nodes
*/

void ASTCompoundStmt::addDecl(std::shared_ptr<ASTDecl> decl) noexcept {
	mDecls.push_back(decl);
}

void ASTCompoundStmt::addStmt(std::shared_ptr<ASTStmt> stmt) noexcept {
	mStmts.push_back(stmt);
}

std::shared_ptr<ASTStmt> ASTCompoundStmt::getLastStmt() noexcept {
	if (mStmts.size() > 0) {
		return mStmts.back();
	} else {
		return nullptr;
	}
}