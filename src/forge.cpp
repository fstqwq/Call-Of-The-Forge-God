//
// Created by fstqwq on 2020/6/2.
//

#include "util.h"
#include "forge.h"
#include <set>

namespace forge_god {

    vector <file_info*> include_stack;

    void error(const string &msg) {
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
                    fputs(("from " + include_stack[i]->name + ":" +
                           std::to_string(include_stack[i]->line_number())).c_str(), stderr);
                    if (i == 0) break;
                }
            }
            fprintf(stderr, "%s", (include_stack.back()->name + ":" + std::to_string(include_stack.back()->line_number()) +
                                   ":").c_str());
        }
        outputColoredError();
        fputs(msg.c_str(), stderr);
        if (!include_stack.empty()) {
            fputs(include_stack.back()->lines.back().c_str(), stderr);
        }
        exit(1);
    }

    void warning(const string &msg) {
        if (!include_stack.empty()) {
            if (include_stack.size() > 1) {
                bool is_first = true;
                for (size_t i = include_stack.size() - 2;; i--) {
                    if (is_first) {
                        fprintf(stderr, "In file included ");
                        is_first = false;
                    } else {
                        fprintf(stderr, "                 ");
                    }
                    fputs(("from " + include_stack[i]->name + ":" +
                           std::to_string(include_stack[i]->lines.size())).c_str(), stderr);
                    if (i == 0) break;
                }
            }
            fprintf(stderr, "%s", (include_stack.back()->name + ":" + std::to_string(include_stack.back()->lines.size()) +
                                   ":").c_str());
        }
        outputColoredWarning();
        fputs(msg.c_str(), stderr);
        if (!include_stack.empty()) {
            fputs(include_stack.back()->lines.back().c_str(), stderr);
        }
    }

    vector <token_sequence> output_lines;
    define_symbol_table symbol_table("symbol table");
    vector <char> if_stack; // vector <bool> is shit
    int if_false_count = 0;
    std::unordered_set <string> pragma_once;

    void set_file_define(const file_info& file) {
        symbol_table.remove("__FILE__");
        symbol_table.remove("__TIMESTAMP__");
        symbol_table.add_object("__FILE__", stringify(file.name));
        symbol_table.add_object("__TIMESTAMP__", stringify(file.time_stamp));
    }

    void enter_file(file_info& file) {
        include_stack.push_back(&file);
        set_file_define(file);
    }

    void exit_file() {
        include_stack.pop_back();
        if (!include_stack.empty()) {
            set_file_define(*include_stack.back());
        }
    }

    void run(const string &name) {
        if (pragma_once.count(name)) {
            return;
        }
        file_info current_file(name);
        enter_file(current_file);

        while (!current_file.eof()) {
            token_sequence line = current_file.get_next_line();
            if (start_with_hash(line)) {
                token directive = get_directive(line);
                if (directive == Define) {
                    visit_define(line);
                } else if (directive == Undef) {
                    visit_undef(line);
                } else if (directive == Include) {
                    visit_include(line);
                } else if (directive == Error) {
                    visit_error(line);
                } else if (directive == Warning) {
                    visit_warning(line);
                } else if (directive == Ifdef) {
                    visit_ifdef(line);
                } else if (directive == Ifndef) {
                    visit_ifndef(line);
                } else if (directive == Else) {
                    visit_else(line);
                } else if (directive == Endif) {
                    visit_endif(line);
                } else if (directive == Pragma) {
                    visit_pragma(line);
                } else if (directive == Line) {
                    visit_line(line);
                }
            } else {
                visit_normal_line(line);
            }
        }
        exit_file();
    }



    void output(const string &output_file_name) {
        FILE *output = output_file_name == "" ? stdout : fopen(output_file_name.c_str(), "w");
        if (output == nullptr) {
            throw std::runtime_error("Failed to open output file " + output_file_name);
        }
        for (auto &i : output_lines) {
            for (auto &j : i) {
                fprintf(output, "%s", j.c_str());
            }
            fputs("", output);
        }
    }

    void visit_define(const token_sequence &line) {
        size_t i = 0, n = line.size();
        while (i < n && line[i] != Define) i++;
        if (i < n && line[i] == SPACE) i++;
        if (i == n) error("no macro name given in #define directive");
        token marco = line[i++];
        if (!is_identifier(marco)) error("macro names must be identifiers");
        define_info info;
        if (i < n && line[i] == LEFT_BRACKET) {
            token last_token = EMPTY;
            for (i++; i < n; i++) {
                if (line[i] == SPACE) {
                    continue;
                }
                else if (line[i] == COMMA) {
                    if (last_token == EMPTY || last_token == COMMA) error("parameter name missing");
                    last_token = COMMA;
                } else if (line[i] == RIGHT_BRACKET) {
                    if (last_token == COMMA) error("parameter name missing");
                    last_token = RIGHT_BRACKET;
                    break;
                } else if (is_identifier(line[i])) {
                    if (last_token != EMPTY && last_token != COMMA) error("macro parameters must be comma-separated");
                    info.params.push_back(line[i]);
                    last_token = line[i];
                } else {
                    error("unexpected " + line[i]);
                }
            }
            if (i == n) error("missing ')' in macro parameter list");
            i++;
        }
        for ( ; i < n; i++) {
            info.content.push_back(line[i]);
        }
        symbol_table.add(marco, info);
    }

    void visit_undef(const token_sequence &line) {
        size_t i = 0, n = line.size();
        while (i < n && line[i] != Define) i++;
        if (i < n && line[i] == SPACE) i++;
        if (i == n) error("no macro name given in #define directive");
        token marco = line[i++];
        if (!is_identifier(marco)) error("macro names must be identifiers");
        symbol_table.remove(marco);
        if (i != n) warning("extra tokens at end of #undef directive");
    }

    void visit_include(const token_sequence &line) {

    }

    void visit_error(const token_sequence &line) {
        error(include_stack.back()->lines.back());
    }

    void visit_warning(const token_sequence &line) {
        warning(include_stack.back()->lines.back());
    }

    void visit_ifdef(const token_sequence &line) {

    }

    void visit_ifndef(const token_sequence &line) {

    }

    void visit_else(const token_sequence &line) {

    }

    void visit_endif(const token_sequence &line) {

    }

    void visit_pragma(const token_sequence &line) {

    }

    void visit_line(const token_sequence &line) {

    }

    void visit_normal_line(const token_sequence &line) {

    }
}