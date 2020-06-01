//
// Created by fstqwq on 2020/6/1.
//
#include "util.h"

#define debug(s) fprintf(stderr, "%s", s)

using namespace forge_god;

vector <file_info> include_stack;

void forge_god::error(const string &msg) {
    if (!include_stack.empty()) {
        if (include_stack.size() > 1) {
            bool is_first = true;
            for (size_t i = include_stack.size() - 2; ; i--) {
                if (is_first) {
                    fprintf(stderr, "In file included ");
                    is_first = false;
                } else {
                    fprintf(stderr, "                 ");
                }
                fputs(("from " + include_stack[i].name + ":" + std::to_string(include_stack[i].lines.size())).c_str(), stderr);
                if (i == 0) break;
            }
        }
        fprintf(stderr, "%s", (include_stack.back().name + ":" + std::to_string(include_stack.back().lines.size()) +
                               ":").c_str());
    }
    outputColoredError();
    fputs(msg.c_str(), stderr);
    if (!include_stack.empty()) {
        fputs(include_stack.back().lines.back().c_str(), stderr);
    }
    exit(1);
}

void forge_god::warning(const string &msg) {
    if (!include_stack.empty()) {
        if (include_stack.size() > 1) {
            bool is_first = true;
            for (size_t i = include_stack.size() - 2; ; i--) {
                if (is_first) {
                    fprintf(stderr, "In file included ");
                    is_first = false;
                } else {
                    fprintf(stderr, "                 ");
                }
                fputs(("from " + include_stack[i].name + ":" + std::to_string(include_stack[i].lines.size())).c_str(), stderr);
                if (i == 0) break;
            }
        }
        fprintf(stderr, "%s", (include_stack.back().name + ":" + std::to_string(include_stack.back().lines.size()) +
                               ":").c_str());
    }
    outputColoredWarning();
    fputs(msg.c_str(), stderr);
    if (!include_stack.empty()) {
        fputs(include_stack.back().lines.back().c_str(), stderr);
    }
}

vector <file_line> output_lines;
define_symbol_table symbol_table("symbol table");

void run(string name) {
    warning("not implemented");
}

int main(int argc, char **argv) {
    string input_file_name = "";
    string output_file_name = "";
    {
        bool read_output = false;
        for (int i = 1; i < argc; i++) {
            string arg = argv[i];
            if (arg == "-o") {
                if (read_output) {
                    error("unexpected double -o");
                }
                read_output = true;
            } else {
                if (read_output) {
                    if (output_file_name.empty()) {
                        output_file_name = arg;
                    } else {
                        error("multiple output file");
                    }
                    read_output = false;
                } else {
                    if (input_file_name.empty()) {
                        input_file_name = arg;
                    } else {
                        error("multiple input file");
                    }
                }
            }
        }
        if (input_file_name.empty()) {
            error("no input file");
        }
    }
    run(input_file_name);
    {
        FILE *output = output_file_name == "" ? stdout : fopen(output_file_name.c_str(), "w");
        if (output == nullptr) {
            throw std::runtime_error("Failed to open output file " + output_file_name);
        }
        for (auto &i : output_lines) {
            for (auto &j : i.tokens) {
                fprintf(output, "%s", j.c_str());
            }
            fputs("", output);
        }
    }
}