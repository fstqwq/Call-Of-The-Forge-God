//
// Created by fstqwq on 2020/6/1.
//
#include "typedefs.h"
#include "forge.h"
#include "util.h"

int main(int argc, char **argv) {
    forge_god::init();

    std::vector<std::string> input_file_names;
    std::string output_file_name = "";
    {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-o") {
                if (i + 1 == argc) forge_god::error("missing filename after ‘-o’");
                std::string name = argv[++i];
                if (name.empty() || name.front() == '-') {
                    forge_god::error("unexpected " + name + " after -o");
                }
                output_file_name = name;
            } else if (arg == "-I") {
                if (i + 1 == argc) forge_god::error("missing path after ‘-I’");
                std::string path = argv[++i];
                if (path.empty() || path.front() == '-') {
                    forge_god::error("unexpected " + path + " after -I");
                }
                for (size_t p; (p = path.find(forge_god::get_path_sep())) != std::string::npos; ) {
                    forge_god::add_include_path(path.substr(0, p));
                    path = path.substr(p + 1, -1llu);
                }
                forge_god::add_include_path(path);
            } else if (arg[0] == '-') {
                forge_god::error("unsupported " + arg);
            } else {
                input_file_names.push_back(arg);
            }
        }
    }
    if (input_file_names.empty() && output_file_name.empty()) {
        forge_god::show_help_msg();
        return 0;
    }
    if (input_file_names.empty()) {
        forge_god::error("no input file");
    }
    if (input_file_names.size() > 1) {
        forge_god::error("one input file is expected");
    }
    try {
        for (const auto &file : input_file_names) {
            forge_god::reset(file);
            forge_god::run(file);
        }
    } catch (...) {
        forge_god::error("fix me");
    }
    forge_god::output(output_file_name);
}