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

    void visit_define();
    void visit_undef();
    void visit_include();
    void visit_error();
    void visit_warning();
    void visit_ifdef();
    void visit_ifndef();
    void visit_else();
    void visit_endif();
    void visit_pragma();
    void visit_line();
    void visit_statements();

    void replace_current_line();
} //FORGE_GOD

#endif //FORGE_FORGE_H
