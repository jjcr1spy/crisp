#include "include/scanner.h"
#include <iostream>
#include <unistd.h> 

int x = 5;

int main(int argc, char * argv[]) {
    if (argc != 2) {
        std::cout << "Command line requires 1 argument: file to be compiled.\n";
        return 1;
    }

    if (access(argv[1], F_OK | R_OK) == -1) {
        std::cout << "Input filename is either non-existent or non-readable.\n";
        return 1;
    }

    Scanner scanner {argv[1]};
    
    scanner.scanTokens();
}
