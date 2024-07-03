#include <sstream>
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Transforms/Utils.h" 

#include "../parse/astNodes.h"
#include "../parse/parse.h"
#include "../parse/symbols.h"
#include "emitter.h"

CodeContext::CodeContext(StringTable& strings, const char * file) noexcept
: mGlobalContext {std::make_unique<llvm::LLVMContext>()}
, mModule {std::make_unique<llvm::Module>(file, *mGlobalContext)}
, mStrings {strings}
, mFunc {nullptr}
, mBlock {nullptr}
, mPrintfIdent {nullptr} { }

Emitter::Emitter(Parser& parser) noexcept
: mCodeContext {parser.mStringTable, parser.mFileName} {
	if (parser.mNeedPrintf) {
		mCodeContext.mPrintfIdent = parser.mSymbolTable.getIdentifier("printf");
	}
		
	// this is what kicks off the generation of the LLVM IR from the AST
	parser.mRoot->codegen(mCodeContext);
}

void Emitter::print() noexcept {
	mCodeContext.mModule->print(llvm::outs(), nullptr);
}

bool Emitter::verify() noexcept {
	std::string errMsg;
    llvm::raw_string_ostream errorStream(errMsg);

	// returns true if failed
    bool result = llvm::verifyModule(*mCodeContext.mModule, &errorStream);

    if (result) {
		llvm::errs() << "\nLLVM module verification failed:\n" << errMsg;
		llvm::errs() << "\ncrisp: error: Emitted bad IR. Compilation halted.\n";
	}

	return !result;
}

void Emitter::optimize() noexcept {
	using namespace llvm;

	// https://llvm.org/docs/NewPassManager.html#id2

	// Create the analysis managers.
	// These must be declared in this order so that they are destroyed in the
	// correct order due to inter-analysis-manager references.
	LoopAnalysisManager LAM;
	FunctionAnalysisManager FAM;
	CGSCCAnalysisManager CGAM;
	ModuleAnalysisManager MAM;

	// Create the new pass manager builder.
	// Take a look at the PassBuilder constructor parameters for more
	// customization, e.g. specifying a TargetMachine or various debugging
	// options.
	PassBuilder PB;

	// Registers a callback function (lambda function) that LLVM will invoke when setting up the optimization pipeline
	// PB.registerPipelineStartEPCallback(
	// [&](ModulePassManager &MPM, OptimizationLevel Level) {
	//   MPM.addPass(llvm::createPromoteMemoryToRegisterPass());
	// });

	// Register all the basic analyses with the managers.
	PB.registerModuleAnalyses(MAM);
	PB.registerCGSCCAnalyses(CGAM);
	PB.registerFunctionAnalyses(FAM);
	PB.registerLoopAnalyses(LAM);
	PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

	// Create the pass manager.
	// This one corresponds to a typical -O3 optimization pipeline.
	llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(OptimizationLevel::O3);

	// Optimize the IR!
	MPM.run(*mCodeContext.mModule, MAM);
}

void Emitter::bitcode() noexcept {
	// using namespace llvm;

	// auto TargetTriple = sys::getDefaultTargetTriple();
}