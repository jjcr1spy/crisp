# Welcome to Crisp

This marks the beginning of my solo compiler project. I will be documenting all my progress for anyone interested in following along. This project is a significant learning experience for me as I am fascinated by compilers and the abstraction they provide to programmers. The programming language (PL) will be a subset of the C programming language, with aspirations to develop a fully functioning front-end and middle-end compiler for the entire C programming language. Given an input .crisp file, the compiler will generate a LLVM bitcode file .bc, which can then be compiled into native assembly for some target architecture using LLVM's backend compiler llc, which translates LLVM bitcode to a native code assembly file. From there, using gcc or any other compiler toolchain you can assemble the assembly file into an executable.

Hooking up the compiler to LLVM API:
- Clone repo and cd into llvm-project. 
- Install LLVM's in house linker lld.
- Run command 'cmake -S llvm -B build -G "Unix Makefiles" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DLLVM_ENABLE_PROJECTS="lld" -DLLVM_USE_LINKER=lld -DCMAKE_BUILD_TYPE=Release '
- cd into build and run command 'make' followed by 'make install' after.
- Edit Makefile to account for LLVM's .o and .h files are when compiling and linking.

## Front End Components

### 1. Lexical Analysis (Scanning)

Responsible for breaking the text file full of characters into tokens that are the unit of "words" for our language. E.g. keywords, identifiers (variable/function names), constants, and so on.

Tools such as Flex are designed to let you throw a handful of regexes at them, and they give you a complete scanner back. 

We will create our own scanner as it is a good learning experience.

After reading online I have decided on this implementation method:

- **Deterministic Finite Automaton (DFA):** Each state represents a specific lexical pattern, and transitions between states occur based on the input characters.

We will use a simple version of DFA with switches. The switch statement will be used to transition from a set of considered tokens to now a narrower range of considered tokens. If nothing matches past the state transition, we simply create an unknown token to identify we are in an invalid state. If we never enter a switch statement and reach the default case that means we have not matched a token with our current input sequence. We must check if it is a constant or an identifier at this point. When checking if an input char sequence is an identifier, we must account for keywords that fit the lexical definition of an identifier too. Words like char/while/if etc. are unique tokens rather than an actual identifier but fit the regex of an identifier. This can be handled by indexing into a map of keywords to determine if it is a keyword or identifier.

### 2. Syntactical Analysis (Parsing)

Once we have the tokens, we must somehow analyze the structure of the tokens and see if it fits a valid definition of what we define as a program in this language. We must model the tokens in a way that we can catch any errors in the code. We must define a grammar which dictates the syntactic rules of the language i.e., how legal sentences could be formed. This is where we use trees.

Tools such as Bison take a formal grammar specification and generate C code for a parser that recognizes the structure of the input based on that grammar.

We will create our own parser as it is a good learning experience.

Languages have recursive structure:

For example, an EXPR can be:
- if EXPR then EXPR else EXPR endif
- while EXPR loop EXPR endwhile

Context-free grammars are a natural notation for this recursive structure.
We will model our program using an Abstract Syntax Tree (AST) and a top-down parser implementation i.e. recursive descent parsing. We must be careful to eliminate things such as ambiguous grammar, left-recursion, and indirect left-recursion. We do this through accounting for associativity and precedence with operators. We will also use right-recursion to account for indirect/left-recursion.
We also want to eliminate backtracking, so we will follow LL(1) where we need to only look ahead 1 token to determine our next production. Recursive-descent parsing implicitly walks a parse tree that is also implicitly shown by sequence of derivation steps (preorder). We add code into parse-routines to build the corresponding AST nodes, since the shape of the parse tree matches that of their call (graph) tree.

### 3. Semantic Analysis

The last "front end" phase of a compiler used to catch all remaining errors i.e. type/scope rule checking. This part is responsible for catching errors not possible to be caught by simply parsing and generating a tree representation of our code. If we have:

```
int x = "sadfsf"
```

This matches our context-free grammar format, but semantically it doesn't make any sense. This is where semantic analysis comes into play. Semantic analysis (aka, non-context-free syntactic analysis) ensures that the program satisfies a set of rules regarding the usage of programming constructs (variables, objects, expressions, statements). 

## Middle End Components

### 1. Lower Intermediate Representation (LIR)

At this point, we have identified valid programs that fit our definition of regular expressions and context-free grammars. We also used semantic analysis to catch static semantic and scoping rule errors that CFG cannot catch. 

We want to represent our program in a machine independant way to perform analyses/transformations. LIR allows us to represent our program in a source/target independent way. We want to easily translate from our high-level IR (AST) to lower-level IR and from the lower-level IR translate to assembly for our target architecture. 

3 common forms of LIR:
- **Three-address code:** 
Makes compiler analysis/optimization a lot easier.
- **Tree representation:**
Was popular for CISC architectures. Easier to generate complex machine code, e.g., MAC (multiply and accumulate) operations.
- **Stack machine:**
Like Java bytecode. Easier to generate from AST (simple because one operand).

We will use LLVM LIR for many reasons:
- Easy to produce, understand, and define.
- Language and target independent for middle-end optimizations.
- One IR for analysis and optimization.
- Supports both high and low-level optimization.
- Optimize as much and as early as possible.

### 2. IR Lowering (Syntax-directed Translation)

We want to convert our HIR (AST) to LLVM LIR. 

We have nested language constructs e.g. while nested within an if statement. 
We need an algorithmic way to translate.
- Different strategy for each HIR construct.
- HIR construct maps to a sequence of LIR instructions.

Solution (Syntax-directed translation)
- Start from the HIR (AST) representation.
- Define translation for each node in LIR.
- Recursively translate nodes by walking the AST.

LLVM IR is machine independent, so we don't need to worry about number of registers.
Instead, there is an unlimited set of virtual registers labelled (%0, %1, %2, %3) that 
we read and write from. The backend of LLVM will be responsible for mapping all these virtual
registers to physical registers in the hardware. 

LLVM IR must be in SSA form. This in simple terms means we can only assign to a virtual register once. 

- Invalid SSA form `x = x + 1` 
- Valid SSA form `x2 = x1 + 1`

What if we don’t follow SSA in our LLVM IR generation? 
- LLVM allows us to have a workaround for those who don't want to create an additional pass to translate their IR to SSA form. 
- LLVM bitcode supports a stack frame via the alloca, load, and store instructions. This has a downside though as this creates lots of redundant memory operations i.e. stores followed immediately by loads. 
- This is the same approach taken by Clang when it first generates LLVM IR. Clang then later converts the IR to SSA form using the mem2reg pass offered by LLVM API, which implements the canonical Cytron algorithm to generate SSA form and cleans up the redundant operations. 

An alternative solution: 
- While converting to LLVM IR, maintain SSA form on the fly using an algorithm in a published paper. Refer to the file in the GitHub repo titled SSA_paper.pdf

We will be using alloca, load, and store instructions initially and leverage mem2reg pass. Down the line we will integrate to the alternative solution. This allows us to not deal with PHI nodes or anything related to matter. We can just use pointers via the stack and load and store from said pointers so SSA form still holds.

I almost forgot to mention why even use SSA????? The short story is optimization algorithms love SSA form due to the way it simplifies data flow and control flow analysis.

### 3. TODO