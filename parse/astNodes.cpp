#include "astNodes.h"

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