Welcome to the start of my solo compiler project. I will be documenting all my progess for anyone to see down the line; it should be a great learning experience!
Why may you ask? Simply put, I am fascinated by compilers and the abstraction they provide to the normal programmer. It is a beutiful inferface that I have the urge
to get a better understanding of. I took a compiler course in college that implemented a subset of c via translating the langauge into LLVM IR. Once there we ran a bunch
of optimizations on the code and from there went to codegen to our target architecure. It was a good learning experience, however given its a college course, they hold
your hand sometimes. I want to dive into this on my own.

Let's begin!

when done, go over A Map of the Territory and update this with extra info too

Characteristics of the language: 
1. Staticcally typed: type is bound to variables at compile time.
2. Imperative: statements change a programs state and explicitly define the step-by-step intructions to do so. 
3. Down the line, include some OOP support...(OOP is evil!!!!! jk..... or am i?).
4. Multifile support

The first step in making a compiler requires us to implement the front end.

Regardless of whether you are compiling or interpreting your language, the front end is normally very similiar.

1. Lexical Analysis (Scanning): responsible for breaking the text file full of characters into tokens that are the unit of "words" for our langauge.
Aka keywords, identifiers (variable/function names), constants, and so on.

Tools sucsh as Flex are designed to let you throw a handful of regexes at them, and they give you a complete scanner back. They are typically called a compiler-compiler, 
because you automate the tedious and error-prone process of manually implementing the lexing and parsing components of your compiler.
We will do this ourselves :)

Couple ways to implement this from what I have looked at online:

1 Deterministic Finite Automaton (DFA): Each state represents a specific lexical pattern, and transitions between states occur based on the input characters.
Typically done with switch statements or Transition tables.
2. Trie: Insert each token into a trie datastructure and then from there use that the tokenize your input
3. Scan the next identifier then compare it to a list of keywords sorted alphabetically with the longest matches before the shortest matches. 
If nothing matches, it is an identifier not a keyword.

We will use a simple version of DFA with switches.
 
2. Sytactical Analysis (Parsing): once we have the tokens, we must somehow analyze the structure of the tokens and see if it fits a valid definition
of what we define as a program in this language. We must model the tokens in a way that we can catch any errors in the code. 
We must define a grammar which dictates the syntactic rules of the language i.e., how legal sentence could be formed.
This is where we use trees!

Note: We could employ the help of bison, but I feel as if it would be a true learning experience if i didnt do it without those tools.

3. Semantic Analyis (Extra form of analysis): this part is responsible for catching errors not possible to be caught by simply parsing. If we have 
int x = "sadfsf", this is valid sytactically in our language, but semantically as it doesnt make sense. We cant have a int variable assigned to a string!
This is why we will do type checking and variable binding as it requires context whereas the parsing is context free.








