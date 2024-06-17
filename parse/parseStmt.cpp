#include "parse.h"

std::shared_ptr<ASTDecl> Parser::parseDecl() {
	std::shared_ptr<ASTDecl> retVal;
	
    // a decl must start with int, char, or double
	if (peekIsOneOf({TokenType::KeyChar, TokenType::KeyDouble, TokenType::KeyInt})) {
		Type declType = Type::Void;

        switch (mCurrToken.mType) {
			case TokenType::KeyInt:
				declType = Type::Int;
				break;
			case TokenType::KeyDouble:
				declType = Type::Double;
				break;
			case TokenType::KeyChar:
				declType = Type::Char;
				break;
			default:
				break;
		}
		
		consumeToken();
		
		// set this to @@variable for now we will later change it assuming we parse the identifier properly
		Identifier * ident = mSymbolTable.getIdentifier("@@variable");
		
		// now we MUST get an identifier so go into a try
		try {
			if (mCurrToken.mType != TokenType::Identifier) {
				throw ParseExceptMsg("Type must be followed by identifier");
			}
			
			const std::string& tokenStr = mCurrToken.mStr;

			if (mSymbolTable.isDeclaredInScope(tokenStr)) {
                reportSemantError(std::string("Invalid redeclaration of identifier '") + std::string(tokenStr) + "'");
            }

            // if redeclaration nothing will happen
			ident = mSymbolTable.createIdentifier(tokenStr); 
			
			consumeToken();
			
			// is this an array declaration?
			if (peekAndConsume(TokenType::LBracket)) {
				std::shared_ptr<ASTConstantExpr> constExpr;

                switch (declType) {
                    case Type::Int:
                    	declType = Type::IntArray;

                        // int arrays must have a constant size defined because crisp does not support initializer lists
                        constExpr = parseConstantFactor();

                        if (!constExpr) {
                            reportSemantError("Int arrays must have a defined constant size");
                        }
                        
                        break;
                    case Type::Char:
                        declType = Type::CharArray;
					
                        // for character we support both constant size or implict size if it is assigned to a constant string
                        constExpr = parseConstantFactor();

                        break;
                    case Type::Double:
                    	declType = Type::DoubleArray;

                        // double arrays must have a constant size defined because crisp does not support initializer lists
                        constExpr = parseConstantFactor();

                        if (!constExpr) {
                            reportSemantError("Double arrays must have a defined constant size");
                        }
                                                
                        break;
                    default:
                        break;
                }

                if (constExpr) {
                    int count = constExpr->getValue();

                    if (count <= 0 || count > 65536) {
                        reportSemantError("Arrays must have a min of 1 and a max of 65536 elements");
                    }

                    ident->setArrayCount(count);
                } else {
                    // we will determine this later in the parse
                    ident->setArrayCount(0);
                }
				
				matchToken(TokenType::RBracket);
			}
			
			ident->setType(declType);
			
			std::shared_ptr<ASTExpr> assignExpr;
			
			// optionally this decl may have an assignment
			int col = mCurrToken.mCol;
				
			if (peekAndConsume(TokenType::Assign)) {
				// we do not allow assignment for int arrays
				if (declType == Type::IntArray) {
					reportSemantError("crisp does not allow assignment of int array declarations");
				} else if (declType == Type::DoubleArray) {
                    reportSemantError("crisp does not allow assignment of double array declarations");
                }
				
				assignExpr = parseExpr();
				if (!assignExpr) {
					throw ParseExceptMsg("Invalid expression after = in declaration");
				}

				// if lhs and rhs don not match
				if (ident->getType() != assignExpr->getType()) {
					std::string err("Cannot assign an expression of type ");
                    err += getTypeText(assignExpr->getType());
					err += " to ";
					err += getTypeText(ident->getType());

					reportSemantError(err, col);
				}

				// if this is a character array we need to do extra checks
				if (ident->getType() == Type::CharArray) {
					std::shared_ptr<ASTStringExpr> rhs = std::dynamic_pointer_cast<ASTStringExpr>(assignExpr);
					
                    if (rhs) {
						// if we have a declared size we need to make sure
						// there is enough room to fit the requested string
						// otherwise we need to set our size
						if (ident->getArrayCount() == 0) {
                            ident->setArrayCount(rhs->getLength() + 1);
                        } else if (ident->getArrayCount() < (rhs->getLength() + 1)) {
                            reportSemantError("Declared array cannot fit string");
                        }
					}
				}
			} else if (ident->getType() == Type::CharArray && ident->getArrayCount() == 0) {
				reportSemantError("char array must have declared size if there is no assignment");
			}
			
			matchToken(TokenType::SemiColon);
			
			retVal = std::make_shared<ASTDecl>(*ident, assignExpr);
		} catch (ParseExcept& e) {
			reportError(e);
			
			// skip all the tokens until the next semi-colon
			consumeUntil(TokenType::SemiColon);
			
			if (mCurrToken.mType == TokenType::EndOfFile) {
				throw EOFExcept();
			}
			
			// grab the semi-colon also
			consumeToken();
			
			// put in a decl here with the bogus identifier
			// "@@error" this is so the parse will continue to the
			// next decl if there is one
			retVal = std::make_shared<ASTDecl>(*ident);
		}
	}
	
	return retVal;
}

