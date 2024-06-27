#include "../parse/astNodes.h"
#include "emitter.h"

// note all llvm headers are in emitter.h

llvm::Value * ASTProg::codegen(CodeContext& ctx) noexcept {
    // add global constant strings from StringTable to module 
	ctx.mStrings.codegen(ctx);

    // if we need to emit declaration for stdlib "printf" 
    if (ctx.mPrintfIdent) {
        // add args 
        std::vector<llvm::Type *> printfArgs;

        // push pointer back to first arg 
    	printfArgs.push_back(llvm::PointerType::get(llvm::Type::getInt8Ty(*ctx.mGlobalContext), 0));
        
        // type for function
    	llvm::FunctionType * printfType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*ctx.mGlobalContext), printfArgs, true);
        
        // create function
    	llvm::Function * func = llvm::Function::Create(printfType, llvm::GlobalValue::LinkageTypes::ExternalLinkage, "printf", *ctx.mModule);

        // use C calling convention
        // ensures that the function printf adheres to the standard calling conventions 
        // crucial for LLVM IR code to interface with C stdlib 
    	func->setCallingConv(llvm::CallingConv::C);
        
    	// map the printf ident to this function
    	ctx.mPrintfIdent->setAddress(func);
    }

    // emit code for all the functions
	for (auto f : this->mFuncs) {
		f->codegen(ctx);
	}

	// program stored in LLVM Module so dont need to return anything
	return nullptr;
}

llvm::Value * ASTFunc::codegen(CodeContext& ctx) noexcept {
    llvm::FunctionType * funcType = nullptr;
	
	// get the return type 
	llvm::Type * retType = nullptr;
    switch (mReturnType) {
        case Type::Int:
            retType = llvm::Type::getInt32Ty(*ctx.mGlobalContext);
            break;
        case Type::Double:
            retType = llvm::Type::getDoubleTy(*ctx.mGlobalContext);
            break;
        case Type::Void:
            retType = llvm::Type::getVoidTy(*ctx.mGlobalContext);
            break;
        case Type::Char:
            retType = llvm::Type::getInt8Ty(*ctx.mGlobalContext);
            break;
        default:
            break;
    }

    // return type of func + return type of args
	if (mArgs.size() == 0) {
        funcType = llvm::FunctionType::get(retType, false);
	} else {
		std::vector<llvm::Type *> args;

		for (auto arg : mArgs) {
            // llvmType returns LLVM Value for said argument  
			args.push_back(arg->getIdent().llvmType(*ctx.mGlobalContext));
		}
		
		funcType = llvm::FunctionType::get(retType, args, false);
	}
	
	// create function and make it the current one
	ctx.mFunc = llvm::Function::Create(funcType, llvm::GlobalValue::LinkageTypes::ExternalLinkage, mIdent.getName(), *ctx.mModule);
	
    // wow that we have a new function reset our SSA builder
	ctx.mSSA.reset();

	// map the ident to this function
	mIdent.setAddress(ctx.mFunc);
	
	// create function and make it the current one
	ctx.mBlock = llvm::BasicBlock::Create(*ctx.mGlobalContext, "entry block", ctx.mFunc);
	
    // add and seal this block
	ctx.mSSA.addBlock(ctx.mBlock, true);

	// if we have arguments we need to set the name/value of them
	if (mArgs.size() > 0) {
		llvm::Function::arg_iterator iter = ctx.mFunc->arg_begin();
		llvm::Function::arg_iterator end = ctx.mFunc->arg_end();
		
        int i = 0;
		while (iter != end) {
			Identifier& argIdent = mArgs[i]->getIdent();
			
            iter->setName(argIdent.getName());
			
			argIdent.writeTo(ctx, iter);
			
			++i;
			++iter;
		}
	}
	
	ctx.mFunc->setCallingConv(llvm::CallingConv::C);
	
	// add all the declarations for variables created in this function
	mScopeTable.codegen(ctx);
	
	// now emit the body
	this->mBody->codegen(ctx);
	
	return ctx.mFunc;
}

