#include "../parse/astNodes.h"
#include "emitter.h"

// note all llvm headers are in emitter.h

llvm::Value * ASTProg::codegen(CodeContext& ctx) const noexcept {
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
	for (auto f : mFuncs) {
		f->codegen(ctx);
	}

	// program stored in LLVM Module so dont need to return anything
	return nullptr;
}

llvm::Value * ASTFunc::codegen(CodeContext& ctx) const noexcept {
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
	
	// Now emit the body
	mBody->codegen(ctx);
	
	return ctx.mFunc;
}

llvm::Value * ASTArgDecl::codegen(CodeContext& ctx) const noexcept {

}

llvm::Value * ASTDecl::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTCompoundStmt::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTIfStmt::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTReturnStmt::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTForStmt::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTWhileStmt::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTExprStmt::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTNullStmt::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTIdentExpr::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTArrayExpr::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTAssignOp::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTFuncExpr::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTLogicalAnd::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTLogicalOr::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTBinaryCmpOp::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTBinaryMathOp::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTNotExpr::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTIncExpr::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTDecExpr::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTAddrOfArray::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTStringExpr::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTConstantExpr::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTDoubleExpr::codegen(CodeContext& ctx) const noexcept {
}

llvm::Value * ASTCharExpr::codegen(CodeContext& ctx) const noexcept {
}