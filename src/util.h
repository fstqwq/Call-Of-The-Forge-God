//
// Created by fstqwq on 2020/6/1.
//
#ifndef FORGE_UTIL_H
#define FORGE_UTIL_H
#include "typedefs.h"
#include "forge.h"
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(WIN32)
#define WINDOWS
#include <windows.h>
#include <stdarg.h>
#endif
#define debug(s) fprintf(stderr, "%s\n", s)
#define debugstd(s) fprintf(stderr, "%s\n", (s).c_str())

namespace forge_god {

    static const token Define  = "define";
    static const token Undef   = "undef";
    static const token Include = "include";
    static const token Error   = "error";
    static const token Warning = "warning";
    static const token If      = "if";
    static const token Ifdef   = "ifdef";
    static const token Ifndef  = "ifndef";
    static const token Else    = "else";
    static const token Endif   = "endif";
    static const token Pragma  = "pragma";
    static const token Once    = "once";
    static const token Line    = "line";
    const token directive_list[] = {Define, Undef, Include, Error, Warning, If, Ifdef, Ifndef, Else, Endif, Pragma, Line};
    static const token Defined = "defined";
    static const token SPACE   = " ";
    static const token EMPTY   = "";
    static const token HASH   = "#";
    static const token DOUBLE_HASH   = "##";
    static const token LEFT_BRACKET = "(";
    static const token RIGHT_BRACKET = ")";
    static const token COMMA = ",";
    static const token DOT = ".";

    const std::unordered_set<string> operators = {
            "!", "~",
            "+", "-", "*", "/", "%", "++", "--", "<<", ">>",
            "<", "<=", ">", ">=", "==", "!=",
            "&", "^", "|", "&&", "||",
            "=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=",
            "&=", "^=", "|=",
            ",",
            "<>", // place holder for template
    };


    void output_colored_warning();

    void output_colored_error();

    string get_file_modified_time(const string& s);

    token_sequence stringify(string s);

    bool is_sep(char c);

    bool is_identifier(const token& s);

    bool is_string(const token& s);

    bool is_number_literal(const token& s);

    int64_t parse_positive_decimal(const token& s);

    int64_t parse_integer(const token& s);
    namespace expr {
        int eval(const token_sequence &tokens);
    }
    bool start_with_hash(const token_sequence &line);

    token get_directive(const token_sequence& line);

    token get_string(const token_sequence& line);

    const string& get_current_time();

    string get_date();

    string get_time();

    void show_help_msg();

    struct file_info {
        string name, time_stamp;
        FILE* file;
        vector <string> lines;
        int64_t line_offset;
        explicit file_info(const string& file_name);

        ~file_info();

        bool eof();

        int64_t get_line_number();

        void set_line_number(int64_t no);

        token_sequence get_next_line();

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

        void add_all(const token_sequence& tokenSequence);

        void concatenate(const token_sequence& tokenSequence);

        token_sequence get_token_sequence();

        bool empty();

        void clear();
    };

    struct define_info {
        bool is_object_like, is_variadic;
        token_sequence params;
        token_sequence content;
        define_info () = default;
        define_info (bool _is_object_like, token_sequence _params, token_sequence _content)
        : is_object_like(_is_object_like), params(move(_params)), content(move(_content)) {}
    };
    struct define_symbol_table {
        const static std::unordered_set<token> disabled_names;
        string name;
        std::unordered_map<token, define_info> table;

        define_symbol_table() = default;

        explicit define_symbol_table(string _name, std::unordered_map<token, token_sequence> predefined);

        void add_object(const token &s, const token_sequence& tokenSequence);

        void add_function(const token &s, const token_sequence& params, const token_sequence& content);

        void add(const token &s, const define_info &defineInfo);

        bool contains(const token &s) const;

        define_info get(const token &s);

        void remove(const token &s);
    };

    class output_buffer {
    private:
        string buffer;
    public:
        void push_back(const token_sequence& tokens);
        void flush(FILE* file);
    };

} //FORGE_GOD
#endif //FORGE_UTIL_H
