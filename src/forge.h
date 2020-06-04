//
// Created by fstqwq on 2020/6/2.
//

#include "typedefs.h"

#ifndef FORGE_FORGE_H
#define FORGE_FORGE_H
namespace forge_god {
    void reset();

    void run(const string &name);

    void output(const string &output_file_name);

    void warning(const string &msg);

    void error(const string &msg);
} //FORGE_GOD

#endif //FORGE_FORGE_H
