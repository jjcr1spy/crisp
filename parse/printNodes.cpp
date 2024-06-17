#include <ostream>
#include "astNodes.h"

// ASTProgram
void ASTProg::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }

    output << "Program:" << std::endl;
    for (auto func : mFuncs) {
        func->printNode(output, depth + 1);
    }
}

// ASTFunction
void ASTFunc::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "Function: ";
    switch (mReturnType) {
        case Type::Void:
            output << "void ";
            break;
        case Type::Int:
            output << "int ";
            break;
        case Type::Char:
            output << "char ";
            break;
        default:
            output << "Shouldn't have gotten here. ";
            break;
    }
    output << mIdent.getName() << std::endl;

    for (auto arg : mArgs) {
        arg->printNode(output, depth + 1);
    }
    
    mBody->printNode(output, depth + 1);
}

// ASTArgDecl
void ASTArgDecl::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "ArgDecl: ";
    switch (mIdent.getType()) {
        case Type::Void:
            output << "void ";
            break;
        case Type::Int:
            output << "int ";
            break;
        case Type::Char:
            output << "char ";
            break;
        case Type::IntArray:
            output << "int[] ";
            break;
        case Type::CharArray:
            output << "char[] ";
            break;
        default:
            output << "Shouldn't have gotten here...";
            break;
    }
    output << mIdent.getName() << std::endl;
}

// ASTArraySub
void ASTArraySub::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "ArraySub: " << mIdent.getName() << std::endl;
    mExpr->printNode(output, depth + 1);
}

// ASTBadExpr
void ASTBadExpr::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "BadExpr:" << std::endl;
}

// ASTLogicalAnd
void ASTLogicalAnd::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "LogicalAnd: " << std::endl;
    mLHS->printNode(output, depth + 1);
    mRHS->printNode(output, depth + 1);
}

// ASTLogicalOr
void ASTLogicalOr::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "LogicalOr: " << std::endl;
    mLHS->printNode(output, depth + 1);
    mRHS->printNode(output, depth + 1);
}

// ASTBinaryCmpOp
void ASTBinaryCmpOp::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "BinaryCmp " << Token::mToString[mOp] << ':' << std::endl;
    mLHS->printNode(output, depth + 1);
    mRHS->printNode(output, depth + 1);
}

// ASTBinaryMathOp
void ASTBinaryMathOp::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "BinaryMath " << Token::mToString[mOp] << ':' << std::endl;
    mLHS->printNode(output, depth + 1);
    mRHS->printNode(output, depth + 1);
}

// ASTNotExpr
void ASTNotExpr::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "NotExpr:" << std::endl;
    mExpr->printNode(output, depth + 1);
}

// ASTConstantExpr
void ASTConstantExpr::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "ConstantExpr: " << mValue << std::endl;
}

// ASTStringExpr
void ASTStringExpr::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "StringExpr: " << mString->getText() << std::endl;
}

// ASTIdentExpr
void ASTIdentExpr::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "IdentExpr: " << mIdent.getName() << std::endl;
}

// ASTArrayExpr
void ASTArrayExpr::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "ArrayExpr: " << std::endl;
    mArray->printNode(output, depth + 1);
}

// ASTFuncExpr
void ASTFuncExpr::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "FuncExpr: " << mIdent.getName() << std::endl;
    for (auto arg : mArgs) {
        arg->printNode(output, depth + 1);
    }
}

// ASTIncExpr
void ASTIncExpr::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "IncExpr: " << mIdent.getName() << std::endl;
}

// ASTDecExpr
void ASTDecExpr::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "DecExpr: " << mIdent.getName() << std::endl;
}

// ASTAddrOfArray
void ASTAddrOfArray::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "AddrOfArray:" << std::endl;
    mArray->printNode(output, depth + 1);
}

// ASTDecl
void ASTDecl::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "Decl: ";
    switch (mIdent.getType()) {
        case Type::Void:
            output << "void";
            break;
        case Type::Int:
            output << "int";
            break;
        case Type::Char:
            output << "char";
            break;
        case Type::IntArray:
            output << "int[" << mIdent.getArrayCount() << ']';
            break;
        case Type::CharArray:
            output << "char[" << mIdent.getArrayCount() << ']';
            break;
        default:
            output << "Shouldn't have gotten here...";
            break;
    }
    output << ' ' << mIdent.getName() << std::endl;
    if (mExpr) {
        mExpr->printNode(output, depth + 1);
    }
}

// ASTCompoundStmt
void ASTCompoundStmt::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "CompoundStmt:" << std::endl;
    for (auto decl : mDecls) {
        decl->printNode(output, depth + 1);
    }
    for (auto stmt : mStmts) {
        stmt->printNode(output, depth + 1);
    }
}

// ASTReturnStmt
void ASTReturnStmt::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    if (!mExpr) {
        output << "ReturnStmt: (empty)" << std::endl;
    } else {
        output << "ReturnStmt:" << std::endl;
        mExpr->printNode(output, depth + 1);
    }
}

// ASTAssignStmt
void ASTAssignStmt::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "AssignStmt: " << mIdent.getName() << std::endl;
    mExpr->printNode(output, depth + 1);
}

// ASTAssignArrayStmt
void ASTAssignArrayStmt::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "AssignArrayStmt:" << std::endl;
    mArray->printNode(output, depth + 1);
    mExpr->printNode(output, depth + 1);
}

// ASTIfStmt
void ASTIfStmt::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "IfStmt: " << std::endl;
    mExpr->printNode(output, depth + 1);
    mThenStmt->printNode(output, depth + 1);
    if (mElseStmt) {
        mElseStmt->printNode(output, depth + 1);
    }
}

// ASTWhileStmt
void ASTWhileStmt::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "WhileStmt" << std::endl;
    mExpr->printNode(output, depth + 1);
    mLoopStmt->printNode(output, depth + 1);
}

// ASTExprStmt
void ASTExprStmt::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "ExprStmt" << std::endl;
    mExpr->printNode(output, depth + 1);
}

// ASTNullStmt
void ASTNullStmt::printNode(std::ostream& output, int depth) const noexcept {
    for (int i = 0; i < depth; i++) {
        output << "---";
    }
    output << "NullStmt" << std::endl;
}
