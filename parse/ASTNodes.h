#ifdef ASTNODES_H
#define ASTNODES_H

class ASTNode {

};

class ASTProg : public ASTNode { 
// contains list of functions and global var declarations

public: 
};

class ASTFunc : public ASTNode { 
// type + name + (arg list (can forgo the variables)) + { compoundstmt } + ; 
// collection of statements

public:
};

class ASTForwardFuncDef : public ASTNode { 
// type + name + (arg list (can forgo the variables)) + ; 
// collection of statements

public:
};

class ASTArgDecl : public ASTNode {

public:
};

// helper node for arrays 
class ASTArrSub : public ASTNode {

public:
};

//----------------------------------------------------------------------

class ASTStmt : public ASTNode {

public:
};

class ASTCompoundStmt : public ASTStmt {
// collection of statements and declarations

public:
};

class ASTDeclStmt : public ASTStmt {
// int x; or int y = 5; or char x[] = "asdfsadf"; or int test[] = {1,2,3,4,5}
public:
};

class ASTDeclArrStmt : public ASTStmt {

public:
};

class ASTIfStmt : public ASTStmt {

public:
};

class ASTForStmt : public ASTStmt {

public:
};

class ASTWhileStmt : public ASTStmt {

public:
};

class ASTReturnStmt : public ASTStmt {

public:
};

class ASTExprStmt : public ASTStmt {

public:
};

class ASTNullStmt : public ASTStmt {

public:
};

//----------------------------------------------------------------------

// template for expressions
class ASTExpr : public ASTNode { 

public:
};

// "Bad" expr is returned if a () subexpr fails, so at least
// further ops will recognize it as a potentially valid op
class ASTBadExpr : public ASTExpr {

public:
};

class ASTIdentExpr : public ASTExpr {
// a
public:
};

class ASTArrExpr : public ASTExpr {
// id [ Expr ]
public:
};

class ASTAssignExpr : public ASTExpr {
// =, +=, -= ex: x = 5; or x[4] += 1; 
public:
};

class ASTFuncExpr : public ASTExpr {
// id ( FuncCallArgs )

public:
};

class ASTAddrOfArray : public ASTExpr {
// &arr
public:
};

class ASTUnaryArithOp : public ASTExpr { 
// ++, -- 
public:
};

class ASTUnaryLogicalOp : public ASTExpr {
// !
public:
};

class ASTBinaryLogicalOp : public ASTExpr { 
// &&, ||
public:
};

class ASTBinaryRelOp : public ASTExpr { 
// ==, !=, <, <=, >, >=
public:
};

class ASTBinaryArithOp : public ASTExpr { 
// +, -, *, /, %, 
public:
};

class ASTDoubleLit : public ASTExpr {
    double Val;

public:
    ASTDoubleLit(double Val) : Val(Val) {}
};

class ASTCharLit : public ASTExpr {
    char Val;

public:
    ASTCharLit(char Val) : Val(Val) {}
};

class ASTIntLit : public ASTExpr {
    int Val;

public:
    ASTIntLit(int Val) : Val(Val) {}
};

#endif