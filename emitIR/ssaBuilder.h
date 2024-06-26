/* 
defines the class SSABuilder using the algorithm outlined in
"Simple and Efficient Construction of SSA Form" (Braun et. al.)
*/

#ifndef SSABUILDER_H
#define SSABUILDER_H

#include <unordered_map>
#include <unordered_set>

// forward declare llvm classes to make compiler happy 
namespace llvm {
    class BasicBlock;
    class Value;
    class PHINode;
    class LLVMContext;
}

// in ../parse/symbols.h
class Identifier;

class SSABuilder {
public:
    SSABuilder(llvm::LLVMContext& ctx) noexcept;
    ~SSABuilder() noexcept = default;

	// called when a new function is started to clear out all the data
	void reset() noexcept;
	
	// for a specific variable in a specific basic block and write its value
	void writeVariable(Identifier * var, llvm::BasicBlock * block, llvm::Value * value) noexcept;
	
	// read the value assigned to the variable in the requested basic block
	// will recursively search predecessor blocks if it was not written in this block
	llvm::Value * readVariable(Identifier * var, llvm::BasicBlock * block) noexcept;
	
	// this is called to add a new block to the maps
	// if the block is sealed will automatically call sealBlock() on it
	void addBlock(llvm::BasicBlock * block, bool isSealed = false) noexcept;
	
	// this is called when a block is "sealed" which means it will not have any
	// further predecessors added and it will complete any PHI nodes (if necessary)
	void sealBlock(llvm::BasicBlock * block) noexcept;
private:	
	// recursively search predecessor blocks for a variable
	llvm::Value * readVariableRecursive(Identifier * var, llvm::BasicBlock * block) noexcept;
	
	// adds phi operands based on predecessors of the containing block
	llvm::Value * addPhiOperands(Identifier * var, llvm::PHINode * phi) noexcept;
	
	// removes trivial phi nodes
	llvm::Value * tryRemoveTrivialPhi(llvm::PHINode * phi) noexcept;
	
	typedef std::unordered_map<Identifier *, llvm::Value *> SubMap;
	typedef std::unordered_map<Identifier *, llvm::PHINode *> SubPHI;
	
	// stores the variable definitions for a particular basic block
	std::unordered_map<llvm::BasicBlock *, SubMap *> mVarDefs;
	
	// stores any incomplete PHI nodes
	std::unordered_map<llvm::BasicBlock *, SubPHI *> mIncompletePhis;
	
	// set of all the sealed blocks in the current function
	std::unordered_set<llvm::BasicBlock *> mSealedBlocks;

    // so we can grab required llvm Values and Types for readVariableRecursive
    llvm::LLVMContext& mCtx;
};

#endif