std::shared_ptr<ASTStmt> Parser::parseStmt() {
	std::shared_ptr<ASTStmt> retVal;
	
    try {
		// parseAssignStmt must go before parseExprStmt read comments in parseAssignStmt for why
		if ((retVal = parseCompoundStmt()));
        else if ((retVal = parseForStmt()));
		else if ((retVal = parseAssignStmt()));
		else if ((retVal = parseReturnStmt()));
		else if ((retVal = parseWhileStmt()));
		else if ((retVal = parseExprStmt()));
		else if ((retVal = parseNullStmt()));
		else if ((retVal = parseIfStmt()));
		else if ((retVal = parseDecl()));
        else {
            // NEED TO PARSE EXPR THEN as its in the form expr;
        }
	} catch (ParseExcept& e) {
		reportError(e);
		
		// skip all the tokens until the next semi-colon
		consumeUntil(TokenType::SemiColon);
		
		if (mCurrToken.mType == TokenType::EndOfFile) {
			throw EOFExcept();
		}
		
		// grab the semi-colon also
		consumeToken();
		
		// put in a null statement here so we can try to continue
		retVal = std::make_shared<ASTNullStmt>();
	}
	
	return retVal;
}


// if the compound statement is a function body then the symbol table scope
// change will happen at a higher level so it should not happen in parseCompoundStmt
std::shared_ptr<ASTCompoundStmt> Parser::parseCompoundStmt(bool isFuncBody) {
	std::shared_ptr<ASTCompoundStmt> retVal;
	
	if (peekAndConsume(TokenType::LBrace)) {
		if (!isFuncBody) mSymbolTable.enterScope();

		retVal = std::make_shared<ASTCompoundStmt>();

		std::shared_ptr<ASTDecl> decl;
		decl = parseDecl();

		while (decl != nullptr) {
			retVal->addDecl(decl);
			decl = parseDecl();
		}

        // preserve the last statment for check
		std::shared_ptr<ASTStmt> stmt, lastStmt; 
		stmt = parseStmt();

		while (stmt != nullptr) {
			retVal->addStmt(stmt);
			lastStmt = stmt;
			stmt = parseStmt();
		}

		if (!std::dynamic_pointer_cast<ASTReturnStmt>(lastStmt) && isFuncBody) {
			if (mCurrReturnType == Type::Void) retVal->addStmt(std::make_shared<ASTReturnStmt>(nullptr));
			else reportSemantError("crisp requires non-void functions to end with a return");
		}

		matchToken(TokenType::RBrace);

		if (!isFuncBody) mSymbolTable.exitScope();
	}
	
	return retVal;
}

std::shared_ptr<ASTStmt> Parser::parseAssignStmt() {
	std::shared_ptr<ASTStmt> retVal;
	std::shared_ptr<ASTArraySub> arraySub;
	
	if (mCurrToken.mType == TokenType::Identifier) {
		Identifier * ident = getVariable(mCurrToken.mStr);
		
		consumeToken();
		
		// now lets see if this is an array subscript
		if (peekAndConsume(TokenType::LBracket)) {
			try {
				std::shared_ptr<ASTExpr> expr = parseExpr();

				if (!expr) {
					throw ParseExceptMsg("Valid expression required inside [ ].");
				}
				
				arraySub = std::make_shared<ASTArraySub>(*ident, expr);
			} catch (ParseExcept& e) {
				// if this expr is bad consume until RBracket
				reportError(e);
				
                consumeUntil(TokenType::RBracket);

				if (mCurrToken.mType == TokenType::EndOfFile) {
					throw EOFExcept();
				}
			}
			
			matchToken(TokenType::RBracket);
		}
		
		// just because we got an identifier DOES NOT necessarily mean
		// this is an assign statement because there is a common left prefix between
		// AssignStmt and an ExprStmt with statements like:
		// id ;
		// id [ Expr ] ;
		// id ( FuncCallArgs ) ;
		
		// so... we see if the next token is a = and if it is then this is
		// an AssignStmt otherwise we set the "unused" variables
		// so parseFactor will later find it and be able to match

		int col = mCurrToken.mCol;
		if (peekAndConsume(TokenType::Assign)) {
			std::shared_ptr<ASTExpr> expr = parseExpr();
			
			if (!expr) {
				throw ParseExceptMsg("= must be followed by an expression");
			}
			
			// if we matched an array we want to make an array assign stmt
			if (arraySub) {
				// make sure the type of this expression matches the declared type
				Type subType;
                
                switch (arraySub->getType()) {
                    case Type::IntArray:
                        subType = Type::Int;
                        break;
                    case Type::DoubleArray:
                        subType = Type::Double;
                        break;
                    case Type::CharArray:
                        subType = Type::Char;
                        break;
                    default:
                        break;
                }

				if (subType != expr->getType()) {
					// dont do conversion at the moment
                    std::string err("Cannot assign an expression of type ");
                    err += getTypeText(expr->getType());
                    err += " to ";
                    err += getTypeText(subType);

                    reportSemantError(err, col);
				}

				retVal = std::make_shared<ASTAssignArrayStmt>(arraySub, expr);
			} else {
				// dont do conversion at the moment
				if (ident->getType() != expr->getType()) {
					std::string err("Cannot assign an expression of type ");
					err += getTypeText(expr->getType());
					err += " to ";
					err += getTypeText(ident->getType());

					reportSemantError(err, col);
				}

				if (ident->getType() == Type::DoubleArray || ident->getType() == Type::IntArray || ident->getType() == Type::CharArray) {
                    reportSemantError("Reassignment of arrays is not allowed", col);
                }

				retVal = std::make_shared<ASTAssignStmt>(*ident, expr);
			}
			
			matchToken(TokenType::SemiColon);
		} else {
			// we either have an unused array or an unused ident
			if (arraySub) mUnusedArray = arraySub;
			else mUnusedIdent = ident;
		}
	}
	
	return retVal;
}