// nothing to emit
llvm::Value * ASTArgDecl::codegen(CodeContext& ctx) noexcept {
    return nullptr;
}

llvm::Value * ASTDecl::codegen(CodeContext& ctx) noexcept {
    // if there is an expression emit this and store it in the ident
	if (mExpr) {
		llvm::Value * declExpr = this->mExpr->codegen(ctx);
		
		llvm::IRBuilder<> build(ctx.mBlock);
		// If this is a string, we have to memcpy
		if (declExpr->getType()->isPointerTy()) {
			// this address should already be saved
			llvm::Value * arrayLoc = mIdent.readFrom(ctx);
			
			// GEP the address of the src
			std::vector<llvm::Value *> gepIdx;
			gepIdx.push_back(llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext)));
			gepIdx.push_back(llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext)));
            
			llvm::Value *  src = build.CreateGEP(declExpr->getType(), declExpr, gepIdx);

			// memcpy into the array
			// memcpy(dest, src, size, align, volatile)
			build.CreateMemCpy(arrayLoc, llvm::MaybeAlign(1), src, llvm::MaybeAlign(1), mIdent.getArrayCount(), false);
		} else {
			// basic types can just be written
			mIdent.writeTo(ctx, declExpr);
		}
	}
	
	return nullptr;
}

llvm::Value * ASTCompoundStmt::codegen(CodeContext& ctx) noexcept {
    for (auto &x : this->mStmts) x->codegen(ctx);

	return nullptr;
}

llvm::Value * ASTIfStmt::codegen(CodeContext& ctx) noexcept {
    auto value = this->mExpr->codegen(ctx);

    llvm::IRBuilder<> builder(ctx.mBlock);

    // check bool
    if (!value->getType()->isIntegerTy(1)) {
        // if not bool make it
        value = builder.CreateICmpNE(value, llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext))); 
    }
    
    auto thenBody = llvm::BasicBlock::Create(*ctx.mGlobalContext, "if.then", ctx.mFunc);
    ctx.mSSA.addBlock(thenBody, true);

    auto end = llvm::BasicBlock::Create(*ctx.mGlobalContext, "if.end", ctx.mFunc);
    ctx.mSSA.addBlock(end);

    if (mElseStmt) {
        auto elseBody = llvm::BasicBlock::Create(*ctx.mGlobalContext, "if.else", ctx.mFunc);

        ctx.mSSA.addBlock(elseBody, true);
        builder.CreateCondBr(value, thenBody, elseBody);

        ctx.mBlock = elseBody;

        mElseStmt->codegen(ctx);
        
        llvm::IRBuilder<> builderElse(ctx.mBlock);
        builderElse.CreateBr(end);
    } else { 
        builder.CreateCondBr(value, thenBody, end);
    }
    
    ctx.mBlock = thenBody;

    mThenStmt->codegen(ctx);

    llvm::IRBuilder<> builderThen(ctx.mBlock);
    builderThen.CreateBr(end);

    ctx.mSSA.sealBlock(end);

    ctx.mBlock = end;

	return nullptr;
}

llvm::Value * ASTReturnStmt::codegen(CodeContext& ctx) noexcept {
    llvm::IRBuilder<> builder(ctx.mBlock);

    if (this->mExpr) builder.CreateRet(this->mExpr->codegen(ctx));
    else builder.CreateRetVoid();

	return nullptr;
}

llvm::Value * ASTForStmt::codegen(CodeContext& ctx) noexcept {
    // TODO
    return nullptr;
}

