/*
defines the classes needed to perform semantic analysis i.e. class Identifier, ScopeTable, SymbolTable, StringTable
*/

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "types.h"

class ASTFunc;

class Identifier {
public:
    friend class SymbolTable;

    Identifier(const std::string& name) noexcept
    : mName {name}
    , mFunction {nullptr}
    , mType {Type::Void}
    , mElemCount {-1} { }

    ~Identifier() noexcept = default;

    bool isDummy() const noexcept {
		return mName == "@@variable" || mName == "@@function";
	}

    const std::string& getName() const noexcept {
        return mName;
    }

    void setType(Type type) noexcept {
        mType = type;
    }

    Type getType() const noexcept {
        return mType;
    }
    
    // sets number of elements in an array (used only for array types)
    void setArrayCount(int count) noexcept {
        mElemCount = count;
    }

    int getArrayCount() const noexcept {
        return mElemCount;
    }

    bool isArray() const noexcept {
        return (mType == Type::IntArray || mType == Type::CharArray || mType == Type::DoubleArray);
    }
    
    // for function identifiers
    bool isFunction() const noexcept {
        return mType == Type::Function;
    }
    
    std::shared_ptr<ASTFunc> getFunction() const noexcept {
        return mFunction;
    }
    
    void setFunction(std::shared_ptr<ASTFunc> func) noexcept {
        mFunction = func;
    }
private:
    // name of ident
    std::string mName;

    // pointer to function of ident
    std::shared_ptr<ASTFunc> mFunction;

    // type of ident
    Type mType;

    // for arrays number of elements
    int mElemCount;
};

// recursive as scope is nested
class ScopeTable {
public:
    // set new ScopeTable to SymbolTable member and add to parent ScopeTable if needed
    ScopeTable(ScopeTable * parent) noexcept;

    // delete all children scope tables
    ~ScopeTable() noexcept;
    
    // adds the requested identifier to the table
    void addIdentifier(Identifier * ident);
    
    // searches this scope for an identifier with
    // the requested name and returns nullptr if not found
    Identifier * searchInScope(const std::string& name) noexcept;
    
    // searches this scope first and if not found searches
    // through parent scopes and if not found return nullptr 
    Identifier * search(const std::string& name) noexcept;
    
    // prints the scope table to the specified stream
    void print(std::ostream& output, int depth = 0) const noexcept;

    ScopeTable * getParent() {
        return mParent;
    }
private:
    // hash table contains all the identifiers in this scope
    std::unordered_map<std::string, Identifier *> mSymbols;
    
    // vector of the child tables
    std::vector<ScopeTable *> mChildren;
    
    // Points to parent ScopeTable
    ScopeTable * mParent;
};

// store member as ScopeTable to exit and leave scopes
class SymbolTable {
public:
    // enter global scope and add dummy function and variable idents also add printf ident 
    SymbolTable() noexcept;

    // delete mCurrentScope which calls delete on all its children etc etc
    ~SymbolTable() noexcept;

    // returns true if declared in this scope
    bool isDeclaredInScope(const std::string& s) const noexcept;

    // creates the requested identifier and returns a pointer to it
    // if the identifier already exists it returns nullptr
    Identifier * createIdentifier(const std::string& s) noexcept;
    
    // returns a pointer to the identifier if found otherwise returns nullptr
    Identifier * getIdentifier(const std::string& s) const noexcept;
    
    // enters a new scope and returns a pointer to this scope table
    ScopeTable * enterScope() noexcept;
    
    // exits the current scope and moves the current scope back to the previous scope table
    void exitScope() noexcept;

    // prints the symbol table to the specified stream
    void print(std::ostream& output) const noexcept;
private:
    ScopeTable * mCurrentScope; 
};

// used to store/reference constant strings
class ConstStr {
public:
	friend class StringTable;

	ConstStr(const std::string& text) noexcept
	: mText {text} {}

    ~ConstStr() noexcept = default;
	
	const std::string& getText() const noexcept {
		return mText;
	}
private:
	std::string mText;
};

class StringTable {
public:
	StringTable() noexcept = default;

    // free all ConstStr instances allocated
	~StringTable() noexcept;
	
	// looks up the requested string in mStrings
	// if it exists returns the corresponding ConstStr
	// otherwise constructs a new ConstStr and returns that
	ConstStr * getString(const std::string& val) noexcept;
private:
	std::unordered_map<std::string, ConstStr *> mStrings;
};

#endif