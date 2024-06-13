/*
defines all AST nodes used in the recursive descent parsing
*/

#ifdef ASTNODES_H
#define ASTNODES_H

#include <types.h>

class ASTNode {
protected:
	ASTNode() { }
    virtual ~ASTNode() { }
	ASTNode(const ASTNode& copy) { }
	ASTNode& operator=(const ASTNode& rhs) { return *this; }
public:
	//virtual void printNode(std::ostream& output, int depth = 0) const noexcept = 0;
	//virtual llvm::Value* emitIR(CodeContext& ctx) noexcept = 0;
};

class ASTProg : public ASTNode { 
    // contains list of functions and global var declarations
    std::vector<std::shared_ptr<ASTFunc>> funcList;
public: 
    void addFunction(std::shared_ptr<ASTFunc> func) noexcept;
};

class ASTFunc : public ASTNode { 
    // type + name + (arg list (can forgo the variables)) + { compoundstmt } + ; 
    std::vector<ASTArgDecl> args;
    std::shared_ptr<ASTCompoundStmt> compoundStmt;
    // Identifier& mIdent;
    // SymbolTable::ScopeTable& mScopeTable;
    Type returnType;
public:
	ASTFunction(Identifier& ident, Type returnType, SymbolTable::ScopeTable& scopeTable) noexcept
	: mIdent(ident)
	, mReturnType(returnType)
	, mScopeTable(scopeTable) { }

    // add arg to this function
    void addArg(std::shared_ptr<ASTArgDecl> arg) noexcept;
    // set the compound statement body
    void setBody(std::shared_ptr<ASTCompoundStmt> body) noexcept;

    bool checkArgType(unsigned int argNum, Type type) const noexcept;
	Type getArgType(unsigned int argNum) const noexcept;

    Type getReturnType() const noexcept {
        return mReturnType;
    }
	
    size_t getNumArgs() const noexcept {
        return mArgs.size();
    }
};

// class ASTForwardFuncDef : public ASTNode { 
//     // type + name + (arg list (can forgo the variables)) + ; 
//     // collection of statements
//     // std::vector<std::shared_ptr<ASTArgDecl>> args;
//     // Identifier& mIdent;
//     // Type mReturnType;
// public:
//     // void addType(std::shared_ptr<ASTArgDecl> arg);
// };

// class ASTArgDecl : public ASTNode {

// public:
// };

// // helper node for arrays 
// class ASTArrSub : public ASTNode {

// public:
// };

// //----------------------------------------------------------------------

// class ASTStmt : public ASTNode {

// public:
// };

// class ASTCompoundStmt : public ASTStmt {
// // collection of statements and declarations

// public:
// };

// class ASTDeclStmt : public ASTStmt {
// // int x; or int y = 5; or char x[] = "asdfsadf"; or int test[] = {1,2,3,4,5}
// public:
// };

// class ASTDeclArrStmt : public ASTStmt {

// public:
// };

// class ASTIfStmt : public ASTStmt {

// public:
// };

// class ASTForStmt : public ASTStmt {

// public:
// };

// class ASTWhileStmt : public ASTStmt {

// public:
// };

// class ASTReturnStmt : public ASTStmt {

// public:
// };

// class ASTExprStmt : public ASTStmt {

// public:
// };

// class ASTNullStmt : public ASTStmt {

// public:
// };

// //----------------------------------------------------------------------

// // template for expressions
// class ASTExpr : public ASTNode { 

// public:
// };

// // "Bad" expr is returned if a () subexpr fails, so at least
// // further ops will recognize it as a potentially valid op
// class ASTBadExpr : public ASTExpr {

// public:
// };

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