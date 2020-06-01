//
// Created by fstqwq on 2020/6/1.
//
#ifndef CALL_OF_THE_FORGE_GOD_UTIL_H
#define CALL_OF_THE_FORGE_GOD_UTIL_H
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <fstream>
#include <cstdio>
namespace forge_god {

    using std::string;
    using std::vector;
    using std::map;
    using std::pair;
    using std::make_pair;
    typedef string token;
    typedef vector<string> token_sequence;

    void warning(const string &msg);

    void error(const string &msg);

    void outputColoredWarning();

    void outputColoredError();

    string get_file_modified_time(const string& s);

    string get_line_std_string(FILE* file);

    token_sequence tokenize(string s);

    token_sequence stringify(string s);

    bool isIdentifier();

    string get_date();

    string get_time();

    struct file_line {
        size_t lineNumber;
        token_sequence tokens;
        file_line (size_t _lineNumber, token_sequence _tokens)
        : lineNumber(_lineNumber), tokens(std::move(_tokens)) {}
    };

    struct file_info {
        string name, timeStamp;
        FILE* file;
        vector <string> lines;

        explicit file_info(const string& file_name) {
            name = file_name;
            timeStamp = get_file_modified_time(file_name);
            if ((file = fopen(file_name.c_str(), "r")) == nullptr) {
                error(file_name + ": Failed to open file");
            }
        }

        ~file_info() {
            if (file != nullptr) {
                fclose(file);
            }
        }


        file_line get_next_line();
    private:
        string get_semantic_line();
        string getline_std_string();
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
        map<string, define_info> table;

        define_symbol_table() = default;

        explicit define_symbol_table(string _name) : name(std::move(_name)) {
            add("__DATE__", define_info({}, stringify(get_date())));
            add("__TIME__", define_info({}, stringify(get_time())));
#ifdef WIN32
            add("WIN32", define_info({}, {}));
#else
            add("__unix__", define_info({}, {}));
#endif
        }

        void add_object(const string &s, const token_sequence& tokenSequence) {
            add(s, define_info({}, tokenSequence));
        }

        void add_function(const string &s, const token_sequence& params, const token_sequence& content) {
            add(s, define_info(params, content));
        }

        void add(const string &s, const define_info &defineInfo) {
            auto iter = table.find(s);
            if (iter != table.end() && (iter->second.params != defineInfo.params || iter->second.content != defineInfo.content)) {
                warning("redefining marco " + s + " that is not effectively the same with previous define");
            }
            table.insert(std::make_pair(s, defineInfo));
        }

        bool contains(const string &s) const {
            return table.count(s) == 1;
        }

        define_info get(const string &s) {
            auto x = table.find(s);
            if (x == table.end()) {
                error("failed to find element" + s + " in " + name);
            }
            return x->second;
        }

        void remove(const string &s) {
            auto x = table.find(s);
            if (x != table.end()) {
                table.erase(x);
            }
        }
    };

    class token_sequence_builder {
        token_sequence tokens;
        string current_token;
    public:
        token_sequence_builder() = default;
        void add(const token& s) {
            // "...", '...'
            if (!current_token.empty()) {
                tokens.push_back(current_token);
            }
            tokens.push_back(s);
        }
        void add(char c) {
            if (isspace(c)) {
                if (!current_token.empty()) {
                    if (current_token[0] == ' ') return;
                    tokens.push_back(current_token);
                }
                current_token = " ";
            } else if (ispunct(c)) {
                if (!current_token.empty()) {
                    tokens.push_back(current_token);
                }
                tokens.push_back({c});
                current_token = "";
            } else {
                if (!current_token.empty() && !isspace(current_token[0])) {
                    tokens.push_back(current_token);
                    current_token = "";
                }
                current_token += c;
            }
        }
        token_sequence get_token_sequence() {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
            }
            return tokens;
        }
    };
}; //FORGE_GOD
#endif //CALL_OF_THE_FORGE_GOD_UTIL_H
