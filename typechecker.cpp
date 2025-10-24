#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "ast.hpp"
#include "json.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.astj>" << std::endl;
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    nlohmann::json jsonAst;
    try {
        // Parse the JSON file using the json.hpp library
        inputFile >> jsonAst;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
         std::cerr << "Error reading file: " << e.what() << std::endl;
        return 1;
    }

    try {
        // Convert the nlohmann::json object to our internal AST structure
        std::unique_ptr<Program> programAst = buildProgram(jsonAst);

        // Perform the type checking by calling the check method on the root Program node
        programAst->check();

        // If no exception was thrown, the program is valid
        std::cout << "valid" << std::endl;

    } catch (const TypeError& e) {
        // Catch specific type errors from our checker
        std::cout << "invalid: " << e.what() << std::endl;
        return 0; // Return 0 for invalid programs as per spec likely
    } catch (const std::exception& e) {
        // Catch other potential errors (e.g., during AST building)
        // You might want to refine this error message
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}