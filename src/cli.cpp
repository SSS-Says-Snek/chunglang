#include <iostream>
#include <filesystem>

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "llvm/IR/LegacyPassManager.h"

#include "chung/file.hpp"
#include "chung/lexer.hpp"
#include "chung/parser.hpp"
#include "chung/sema.hpp"

#include "chung/stringify.hpp"
#include "chung/utils/ansi.hpp"

#include "chung/library/setup_prelude.hpp"

#define CHUNG_VER_MAJOR 0
#define CHUNG_VER_MINOR 0
#define CHUNG_VER_PATCH 1

inline std::string chung_ver_string() {
    return std::to_string(CHUNG_VER_MAJOR) + '.' + std::to_string(CHUNG_VER_MINOR) + '.' +
           std::to_string(CHUNG_VER_PATCH);
}

void run_help() {
    std::cout << "Chungussy Programming Language Compiler\n\n";
    std::cout << "Usage:\n";
    std::cout << "    chung [command] [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "    chung parse <file.chung>   Lexes and parses the file, then dumps the AST\n";
}

int run_parse(std::vector<std::string>& args) {
    std::cout << ANSI_BOLD << "Running Chungussy " << chung_ver_string() << '\n' << ANSI_RESET;
    if (args.size() != 2) {
        std::cerr << ANSI_RED << "Expected 1 argument, received " << args.size() - 1 << '\n' << ANSI_RESET;
        std::exit(1);
    }

    const std::string& file_path = args[1];
    if (!file_exists(file_path)) {
        std::cerr << ANSI_RED << "File not found: \"" << file_path << "\" cannot be located" << '\n' << ANSI_RESET;
        std::exit(1);
    }
    std::cout << "Lexing " << file_path << '\n';

    Context ctx{};
    std::string source = read_source(file_path);
    Lexer lexer{source};

    std::vector<Token> tokens;
    std::vector<LexException> lex_exceptions;
    std::tie(tokens, lex_exceptions) = lexer.lex();

    if (!lex_exceptions.empty()) {
        std::cout << ANSI_RED;
        for (auto& lex_exception : lex_exceptions) {
            std::cout << lex_exception.write({}) << '\n'; // TODO: Lazy for lex exceptions
        }
        std::cout << ANSI_RESET;
    } else {
        std::cout << ANSI_GREEN << "Successfully lexed with no exceptions!\n\n" << ANSI_RESET;
        std::cout << ANSI_CYAN << "==============================================\n" << ANSI_RESET;
        std::cout << ANSI_BOLD << "                Program Tokens                \n" << ANSI_RESET;
        std::cout << ANSI_CYAN << "==============================================\n" << ANSI_RESET;
        for (auto& token : tokens) {
            std::cout << '|' << ANSI_BOLD << stringify(token) << ANSI_RESET << "| ";
        }
        std::cout << "\n\n";
    }

    std::cout << "Parsing " << file_path << '\n';
    std::vector<std::string> source_lines = lexer.get_source_lines();
    Parser parser{tokens, source_lines, ctx};
    auto statements = parser.parse();
    auto parse_exceptions = parser.get_exceptions();

    if (!parse_exceptions.empty()) {
        for (auto& parse_exception : parse_exceptions) {
            std::cout << parse_exception.write(source_lines) << '\n';
        }
    } else {
        std::cout << ANSI_GREEN << "Successfully parsed with no exceptions!\n\n" << ANSI_RESET;
    }

    if (!statements.empty()) {
        setup_prelude(ctx);

        std::cout << ANSI_CYAN << "==============================================\n" << ANSI_RESET;
        std::cout << ANSI_BOLD << "                 Program AST                  \n" << ANSI_RESET;
        std::cout << ANSI_CYAN << "==============================================\n" << ANSI_RESET << '\n';
        std::cout << ANSI_CYAN << "Number of statements: " << statements.size() << ANSI_RESET << '\n';

        for (auto& statement : statements) {
            std::cout << statement->stringify() << '\n';

            // llvm::Value* statement_value = statement->codegen(ctx);
            // if (statement_value) {
            //     statement_value->print(llvm::outs());
            // }
        }

        if (!parse_exceptions.empty()) {
            return 1;
        }

        Sema sema{std::move(statements)};
        auto resolved_ast = sema.resolve();

        for (auto& resolved_statement : resolved_ast) {
            llvm::Value* statement_value = resolved_statement->codegen(ctx);
            if (statement_value) {
                statement_value->print(llvm::outs());
            }
        }

        std::cout << "\n\n";
        std::cout << ANSI_CYAN << "==============================================\n" << ANSI_RESET;
        std::cout << ANSI_BOLD << "      Module IR (temporary trust me bro)      \n" << ANSI_RESET;
        std::cout << ANSI_CYAN << "==============================================\n" << ANSI_RESET << std::endl;
        ctx.module->print(llvm::outs(), nullptr);

        std::cout << "\nCompiling " << file_path << '\n';

        // Compile to object file
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        std::string target_triple_str = llvm::sys::getDefaultTargetTriple();
        llvm::Triple triple{target_triple_str};

        std::string target_error;

        const auto* target = llvm::TargetRegistry::lookupTarget(triple, target_error);
        if (!target) {
            llvm::errs() << target_error;
            std::exit(1);
        }

        std::string cpu{"generic"};
        std::string features{};

        llvm::TargetOptions options;

        auto rm = std::optional<llvm::Reloc::Model>();
        auto* target_machine = target->createTargetMachine(triple, cpu, features, options, rm);

        ctx.module->setDataLayout(target_machine->createDataLayout());
        ctx.module->setTargetTriple(triple);

        // Create chungbuild directory
        std::string output_filename{"output.o"};
        std::filesystem::create_directory("chungbuild");

        std::string output_filepath{std::filesystem::path{"chungbuild"} / output_filename};
        std::error_code errcode;
        llvm::raw_fd_ostream dest{output_filepath, errcode, llvm::sys::fs::OF_None};

        if (errcode) {
            llvm::errs() << "Could not open file: " << errcode.message();
            std::exit(1);
        }

        // Compile to object file
        llvm::legacy::PassManager pass;
        auto filetype = llvm::CodeGenFileType::ObjectFile;

        if (target_machine->addPassesToEmitFile(pass, dest, nullptr, filetype)) {
            llvm::errs() << "TargetMachine can't emit a file of this type";
            std::exit(1);
        }

        pass.run(*ctx.module);
        dest.flush();

        // IDK /shrug
        // system("clang++ src/library/prelude.cpp -Iinclude -c -o chungbuild/prelude.o");
        // system((std::string{"clang++ $(llvm-config --ldflags --libs) "} + output_filepath + " chungbuild/prelude.o -o
        // chungbuild/output.out").c_str());
        system("clang++ src/library/prelude.cpp -Iinclude -c -o chungbuild/prelude.o");      // NOLINT
        system("clang++ chungbuild/output.o chungbuild/prelude.o -o chungbuild/output.out"); // NOLINT
    }

    return 0;
}

int main(const int argc, const char** argv) {
    std::vector<std::string> args;
    // Goofy ahh first argument
    for (int i = 1; i < argc; i++) {
        args.emplace_back(argv[i]);
    }

    if (args.empty()) {
        run_help();
        return 0;
    }

    std::string command = args[0];
    if (command == "help") {
        run_help();
    } else if (command == "parse") {
        return run_parse(args);
    }

    return 0;
}
