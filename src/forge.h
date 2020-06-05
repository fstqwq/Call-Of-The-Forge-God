//
// Created by fstqwq on 2020/6/2.
//

#include "typedefs.h"
#include "util.h"

#ifndef FORGE_FORGE_H
#define FORGE_FORGE_H
namespace forge_god {
    static const string predefined          = "__predefined.cpp";
    static const string include_dir_config  = "include_dir.cfg";

    void init();

    void add_include_path(string path);

    void reset(const string & name);

    void run(string name);

    void output(const string &output_file_name);

    void warning(const string &msg);

    void error(const string &msg);
} //FORGE_GOD

#endif //FORGE_FORGE_H