llvm::Value * ASTWhileStmt::codegen(CodeContext& ctx) noexcept {
    auto cond = llvm::BasicBlock::Create(*ctx.mGlobalContext, "while.cond", ctx.mFunc);
    ctx.mSSA.addBlock(cond);
    
    llvm::IRBuilder<> builder(ctx.mBlock);
    builder.CreateBr(cond); // unconditional branch in predecessor

    ctx.mBlock = cond;

    auto value = this->mExpr->codegen(ctx);
    llvm::IRBuilder<> builderCond(ctx.mBlock);

    // check bool
    if (!value->getType()->isIntegerTy(1)) {
        // if not bool make it
        value = builder.CreateICmpNE(value, llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext))); 
    }

    // after exprs codegen
    auto body = llvm::BasicBlock::Create(*ctx.mGlobalContext, "while.body", ctx.mFunc); 
    ctx.mSSA.addBlock(body, true);

    // conditional branch in while.cond
    auto end = llvm::BasicBlock::Create(*ctx.mGlobalContext, "while.end", ctx.mFunc);
    ctx.mSSA.addBlock(end, true);
    builderCond.CreateCondBr(value, body, end); 

    ctx.mBlock = body;
    this->mLoopStmt->codegen(ctx);
    llvm::IRBuilder<> builderBody(ctx.mBlock);
    builderBody.CreateBr(cond);
    ctx.mSSA.sealBlock(cond);
    ctx.mBlock = end;
    
	return nullptr;
}

llvm::Value * ASTExprStmt::codegen(CodeContext& ctx) noexcept {
    this->mExpr->codegen(ctx);

    return nullptr;
}

llvm::Value * ASTIdentExpr::codegen(CodeContext& ctx) noexcept {
    return this->mIdent.readFrom(ctx);
}

llvm::Value * ASTArrayExpr::codegen(CodeContext& ctx) noexcept {
    // evaluate the sub expression to get the desired index
	llvm::Value * arrayIdx = this->mExpr->codegen(ctx);
	
	// this address should already be saved
	llvm::Value * addr = mIdent.readFrom(ctx);
	
	// GEP from the array address
	llvm::IRBuilder<> build(ctx.mBlock);

    // generate the array subscript which will give us the address
	llvm::Value * gep = build.CreateInBoundsGEP(addr->getType(), addr, arrayIdx);
	
	// this still needs to be a loaded because arrays are in memory
	return build.CreateLoad(addr->getType(), addr);
}

llvm::Value * ASTAssignOp::codegen(CodeContext& ctx) noexcept {
    llvm::Value * retVal = nullptr;

    llvm::IRBuilder<> build(ctx.mBlock);

    llvm::Value * lhs = this->mLHS->codegen(ctx);
    llvm::Value * rhs = this->mRHS->codegen(ctx);

    // do we have an ASTIdentExpr or ASTIdentExpr
    if (auto check = std::dynamic_pointer_cast<ASTArrayExpr>(this->mLHS)) { 
        // for array idents
        switch (this->mOp) {
            case TokenType::Assign:
                build.CreateStore(rhs, lhs);
                
                // store the result back into lhs
                retVal = rhs;

                break;
            case TokenType::DecAssign: {
                llvm::Value * subValue = build.CreateSub(lhs, rhs, "-=");

                // store the result back into lhs
                build.CreateStore(subValue, lhs);

                retVal = subValue;

                break;
            }
            case TokenType::IncAssign: {
                llvm::Value * incValue = build.CreateAdd(lhs, rhs);

                // store the result back into lhs
                build.CreateStore(incValue, lhs, "+=");

                retVal = incValue;

                break;
            }
            default:
                break;
        }
    } else if (auto check = std::dynamic_pointer_cast<ASTIdentExpr>(this->mLHS)) { 
        // for idents
        switch (this->mOp) {
            case TokenType::Assign:
                check->mIdent.writeTo(ctx, rhs);  

                // store the result back into lhs
                retVal = rhs;

                break;
            case TokenType::DecAssign: {
                llvm::Value * subValue = build.CreateSub(lhs, rhs, "-=");

                // store the result back into lhs
                check->mIdent.writeTo(ctx, subValue);  

                retVal = subValue;

                break;
            }
            case TokenType::IncAssign: {
                llvm::Value * incValue = build.CreateAdd(lhs, rhs, "+=");

                // store the result back into lhs
                check->mIdent.writeTo(ctx, incValue);  

                retVal = incValue;

                break;
            }
            default:
                break;
        }
    }

    // assign is an expr so we need to return a value
	return retVal;
}

