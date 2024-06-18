#include "parse.h"

std::shared_ptr<ASTExpr> Parser::parseExpr() {
	std::shared_ptr<ASTExpr> retVal;
	
	// we should first get a AndTerm
	std::shared_ptr<ASTExpr> andTerm = parseAndTerm();
	
	// if we did not get an andTerm then this is not an expr
	if (andTerm) {
		retVal = andTerm;
		
        // check if this is followed by an op (optional)
		std::shared_ptr<ASTLogicalOr> exprPrime = parseExprPrime(retVal);
		
		if (exprPrime) {
			// if we got a exprPrime return this instead of just term
			retVal = exprPrime;
		}
	}
	
	return retVal;
}

std::shared_ptr<ASTLogicalOr> Parser::parseExprPrime(std::shared_ptr<ASTExpr> lhs) {
	std::shared_ptr<ASTLogicalOr> retVal;
	
	// must be ||
	if (mCurrToken.mType == TokenType::Or) {
		// make the binary cmp op
		TokenType op = mCurrToken.mType;

		retVal = std::make_shared<ASTLogicalOr>();

		int col = mCurrToken.mCol;

		consumeToken();
		
		// set the lhs to our parameter
		retVal->setLHS(lhs);
		
		// we MUST get an andTerm as the RHS of this operand
		std::shared_ptr<ASTExpr> rhs = parseAndTerm();
		
        if (!rhs) {
			throw OperandMissing(op);
		}
		
		retVal->setRHS(rhs);
		
		// finalize op
		if (!retVal->finalizeOp()) {
			std::string err("Cannot perform op between type ");
			err += getTypeText(lhs->getType());
			err += " and ";
			err += getTypeText(rhs->getType());

			reportSemantError(err, col);
		}

		// see comment in parseTermPrime if you are confused by this
		std::shared_ptr<ASTLogicalOr> exprPrime = parseExprPrime(retVal);

		if (exprPrime) {
			retVal = exprPrime;
		}
	}
	
	return retVal;
}

std::shared_ptr<ASTExpr> Parser::parseAndTerm() {
	std::shared_ptr<ASTExpr> retVal;
	std::shared_ptr<ASTLogicalAnd> prime;

	// this should not directly check factor but instead implement the proper grammar rule
	std::shared_ptr<ASTExpr> v = parseRelExpr();
	if (v) {
		retVal = v;
		prime = parseAndTermPrime(v);
		
        if (prime) retVal = prime;
	}
	
	return retVal;
}

std::shared_ptr<ASTLogicalAnd> Parser::parseAndTermPrime(std::shared_ptr<ASTExpr> lhs) {
	std::shared_ptr<ASTLogicalAnd> retVal;
	std::shared_ptr<ASTLogicalAnd> recursion;
	std::shared_ptr<ASTExpr> rhs;

	if (mCurrToken.mType == TokenType::And) {
		retVal = std::make_shared<ASTLogicalAnd>();

		retVal->setLHS(lhs);

		int col = mCurrToken.mCol;

		consumeToken();

		rhs = parseRelExpr();

		if (!rhs) throw OperandMissing(TokenType::And);
		
        retVal->setRHS(rhs);

		if (!retVal->finalizeOp()) {
			std::string err("Cannot perform op between type ");
			err += getTypeText(lhs->getType());
			err += " and ";
			err += getTypeText(rhs->getType());

			reportSemantError(err, col);
		}

		recursion = parseAndTermPrime(retVal);

		if (recursion) retVal = recursion;
	}
	
	return retVal;
}

std::shared_ptr<ASTExpr> Parser::parseRelExpr() {
	std::shared_ptr<ASTExpr> retVal;
	std::shared_ptr<ASTBinaryCmpOp> prime;

	std::shared_ptr<ASTExpr> v = parseNumExpr();
	if (v) {
		retVal = v;

		prime = parseRelExprPrime(v);

		if (prime) retVal = prime;
	}
	
	return retVal;
}

