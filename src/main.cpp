#include <iostream>
#include <unistd.h>  
#include "../scan/scan.h"
#include "../parse/parse.h"
#include "../parse/symbols.h"
#include "../error/parseExcept.h"
#include "../emitIR/emitter.h"

int main(int argc, char * argv[]) {
    if (argc != 2) {
        std::cout << "crisp: error: Command line requires 1 argument to start the compilation process\n";
        return 1;
    }

    if (access(argv[1], F_OK | R_OK) == -1) {
        std::cout << "crisp: error: Input filename is either non-existent or non-readable\n";
        return 1;
    }

    /*
    
    input format: ./crisp [options] <examplefile.crisp>

    options:

    -a: emit AST to stdout. no LLVM IR will be generated unless -b | -c specified 

    -b: emit LLVM bitcode to stdout. 

    -o: specify target output file, the default output file is the input file stripped of .crisp

    -c: compile input file and produce exe. if -o not specified target output file will be input file minus the .crisp extension.

    -O: enables optimization passes

    -h: prints usage instructions

    */ 

    try {
        // scan input file into tokens
        Scanner scanner {argv[1]};
        scanner.scanTokens();

        // designate stdout and stderr stream
        std::ostream * astStream = &std::cout;
        std::ostream * errStream = &std::cerr;

        // init SymbolTable and StringTable 
        SymbolTable symTable {};
        StringTable strTable {};

        // parse tokens into AST  
        // AST can be printed to stdout if specified and no parsing errors
        Parser parser {scanner, symTable, strTable, argv[1], errStream, astStream};

        // parsing errors?
        if (!parser.isValid()) {
            std::cerr << parser.getNumErrors() << " Error(s)" << std::endl;
			return 1;
        }

        // LLVM IR generation
        Emitter emit {parser};
        

        // verify no problems with IR gen

        // convert IR to SSA form
        // TODO

        // continue optimizations 
        // TODO

        // continue w backend shit 
        // TODO
    } catch (ParseExcept& e) {
		std::cerr << "crisp: error: Critical error. Compilation halted." << std::endl;
		return 1;
	}

    return 0;
}