llvm::Value * ASTFuncExpr::codegen(CodeContext& ctx) noexcept {
    // at this point we can assume the argument types match
	// create the list of arguments
	std::vector<llvm::Value *> callList;
	for (auto arg : this->mArgs) {
		llvm::Value * argValue = arg->codegen(ctx);
		// if this is an array or ptr we need to change this to a getelemptr
		// provided it already isn't one
		if (!llvm::isa<llvm::GetElementPtrInst>(argValue) && argValue->getType()->isPointerTy()) {
			if (argValue->getType()->isArrayTy()) {
				llvm::IRBuilder<> build(ctx.mBlock);

				std::vector<llvm::Value*> gepIdx;
				gepIdx.push_back(llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext)));
				gepIdx.push_back(llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext)));
				
				argValue = build.CreateInBoundsGEP(argValue->getType(), argValue, gepIdx);
			} else {
				llvm::IRBuilder<> build(ctx.mBlock);				
				// need to return the address of the specific index in question so need a GEP
				argValue = build.CreateInBoundsGEP(argValue->getType(), argValue, llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext)));
			}
		}
			
		callList.push_back(argValue);
	}
	
	// now call the function and return it
	llvm::Value * retVal = nullptr;
	
	llvm::IRBuilder<> build(ctx.mBlock);

    // get the llvm function 
    llvm::Function * func = llvm::cast<llvm::Function>(this->mIdent.getAddress());

    // create callable entity in LLVM IR
    llvm::FunctionCallee funcCallee(func);
    
	if (this->mType != Type::Void) {
		retVal = build.CreateCall(funcCallee, callList, "call");
	} else {
		build.CreateCall(funcCallee, callList, "call-void");
	}
	
	return retVal;
}

llvm::Value * ASTLogicalAnd::codegen(CodeContext& ctx) noexcept {	
	// create the block for the RHS
	llvm::BasicBlock * rhsBlock = llvm::BasicBlock::Create(*ctx.mGlobalContext, "and.rhs", ctx.mFunc);
	
    // add the rhs block to SSA (not sealed)
	ctx.mSSA.addBlock(rhsBlock);
	
	// in both "true" and "false" condition we will jump to and.end
	// this is because we will insert a phi node that assumes false
	// if the and.end jump was from the lhs block
	llvm::BasicBlock * endBlock = llvm::BasicBlock::Create(*ctx.mGlobalContext, "and.end", ctx.mFunc);
	
    // also not sealed
	ctx.mSSA.addBlock(endBlock);
	
	// now generate the LHS
	llvm::Value * lhsVal = mLHS->codegen(ctx);

	llvm::BasicBlock * lhsBlock = ctx.mBlock;
	
	// add the branch to the end of the LHS
	{
		llvm::IRBuilder<> build(ctx.mBlock);
		// we can assume it will be an i32 here
		// since it would have been zero-extended otherwise
		lhsVal = build.CreateICmpNE(lhsVal, llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext)), "tobool");
		build.CreateCondBr(lhsVal, rhsBlock, endBlock);
	}
	
	// rhsBlock should now be sealed
	ctx.mSSA.sealBlock(rhsBlock);
	
	// code should now be generated in the RHS block
	ctx.mBlock = rhsBlock;

	llvm::Value * rhsVal = mRHS->codegen(ctx);
	
	// this is the final RHS block (for the phi node)
	rhsBlock = ctx.mBlock;
	
	// add the branch and the end of the RHS
	{
		llvm::IRBuilder<> build(ctx.mBlock);
		rhsVal = build.CreateICmpNE(rhsVal, llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext)), "tobool");
		
		// We do an unconditional branch because the phi mode will handle
		// the correct value
		build.CreateBr(endBlock);
	}
	
	// endBlock should now be sealed
	ctx.mSSA.sealBlock(endBlock);
	
	ctx.mBlock = endBlock;
	
	llvm::IRBuilder<> build(ctx.mBlock);
	
	// figure out the value to zext
	llvm::Value * zextVal = nullptr;
	
	// if rhs is true we need to make a phi
	if (rhsVal != llvm::ConstantInt::getFalse(*ctx.mGlobalContext)) {
		llvm::PHINode * phi = build.CreatePHI(llvm::Type::getInt1Ty(*ctx.mGlobalContext), 2);
		
        // if we came from the lhs it had to be false
		phi->addIncoming(llvm::ConstantInt::getFalse(*ctx.mGlobalContext), lhsBlock);
		
        phi->addIncoming(rhsVal, rhsBlock);

		zextVal = phi;
	} else {
		zextVal = llvm::ConstantInt::getFalse(*ctx.mGlobalContext);
	}
	
	return build.CreateZExt(zextVal, llvm::Type::getInt32Ty(*ctx.mGlobalContext));
}

