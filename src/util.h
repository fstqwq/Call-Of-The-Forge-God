//
// Created by fstqwq on 2020/6/1.
//
#ifndef FORGE_UTIL_H
#define FORGE_UTIL_H
#include <fstream>
#include <cstdio>
#include "typedefs.h"
#include "forge.h"
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(WIN32)
#define WINDOWS
#include <windows.h>
#include <stdarg.h>
#endif
#define debug(s) fprintf(stderr, "%s", s)

namespace forge_god {

    static const token Define  = "define";
    static const token Undef   = "undef";
    static const token Include = "include";
    static const token Error   = "error";
    static const token Warning = "warning";
    static const token Ifdef   = "ifdef";
    static const token Ifndef  = "ifndef";
    static const token Else    = "else";
    static const token Endif   = "endif";
    static const token Pragma  = "pragma";
    static const token Once    = "once";
    static const token Line    = "line";
    const token directive_list[] = {Define, Undef, Include, Error, Warning, Ifdef, Ifndef, Else, Endif, Pragma, Line};
    static const token SPACE   = " ";
    static const token EMPTY   = "";
    static const token HASH   = "#";
    static const token LEFT_BRACKET = "(";
    static const token RIGHT_BRACKET = ")";
    static const token COMMA = ",";

    void outputColoredWarning();

    void outputColoredError();

    string get_file_modified_time(const string& s);

    token_sequence stringify(string s);

    bool is_sign(char c);

    bool is_identifier(const token& s);

    bool start_with_hash(const token_sequence &line);

    token get_directive(const token_sequence& line);

    const string& get_current_time();

    string get_date();

    string get_time();

    void show_help_msg();

    struct file_info {
        string name, time_stamp;
        FILE* file;
        vector <string> lines;
        int line_offset;

        explicit file_info(const string& file_name) {
            name = file_name;
            time_stamp = get_file_modified_time(file_name);
            line_offset = 0;
            if ((file = fopen(file_name.c_str(), "r")) == nullptr) {
                error(file_name + ": Failed to open file");
            }
        }

        ~file_info() {
            if (file != nullptr) {
                fclose(file);
            }
        }
        bool eof() {
            return feof(file);
        }
        int line_number() {
            return int(lines.size()) + line_offset;
        }
        void set_line_number(int no) {
            line_offset = no - 1 - int(lines.size());
        }
        token_sequence get_next_line();
    private:
        string get_semantic_line();
        string getline_std_string();
    };

    class token_sequence_builder {
        token_sequence tokens;
        string current_token;
    public:
        token_sequence_builder() = default;
        void add(const token& s);
        void add(char c);
        token_sequence get_token_sequence();
    };

    struct define_info {
        token_sequence params;
        token_sequence content;
        define_info () = default;
        define_info (token_sequence _params, token_sequence _content)
        : params(move(_params)), content(move(_content)) {}
    };
    struct define_symbol_table {
        string name;
        std::unordered_map<token, define_info> table;

        define_symbol_table() = default;

        explicit define_symbol_table(string _name) : name(std::move(_name)) {
            add("__DATE__", define_info({}, stringify(get_date())));
            add("__TIME__", define_info({}, stringify(get_time())));
#ifdef WINDOWS
            add("WIN32", define_info({}, {}));
#else
            add("__unix__", define_info({}, {}));
#endif
        }

        void add_object(const token &s, const token_sequence& tokenSequence) {
            add(s, define_info({}, tokenSequence));
        }

        void add_function(const token &s, const token_sequence& params, const token_sequence& content) {
            add(s, define_info(params, content));
        }

        void add(const token &s, const define_info &defineInfo) {
            auto iter = table.find(s);
            if (iter != table.end() && (iter->second.params != defineInfo.params || iter->second.content != defineInfo.content)) {
                warning("redefining marco " + s + " that is not effectively the same with previous define");
            }
            table.insert(std::make_pair(s, defineInfo));
        }

        bool contains(const token &s) const {
            return table.count(s) == 1;
        }

        define_info get(const token &s) {
            auto x = table.find(s);
            if (x == table.end()) {
                error("failed to find element" + s + " in " + name);
            }
            return x->second;
        }

        void remove(const token &s) {
            auto x = table.find(s);
            if (x != table.end()) {
                table.erase(x);
            }
        }
    };

}; //FORGE_GOD
#endif //FORGE_UTIL_H
