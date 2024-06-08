#ifdef ASTNODES_H
#define ASTNODES_H

class ASTNode {

};

class ASTProgram : public ASTNode { 

    
};

class ASTFunction : public ASTNode { 
    // collection of statements
    // can be also

public:

private:
	std::shared_ptr<ASTCompoundStmt> mBody;
	std::vector<std::shared_ptr<ASTArgDecl>> mArgs;
	Identifier& mIdent;
	SymbolTable::ScopeTable& mScopeTable;
	Type mReturnType;
};

class ASTExpr : public ASTNode {
    // collection of many operators 
}

class ASTStmt : public ASTNode {
    // different types of statements
    /*
    CompoundStmt
    | AssignStmt
    | IfStmt
    | WhileStmt
    | ReturnStmt
    | Expr ;
    | ;
    */ 
}

class ASTIfStmt : public ASTStmt {

};

#endif