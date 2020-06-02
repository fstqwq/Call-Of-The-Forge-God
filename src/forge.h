//
// Created by fstqwq on 2020/6/2.
//

#include "typedefs.h"

#ifndef FORGE_FORGE_H
#define FORGE_FORGE_H
namespace forge_god {
    void run(const string &name);
    void output(const string &output_file_name);
    void warning(const string &msg);
    void error(const string &msg);

    void visit_define(const token_sequence& line);
    void visit_undef(const token_sequence& line);
    void visit_include(const token_sequence& line);
    void visit_error(const token_sequence& line);
    void visit_warning(const token_sequence& line);
    void visit_ifdef(const token_sequence& line);
    void visit_ifndef(const token_sequence& line);
    void visit_else(const token_sequence& line);
    void visit_endif(const token_sequence& line);
    void visit_pragma(const token_sequence& line);
    void visit_line(const token_sequence& line);
    void visit_normal_line(const token_sequence& line);
} //FORGE_GOD

#endif //FORGE_FORGE_H
