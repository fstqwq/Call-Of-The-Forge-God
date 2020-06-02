//
// Created by fstqwq on 2020/6/1.
//
#include "typedefs.h"
#include "forge.h"
#include "util.h"

int main(int argc, char **argv) {
    string input_file_name = "";
    string output_file_name = "";
    {
        bool read_output = false;
        for (int i = 1; i < argc; i++) {
            string arg = argv[i];
            if (arg == "-o") {
                if (read_output) {
                    forge_god::error("unexpected double -o");
                }
                read_output = true;
            } else {
                if (read_output) {
                    if (output_file_name.empty()) {
                        output_file_name = arg;
                    } else {
                        forge_god::error("multiple output file");
                    }
                    read_output = false;
                } else {
                    if (input_file_name.empty()) {
                        input_file_name = arg;
                    } else {
                        forge_god::error("multiple input file");
                    }
                }
            }
        }
    }
    if (input_file_name.empty() && output_file_name.empty()) {
        forge_god::show_help_msg();
        return 0;
    }
    if (input_file_name.empty()) {
        forge_god::error("no input file");
    }
    forge_god::run(input_file_name);
    forge_god::output(output_file_name);
}