llvm::Value * ASTLogicalOr::codegen(CodeContext& ctx) noexcept {
	// create the block for the RHS
	llvm::BasicBlock * rhsBlock = llvm::BasicBlock::Create(*ctx.mGlobalContext, "or.rhs", ctx.mFunc);
	
    // add the rhs block to SSA (not sealed)
	ctx.mSSA.addBlock(rhsBlock);
	
	// in both "true" and "false" condition we will jump to and.end
	// this is because we will insert a phi node that assumes false
	// if the and.end jump was from the lhs block
	llvm::BasicBlock * endBlock = llvm::BasicBlock::Create(*ctx.mGlobalContext, "or.end", ctx.mFunc);
	
    // also not sealed
	ctx.mSSA.addBlock(endBlock);
	
	// now generate the LHS
	llvm::Value * lhsVal = mLHS->codegen(ctx);

	llvm::BasicBlock * lhsBlock = ctx.mBlock;
	
	// add the branch to the end of the LHS
	{
		llvm::IRBuilder<> build(ctx.mBlock);
		// we can assume it will be an i32 here
		// since it would have been zero-extended otherwise
		lhsVal = build.CreateICmpNE(lhsVal, llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext)), "tobool");
		build.CreateCondBr(lhsVal, rhsBlock, endBlock);
	}
	
	// rhsBlock should now be sealed
	ctx.mSSA.sealBlock(rhsBlock);
	
	// code should now be generated in the RHS block
	ctx.mBlock = rhsBlock;

	llvm::Value * rhsVal = mRHS->codegen(ctx);
	
	// this is the final RHS block (for the phi node)
	rhsBlock = ctx.mBlock;
	
	// add the branch and the end of the RHS
	{
		llvm::IRBuilder<> build(ctx.mBlock);
		rhsVal = build.CreateICmpNE(rhsVal, llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext)), "tobool");
		
		// We do an unconditional branch because the phi mode will handle
		// the correct value
		build.CreateBr(endBlock);
	}
	
	// endBlock should now be sealed
	ctx.mSSA.sealBlock(endBlock);
	
	ctx.mBlock = endBlock;
	
	llvm::IRBuilder<> build(ctx.mBlock);
	
	// figure out the value to zext
	llvm::Value * zextVal = nullptr;
	
	// if rhs is false we need to make a phi
	if (rhsVal != llvm::ConstantInt::getTrue(*ctx.mGlobalContext)) {
		llvm::PHINode * phi = build.CreatePHI(llvm::Type::getInt1Ty(*ctx.mGlobalContext), 2);
		
        // if we came from the lhs it had to be true
		phi->addIncoming(llvm::ConstantInt::getTrue(*ctx.mGlobalContext), lhsBlock);
		
        phi->addIncoming(rhsVal, rhsBlock);

		zextVal = phi;
	} else {
		zextVal = llvm::ConstantInt::getTrue(*ctx.mGlobalContext);
	}
	
	return build.CreateZExt(zextVal, llvm::Type::getInt32Ty(*ctx.mGlobalContext));
}