std::shared_ptr<ASTBinaryCmpOp> Parser::parseRelExprPrime(std::shared_ptr<ASTExpr> lhs) {
	std::shared_ptr<ASTBinaryCmpOp> retVal;
	std::shared_ptr<ASTBinaryCmpOp> recursion;
	std::shared_ptr<ASTExpr> rhs;
	
	if (peekIsOneOf({TokenType::EqualTo, TokenType::NotEqual, TokenType::LessThan, TokenType::GreaterThan, TokenType::LThanOrEq, TokenType::GThanOrEq})) {
		TokenType token = mCurrToken.mType;

		retVal = std::make_shared<ASTBinaryCmpOp>(token);

		int col = mCurrToken.mCol;

		consumeToken();

		retVal->setLHS(lhs);

		rhs = parseNumExpr();

		if (!rhs) throw OperandMissing(token);

		retVal->setRHS(rhs);

		if (!retVal->finalizeOp()) {
			std::string err("Cannot perform op between type ");
			err += getTypeText(lhs->getType());
			err += " and ";
			err += getTypeText(rhs->getType());

			reportSemantError(err, col);
		}

		recursion = parseRelExprPrime(retVal);

		if (recursion) retVal = recursion;
	}
	
	return retVal;
}

std::shared_ptr<ASTExpr> Parser::parseNumExpr() {
	std::shared_ptr<ASTExpr> retVal;
	std::shared_ptr<ASTBinaryMathOp> prime;
	
	std::shared_ptr<ASTExpr> v = parseTerm();
	if (v) {
		retVal = v;
		
        prime = parseNumExprPrime(v);
		
        if (prime) retVal = prime;
	}
	
	return retVal;
}

std::shared_ptr<ASTBinaryMathOp> Parser::parseNumExprPrime(std::shared_ptr<ASTExpr> lhs) {
	std::shared_ptr<ASTBinaryMathOp> retVal;
	std::shared_ptr<ASTBinaryMathOp> recursion;
	std::shared_ptr<ASTExpr> rhs;

	if (peekIsOneOf({TokenType::Plus, TokenType::Minus})) {
        TokenType token = mCurrToken.mType;

		retVal = std::make_shared<ASTBinaryMathOp>(token);

		int col = mCurrToken.mCol;

		consumeToken();

		retVal->setLHS(lhs);

		rhs = parseTerm();

		if (!rhs) throw OperandMissing(token);
		
        retVal->setRHS(rhs);

		if (!retVal->finalizeOp()) {
			std::string err("Cannot perform op between type ");
			err += getTypeText(lhs->getType());
			err += " and ";
			err += getTypeText(rhs->getType());

			reportSemantError(err, col);
		}

		recursion = parseNumExprPrime(retVal);

		if (recursion) retVal = recursion;
	}
	
	return retVal;
}


std::shared_ptr<ASTExpr> Parser::parseTerm() {
	std::shared_ptr<ASTExpr> retVal;
	std::shared_ptr<ASTBinaryMathOp> prime;

	
	std::shared_ptr<ASTExpr> v = parseValue();
	if (v) {
		retVal = v;

		prime = parseTermPrime(v);

		if (prime) retVal = prime;
	}
	
	return retVal;
}

std::shared_ptr<ASTBinaryMathOp> Parser::parseTermPrime(std::shared_ptr<ASTExpr> lhs) {
	std::shared_ptr<ASTBinaryMathOp> retVal;
	std::shared_ptr<ASTBinaryMathOp> recursion;
	std::shared_ptr<ASTExpr> rhs;

	if (peekIsOneOf({TokenType::Mult, TokenType::Div, TokenType::Mod})) {
		TokenType token = mCurrToken.mType;

		retVal = std::make_shared<ASTBinaryMathOp>(token);

		int col = mCurrToken.mCol;

		consumeToken();

		retVal->setLHS(lhs);

		rhs = parseValue();
		
        if (!rhs) throw OperandMissing(token);
		
        retVal->setRHS(rhs);

		if (!retVal->finalizeOp()) {
			std::string err("Cannot perform op between type ");
			err += getTypeText(lhs->getType());
			err += " and ";
			err += getTypeText(rhs->getType());

			reportSemantError(err, col);
		}
		
		recursion = parseTermPrime(retVal);

		if (recursion) retVal = recursion;
	}
	
	return retVal;
}

std::shared_ptr<ASTExpr> Parser::parseValue() {
	std::shared_ptr<ASTExpr> retVal;
	
	if (peekAndConsume(TokenType::Not)) {
		std::shared_ptr<ASTExpr> f = parseFactor();

		if (f) retVal = std::make_shared<ASTNotExpr>(f);
		else throw ParseExceptMsg("! must be followed by an expression.");
	} else {
       retVal = parseFactor(); 
    }
	
	return retVal;
}