std::shared_ptr<ASTIfStmt> Parser::parseIfStmt() {
	std::shared_ptr<ASTIfStmt> retVal;
	
	if (peekAndConsume(TokenType::KeyIf)) {
		matchToken(TokenType::LParen);

		std::shared_ptr<ASTExpr> expr = parseExpr();

		if (!expr) throw ParseExceptMsg("Invalid condition for if statement");
		
        matchToken(TokenType::RParen);

		std::shared_ptr<ASTStmt> stmt = parseStmt();
		std::shared_ptr<ASTStmt> elseStmt;

		if (peekAndConsume(TokenType::KeyElse)) elseStmt = parseStmt();

		retVal = std::make_shared<ASTIfStmt>(expr, stmt, elseStmt);
	}
	
	return retVal;
}

std::shared_ptr<ASTForStmt> Parser::parseForStmt() {
	std::shared_ptr<ASTForStmt> retVal;
	
	if (peekAndConsume(TokenType::KeyFor)) {
		// matchToken(TokenType::LParen);

        // // should be decl or assignment or empty
		// // std::shared_ptr<ASTExpr> expr = parseExpr();

		// if (!expr) throw ParseExceptMsg("Invalid condition for if statement");
		
        // matchToken(TokenType::RParen);

		// std::shared_ptr<ASTStmt> stmt = parseStmt();
		// std::shared_ptr<ASTStmt> elseStmt;

		// if (peekAndConsume(TokenType::KeyElse)) elseStmt = parseStmt();

		// retVal = std::make_shared<ASTIfStmt>(expr, stmt, elseStmt);
	}
	
	return retVal;
}

std::shared_ptr<ASTWhileStmt> Parser::parseWhileStmt() {
	std::shared_ptr<ASTWhileStmt> retVal;
	
	if (peekAndConsume(TokenType::KeyWhile)) {
		std::shared_ptr<ASTExpr> expr;
		std::shared_ptr<ASTStmt> stmt;
		
        matchToken(TokenType::LParen);
		
        expr = parseExpr();
		
        if (!expr) throw ParseExceptMsg("Invalid condition for while statement");
		
        matchToken(TokenType::RParen);

		stmt = parseStmt();

		retVal = std::make_shared<ASTWhileStmt>(expr, stmt);
	}
	
	return retVal;
}

std::shared_ptr<ASTReturnStmt> Parser::parseReturnStmt() {
	std::shared_ptr<ASTReturnStmt> retVal;
	
	if (peekAndConsume(TokenType::KeyReturn)) {
		if (peekIsOneOf({TokenType::SemiColon})) {
			retVal = std::make_shared<ASTReturnStmt>(nullptr);

			if (mCurrReturnType != Type::Void) reportSemantError("Invalid empty return in non-void function");
			
            consumeToken();
		} else {
			int col = mCurrToken.mCol;
			
            std::shared_ptr<ASTExpr> expr = parseExpr();

			// no conversion atm
			if (mCurrReturnType != expr->getType()) {
				std::string err("Expected type ");
				err += getTypeText(mCurrReturnType);
				err += " in return statement";

				reportSemantError(err, col);
			}

			retVal = std::make_shared<ASTReturnStmt>(expr);
			
            matchToken(TokenType::SemiColon);
		}
	}
	
	return retVal;
}

std::shared_ptr<ASTExprStmt> Parser::parseExprStmt() {
	std::shared_ptr<ASTExprStmt> retVal;
	
	std::shared_ptr<ASTExpr> e = parseExpr();
    if (e) {
		retVal = std::make_shared<ASTExprStmt>(e);
		matchToken(TokenType::SemiColon);
	}
	
	return retVal;
}

std::shared_ptr<ASTNullStmt> Parser::parseNullStmt() {
	std::shared_ptr<ASTNullStmt> retVal;
	
	if (peekAndConsume(TokenType::SemiColon)) retVal = std::make_shared<ASTNullStmt>();
	
	return retVal;
}