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
    // todo
}

llvm::Value * ASTIdentExpr::codegen(CodeContext& ctx) noexcept {
    return this->mIdent.readFrom(ctx);
}

llvm::Value * ASTArrayExpr::codegen(CodeContext& ctx) noexcept {
    // todo
}

llvm::Value * ASTAssignOp::codegen(CodeContext& ctx) noexcept {
    // todo
}

llvm::Value * ASTFuncExpr::codegen(CodeContext& ctx) noexcept {
    // todo
}

llvm::Value * ASTLogicalAnd::codegen(CodeContext& ctx) noexcept {
}

llvm::Value * ASTLogicalOr::codegen(CodeContext& ctx) noexcept {
}

llvm::Value * ASTBinaryCmpOp::codegen(CodeContext& ctx) noexcept {
}

llvm::Value * ASTBinaryMathOp::codegen(CodeContext& ctx) noexcept {
}

llvm::Value * ASTNotExpr::codegen(CodeContext& ctx) noexcept {
}

llvm::Value * ASTIncExpr::codegen(CodeContext& ctx) noexcept {
}

llvm::Value * ASTDecExpr::codegen(CodeContext& ctx) noexcept {
}

llvm::Value * ASTAddrOfArray::codegen(CodeContext& ctx) noexcept {
}

llvm::Value * ASTStringExpr::codegen(CodeContext& ctx) noexcept {
}

llvm::Value * ASTConstantExpr::codegen(CodeContext& ctx) noexcept {
}

llvm::Value * ASTDoubleExpr::codegen(CodeContext& ctx) noexcept {
}

llvm::Value * ASTCharExpr::codegen(CodeContext& ctx) noexcept {
}

llvm::Value * ASTNullStmt::codegen(CodeContext& ctx) noexcept {
    return nullptr;
}