llvm::Value * ASTBinaryCmpOp::codegen(CodeContext& ctx) noexcept {
    llvm::Value * retVal = nullptr;
	
    llvm::IRBuilder<> builder(ctx.mBlock);
    
    llvm::Value * rhs = this->mRHS->codegen(ctx);
    llvm::Value * lhs = this->mLHS->codegen(ctx);
    
    switch (this->mOp) {
        case TokenType::EqualTo:
            retVal = builder.CreateICmpEQ(lhs, rhs, "==");
            break;
        case TokenType::NotEqual:
            retVal = builder.CreateICmpNE(lhs, rhs, "!=");
            break;
        case TokenType::GreaterThan:
            retVal = builder.CreateICmpSGT(lhs, rhs, ">");
            break;
        case TokenType::LessThan:
            retVal = builder.CreateICmpSLT(lhs, rhs, "<");
            break;
        case TokenType::GThanOrEq:
            retVal = builder.CreateICmpSGE(lhs, rhs, ">=");
            break;
        case TokenType::LThanOrEq:
            retVal = builder.CreateICmpSLE(lhs, rhs, "<=");
            break;
        default:
            break;
    }
	
	return retVal;
}

llvm::Value * ASTBinaryMathOp::codegen(CodeContext& ctx) noexcept {
        llvm::Value * retVal = nullptr;
	
    llvm::IRBuilder<> builder(ctx.mBlock);
    
    llvm::Value * rhs = this->mRHS->codegen(ctx);
    llvm::Value * lhs = this->mLHS->codegen(ctx);
    
    switch (this->mOp) {
        case TokenType::Plus:
            retVal = builder.CreateAdd(lhs, rhs, "+");
            break;
        case TokenType::Minus:
            retVal = builder.CreateSub(lhs, rhs, "-");
            break;
        case TokenType::Mult:
            retVal = builder.CreateMul(lhs, rhs, "*");
            break;
        case TokenType::Div:
            retVal = builder.CreateSDiv(lhs, rhs, "/");
            break;
        case TokenType::Mod:
            retVal = builder.CreateSRem(lhs, rhs, "%");
            break;
        default:
            break;
    }
	
	return retVal;
}

llvm::Value * ASTNotExpr::codegen(CodeContext& ctx) noexcept {
    llvm::Value * retVal = nullptr;
	
    llvm::IRBuilder<> builder(ctx.mBlock);

	llvm::Value * value = this->mExpr->codegen(ctx);

    value = builder.CreateICmpEQ(value, llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext)));
    
    retVal = builder.CreateZExt(value, llvm::Type::getInt32Ty(*ctx.mGlobalContext));

	return retVal;
}

llvm::Value * ASTIncExpr::codegen(CodeContext& ctx) noexcept {
	llvm::IRBuilder<> builder(ctx.mBlock);

    llvm::Value * value = mIdent.readFrom(ctx);

    // use the same type as value
    value = builder.CreateAdd(value, llvm::ConstantInt::get(value->getType(), 1), "++ident");

    mIdent.writeTo(ctx, value);

	return value;
}

llvm::Value * ASTDecExpr::codegen(CodeContext& ctx) noexcept {
    llvm::IRBuilder<> builder(ctx.mBlock);

    llvm::Value * value = mIdent.readFrom(ctx);

    // use the same type as value
    value = builder.CreateSub(value, llvm::ConstantInt::get(value->getType(), 1), "--ident");

    mIdent.writeTo(ctx, value);

	return value;
}

llvm::Value * ASTAddrOfArray::codegen(CodeContext& ctx) noexcept {
	return this->mArray->codegen(ctx);
}

llvm::Value * ASTStringExpr::codegen(CodeContext& ctx) noexcept {
    return this->mString->getValue();
}

llvm::Value * ASTConstantExpr::codegen(CodeContext& ctx) noexcept {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx.mGlobalContext), mValue);
}

llvm::Value * ASTDoubleExpr::codegen(CodeContext& ctx) noexcept {
    return llvm::ConstantInt::get(llvm::Type::getDoubleTy(*ctx.mGlobalContext), mValue);
}

llvm::Value * ASTCharExpr::codegen(CodeContext& ctx) noexcept {
    return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*ctx.mGlobalContext), mValue);
}

llvm::Value * ASTNullStmt::codegen(CodeContext& ctx) noexcept {
    return nullptr;
}