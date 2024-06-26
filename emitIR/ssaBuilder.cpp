#include "../parse/symbols.h"
#include "emitter.h"

// note all llvm headers are in emitter.h

SSABuilder::SSABuilder(llvm::LLVMContext& ctx) noexcept
: mCtx {ctx} { }

// called when a new function is started to clear out all the data
void SSABuilder::reset() noexcept {
    for (auto& i : mVarDefs) delete i.second;
    mVarDefs.clear();
    
    for (auto & i : mIncompletePhis) delete i.second;
    mIncompletePhis.clear();

    mSealedBlocks.clear();
}

// for a specific variable in a specific basic block and write its value
void SSABuilder::writeVariable(Identifier* var, llvm::BasicBlock* block, llvm::Value* value) noexcept {
    (*mVarDefs[block])[var] = value;
}

// read the value assigned to the variable in the requested basic block
// will recursively search predecessor blocks if it was not written in this block
llvm::Value * SSABuilder::readVariable(Identifier * var, llvm::BasicBlock * block) noexcept {
    auto& subMap = mVarDefs[block];
    
    if (subMap->find(var) != subMap->end()) return (*subMap)[var];

    return readVariableRecursive(var, block);
}

// this is called to add a new block to the maps
// if the block is sealed will automatically call sealBlock() on it
void SSABuilder::addBlock(llvm::BasicBlock * block, bool isSealed /* = false */) noexcept {
    mVarDefs.emplace(block, new SubMap());
    
    mIncompletePhis.emplace(block, new SubPHI());

    if (isSealed) sealBlock(block);
}

// this is called when a block is "sealed" which means it will not have any
// further predecessors added and it will complete any PHI nodes (if necessary)
void SSABuilder::sealBlock(llvm::BasicBlock * block) noexcept {
    for (auto& i : *mIncompletePhis[block]) addPhiOperands(i.first, i.second);
    
    mSealedBlocks.emplace(block);
}

// recursively search predecessor blocks for a variable
llvm::Value * SSABuilder::readVariableRecursive(Identifier * var, llvm::BasicBlock * block) noexcept {
    llvm::Value * val = nullptr;

    if (mSealedBlocks.find(block) == mSealedBlocks.end()) {
        llvm::BasicBlock::iterator it = block->getFirstNonPHIIt();

        if (it == block->end()) {
            val = llvm::PHINode::Create(var->llvmType(mCtx), 0, "Phi", block);
        } else {
            val = llvm::PHINode::Create(var->llvmType(mCtx), 0, "Phi", block->getFirstNonPHI());
        }

        (*mIncompletePhis[block])[var] = llvm::cast<llvm::PHINode>(val);
    } else if (block->getSinglePredecessor()) {
        val = readVariable(var, block->getSinglePredecessor());
    } else {
        llvm::BasicBlock::iterator it = block->getFirstNonPHIIt();

        if (it == block->end()) {
            val = llvm::PHINode::Create(var->llvmType(mCtx), 0, "Phi", block);
        } else {
            val = llvm::PHINode::Create(var->llvmType(mCtx), 0, "Phi", block->getFirstNonPHI());
        }

        writeVariable(var, block, val);

        val = addPhiOperands(var, llvm::cast<llvm::PHINode>(val));
    }

    writeVariable(var, block, val);
	
	return val;
}

// adds phi operands based on predecessors of the containing block
llvm::Value * SSABuilder::addPhiOperands(Identifier * var, llvm::PHINode * phi) noexcept {
    llvm::BasicBlock * block = phi->getParent();

    for (auto it = pred_begin(block); it != pred_end(block); it++) {
        phi->addIncoming(readVariable(var, *it), *it);
    }
    
    return tryRemoveTrivialPhi(phi);
}

// removes trivial phi nodes
llvm::Value * SSABuilder::tryRemoveTrivialPhi(llvm::PHINode * phi) noexcept {
    llvm::Value * same = nullptr;
	
    for (int i = 0; i < phi->getNumIncomingValues(); i++) {
        llvm::Value *  op = phi->getIncomingValue(i);
        
        if (op == same || op == phi) continue;
        
        if (same != nullptr) return phi;
        
        same = op;
    }

    if (same == nullptr) same = llvm::UndefValue::get(phi->getType());

    std::vector<llvm::User *> users;
    for (auto it = phi->user_begin(); it != phi->user_end(); it++) {
        llvm::User * use = *it;

        if (use != phi) users.emplace_back(use);
    }

    phi->replaceAllUsesWith(same);

    for (auto& b: mVarDefs) {
        for (auto& map: *b.second) {
            if (map.second == phi) {
                (*mVarDefs[b.first])[map.first] = same;       
            }  
        }
    }

    phi->eraseFromParent();
    
    for (auto& use : users) {
        if (llvm::isa<llvm::PHINode>(use)) {
            tryRemoveTrivialPhi(llvm::cast<llvm::PHINode>(use));
        }
    }

	return same;
}