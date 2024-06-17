/*
defines the classes needed to perform semantic analysis i.e. class Identifier, ScopeTable, SymbolTable 
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
    void setArrayCount(size_t count) noexcept {
        mElemCount = count;
    }

    size_t getArrayCount() const noexcept {
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
    // private so only the SymbolTable can create idents
    Identifier(const std::string& name) noexcept
    : mName(name)
    , mFunction(nullptr)
    , mType(Type::Void)
    , mElemCount(-1) { }
    
    // name of ident
    std::string mName;

    // pointer to function of ident
    std::shared_ptr<ASTFunc> mFunction;

    // type of ident
    Type mType;

    // for arrays number of elements
    size_t mElemCount;
};

// recursive as scope is nested
class ScopeTable {
public:
    ScopeTable(ScopeTable * parent) noexcept;

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
    SymbolTable() noexcept;
    ~SymbolTable() noexcept;

    // returns true if declared in this scope
    bool isDeclaredInScope(const std::string& s) const noexcept;

    // creates the requested identifier and returns a pointer to it
    // if the identifier already exists it returns nullptr
    Identifier * createIdentifier(const std::string& s) noexcept;
    
    // returns a pointer to the identifier if found otherwise returns nullptr
    Identifier * getIdentifier(const std::string& s) noexcept;
    
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

	ConstStr(std::string& text)
	: mText(text) {}
	
	const std::string& getText() const noexcept {
		return mText;
	}
private:
	const std::string& mText;
};

class StringTable {
public:
	StringTable() noexcept = default;
	~StringTable() noexcept;
	
	// looks up the requested string in the string table
	// if it exists returns the corresponding ConstStr
	// otherwise constructs a new ConstStr and returns that
	ConstStr * getString(std::string& val) noexcept;
private:
	std::unordered_map<std::string, ConstStr *> mStrings;
};

#endif