std::shared_ptr<ASTExpr> Parser::parseFactor() {
	std::shared_ptr<ASTExpr> retVal;
	
	// try parse identifier factors FIRST so
	// we make sure to consume the mUnusedIdents
	// before we try any other rules
	
	if ((retVal = parseIdentFactor()));
	else if ((retVal = parseConstantFactor()));
	else if ((retVal = parseStringFactor()));
	else if ((retVal = parseParenFactor()));
	else if ((retVal = parseIncFactor()));
	else if ((retVal = parseDecFactor()));
	else if ((retVal = parseAddrOfArrayFactor()));
	
	return retVal;
}

// ( Expr )
std::shared_ptr<ASTExpr> Parser::parseParenFactor() {
	std::shared_ptr<ASTExpr> retVal;

	if (peekAndConsume(TokenType::LParen)) {
		retVal = parseExpr();
		
        if (!retVal) throw ParseExceptMsg("Not a valid expression inside parenthesis");
		
        matchToken(TokenType::RParen);
	}
	
	return retVal;
}

// constant
std::shared_ptr<ASTConstantExpr> Parser::parseConstantFactor() {
	std::shared_ptr<ASTConstantExpr> retVal;
	
	if (peekIsOneOf({TokenType::IntLit})) {
		retVal = std::make_shared<ASTConstantExpr>(mCurrToken.mStr);
		consumeToken();
	}

	return retVal;
}

// string
std::shared_ptr<ASTStringExpr> Parser::parseStringFactor() {
	std::shared_ptr<ASTStringExpr> retVal;

	if (peekIsOneOf({TokenType::StringLit})) {
		retVal = std::make_shared<ASTStringExpr>(mCurrToken.mStr, mStrings);
		consumeToken();
	}

	return retVal;
}

