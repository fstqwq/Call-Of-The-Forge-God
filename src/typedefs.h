//
// Created by fstqwq on 2020/6/2.
//

#ifndef FORGE_TYPEDEFS_H
#define FORGE_TYPEDEFS_H
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <unordered_set>
#include <cassert>
#include <functional>
namespace forge_god {
    using std::string;
    using std::vector;
    using std::pair;
    using std::make_pair;

    typedef std::string token;

    typedef std::vector<std::string> token_sequence;

    struct file_info;

    class token_sequence_builder;

    struct define_info;

    struct define_symbol_table;

    class output_buffer;
} //FORGE_GOD
#endif //FORGE_TYPEDEFS_H