// id
// id [ Expr ]
// id ( FuncCallArgs )
std::shared_ptr<ASTExpr> Parser::parseIdentFactor() {
	std::shared_ptr<ASTExpr> retVal;
	if (mCurrToken.mType == TokenType::Identifier || mUnusedIdent || mUnusedArray) {
		if (mUnusedArray) {
			// "unused array" means that AssignStmt looked at this array
			// and decided it didn't want it, so it's already made an
			// array sub node
			retVal = std::make_shared<ASTArrayExpr>(mUnusedArray);
			mUnusedArray = nullptr;
		} else {
			Identifier* ident = nullptr;
			
			// If we have an "unused identifier," which means that
			// AssignStmt looked at this and decided it didn't want it,
			// that means we're already a token AFTER the identifier.
			if (mUnusedIdent) {
				ident = mUnusedIdent;
				mUnusedIdent = nullptr;
            } else {
				ident = getVariable(mCurrToken.mStr);
				consumeToken();
			}
			
			// Now we need to look ahead and see if this is an array
			// or function call reference, since id is a common
			// left prefix.
			if (mCurrToken.mType == TokenType::LBracket) {
				// Check to make sure this is an array
				if (ident->getType() != Type::IntArray && ident->getType() != Type::CharArray && !ident->isDummy()) {
					std::string err("'");
					err += ident->getName();
					err += "' is not an array";

					reportSemantError(err);

					consumeUntil(TokenType::RBracket);

					if (mCurrToken.mType == TokenType::EndOfFile) {
						throw EOFExcept();
					}
					
					matchToken(TokenType::RBracket);
					
					// just return our error variable
					retVal = std::make_shared<ASTIdentExpr>(*mSymbolTable.getIdentifier("@@variable"));
				} else {
					consumeToken();

					try {
						std::shared_ptr<ASTExpr> expr = parseExpr();

						if (!expr) {
							throw ParseExceptMsg("Valid expression required inside [ ]");
						}
						
						std::shared_ptr<ASTArraySub> array = std::make_shared<ASTArraySub>(*ident, expr);

						retVal = std::make_shared<ASTArrayExpr>(array);
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
			} else if (mCurrToken.mType == TokenType::LParen) {
				// check to make sure this is a function
				if (ident->getType() != Type::Function && !ident->isDummy()) {
					std::string err("'");
					err += ident->getName();
					err += "' is not a function";

					reportSemantError(err);

					consumeUntil(TokenType::RParen);

					if (mCurrToken.mType == TokenType::EndOfFile) {
						throw EOFExcept();
					}
					
					matchToken(TokenType::RParen);
					
					// just return our error variable
					retVal = std::make_shared<ASTIdentExpr>(*mSymbolTable.getIdentifier("@@variable"));
				} else {
					consumeToken();
					
                    // a function call can have zero or more arguments
					std::shared_ptr<ASTFuncExpr> funcCall = std::make_shared<ASTFuncExpr>(*ident);
					retVal = funcCall;
					
					// get the number of arguments for this function
					std::shared_ptr<ASTFunc> func = ident->getFunction();
					
					try {
						int currArg = 1, col = mCurrToken.mCol;

						std::shared_ptr<ASTExpr> arg = parseExpr();

						while (arg) {
							// check for validity of this argument (for non-dummy functions)
							if (!ident->isDummy()) {
								// special case for "printf" since we don't make a node for it
								if (ident->getName() == "printf") {
									mNeedPrintf = true;
									if (currArg == 1 && arg->getType() != Type::CharArray) {
										reportSemantError("The first parameter to printf must be a char[]");
									}
                                }
                            } else if (currArg > static_cast<int>(func->getNumArgs())) {
                                    std::string err("Function ");
                                    err += ident->getName();
                                    err += " takes only ";

                                    std::ostringstream ss;
                                    ss << func->getNumArgs();

                                    err += ss.str();
                                    err += " arguments";

                                    reportSemantError(err, col);
                            } else if (!func->checkArgType(currArg, arg->getType())) {
                                if (arg->getType() != func->getArgType(currArg)) {
                                    std::string err("Expected expression of type ");
                                    err += getTypeText(func->getArgType(currArg));
                                    
                                    reportSemantError(err, col);
                                }
							}
							
							funcCall->addArg(arg);
							
							currArg++;
							
							if (peekAndConsume(TokenType::Comma)) {
								col = mCurrToken.mCol;

								arg = parseExpr();

								if (!arg) throw ParseExceptMsg("Comma must be followed by expression in function call");
							} else {
								break;
							}
						}
					} catch (ParseExcept& e) {
						reportError(e);

						consumeUntil(TokenType::RParen);

						if (mCurrToken.mType == TokenType::EndOfFile) throw EOFExcept();
					}
					
					// now make sure we have the correct number of arguments
					if (!ident->isDummy()) {
						// Special case for printf
						if (ident->getName() == "printf") {
							if (funcCall->getNumArgs() == 0) {
								reportSemantError("printf requires a minimum of one argument");
							}
						} else if (funcCall->getNumArgs() < func->getNumArgs()) {
							std::string err("Function ");
							err += ident->getName();
							err += " requires ";

							std::ostringstream ss;
							ss << func->getNumArgs();

							err += ss.str();
							err += " arguments";

							reportSemantError(err);
						}
					}
					
					matchToken(TokenType::RParen);
				}
			} else {
				// just a plain old ident
				retVal = std::make_shared<ASTIdentExpr>(*ident);
			}
		}
	}
	
	return retVal;
}

// ++id
std::shared_ptr<ASTExpr> Parser::parseIncFactor() {
	std::shared_ptr<ASTExpr> retVal;
	
	if (peekAndConsume(TokenType::Inc)) {
		retVal = std::make_shared<ASTIncExpr>(*getVariable(mCurrToken.mStr));
		matchToken(TokenType::Identifier);
	}
	
	return retVal;
}

// --id
std::shared_ptr<ASTExpr> Parser::parseDecFactor() {
	std::shared_ptr<ASTExpr> retVal;
	
	if (peekAndConsume(TokenType::Dec)) {
		retVal = std::make_shared<ASTDecExpr>(*getVariable(mCurrToken.mStr));
		matchToken(TokenType::Identifier);
	}

	return retVal;
}

// &id[ Expr ]
std::shared_ptr<ASTExpr> Parser::parseAddrOfArrayFactor() {
	std::shared_ptr<ASTExpr> retVal;
	
	if (peekAndConsume(TokenType::Addr)) {
		if (!peekIsOneOf({TokenType::Identifier})) throw ParseExceptMsg("& must be followed by an identifier.");

		Identifier * ident = getVariable(mCurrToken.mStr);

		consumeToken();

		matchToken(TokenType::LBracket);

		std::shared_ptr<ASTExpr> expr = parseExpr();
		
        if (!expr) throw ParseExceptMsg("Missing required subscript expression.");
		
        matchToken(TokenType::RBracket);

		retVal = std::make_shared<ASTAddrOfArray>(std::make_shared<ASTArraySub>(*ident, expr));
	}
	
	return retVal;
}
