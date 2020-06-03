
//
// Created by fstqwq on 2020/6/1.
//
#include <string>
#include <map>
#include <ctime>
#include "util.h"
#include <sys/stat.h>
#include <cassert>

#ifndef WIN32
#include <unistd.h>
#endif
#ifdef WIN32
#define stat _stat
#endif
namespace forge_god {

    const string& get_current_time() {
        static auto current_time = time(nullptr);
        static auto current_time_struct = localtime(&current_time);
        static auto ret_val = (string) asctime(current_time_struct);
        /* Mon Jun 01 18:24:12 2020 */
        return ret_val;
    }

    string get_date() {
        return get_current_time().substr(4, 7) + get_current_time().substr(20, 4);
    }

    string get_time() {
        return get_current_time().substr(11, 8);
    }

    string get_file_modified_time(const string& s) {
        struct stat attr{};
        if (stat(s.c_str(), &attr) == 0) {
            return (string) asctime(localtime(&attr.st_mtime));
        }
        error(s + ": No such file or directory");
        return nullptr;
    }

    token_sequence stringify(string s) {
        return {"\"" + s + "\""};
    }

    bool is_identifier(const token& s) {
        bool firstLetter = false;
        for (const auto &i : s) {
            switch (i) {
                case 'A'...'Z':
                case 'a'...'z':
                case '_': case '$':
                    firstLetter = true;
                    break;
                case '0' ... '9':
                    if (!firstLetter) return false;
                    break;
                default:
                    return false;
            }
        }
        return true;
    }
    bool is_string(const token& s) {
        return s.size() > 1 && s.front() == '"' && s.back() == '"';
    }

    int parse_number(const token &s) {
        if (!s.size()) error("not a number");
        int ret = 0;
        for (size_t j = 0; j < s.size(); j++) {
            switch (s[j]) {
                case '0'...'9':
                    ret = ret * 10 + s[j] - '0';
                    break;
                default:
                    error("not a number");
            }
        }
        return ret;
    }

    bool is_sep(char c) {
        switch (c) {
            case 'A'...'Z':
            case 'a'...'z':
            case '0'...'9':
            case '_': case '$':
            case ' ': case '\r': case '\n': case '\t':
                return false;
        }
        return true;
    }

    bool start_with_hash(const token_sequence &line) {
        return line.size() >= 1 && line[line[0] == SPACE && line.size() >= 2] == HASH;
    }

    token get_directive(const token_sequence &line) {
        // should check 'start_with_hash' before call
        size_t i = 1 + (line[0] == SPACE);
        if (i < line.size() && line[i] == SPACE) i++;
        if (i < line.size()) {
            for (auto &item : directive_list) {
                if (line[i] == item) {
                    return item;
                }
            }
        }
        return EMPTY;
    }

    token get_string(const token_sequence &line) {
        size_t i = 0, n = line.size();
        token ret = "\"";
        if (i < n && line[i] == SPACE) i++;
        if (n > 0 && line[n - 1] == SPACE) n--;
        for ( ; i < n; i++) {
            for (const auto &j : line[i]) {
                if (j == '"') ret += "\\\"";
                else if (j == '\\') ret += "\\\\";
                else ret += j;
            }
        }
        return ret;
    }

    string file_info::getline_std_string() {
        string ret;
        char ch = static_cast<char>(fgetc(file));
        if (ch == EOF) {
            error("unexpected eof");
        }
        while (ch != '\n' && ch != EOF) {
            ret += ch;
            ch = static_cast<char>(fgetc(file));
        }
        while (!ret.empty() && ret.back() == '\r') {
            ret.pop_back();
        }
        lines.push_back(ret);
        return ret;
    }

    string file_info::get_semantic_line() {
        assert(file != nullptr);
        string ret;
        bool next_line_expected;
        do {
            string s = getline_std_string();
            if (!s.empty() && s.back() == '\\') {
                s.back() = ' ';
                next_line_expected = true;
            } else {
                next_line_expected = false;
            }
            ret += s;
        } while (next_line_expected);
        return ret;
    }

    token_sequence file_info::get_next_line() {
        token_sequence_builder ret;
        string s = get_semantic_line();
        for (size_t i = 0; i < s.size(); ) {
            if (i + 1 < s.size() && s[i] == '/' && s[i + 1] == '*') {
                size_t j = i + 2;
                bool found = false;
                do {
                    while (j + 1 < s.size()) {
                        if (s[j] == '*' && s[j + 1] == '/') {
                            found = true;
                            break;
                        }
                    }
                    i = j + 1;
                    if (found) {
                        ret.add(' ');
                        i++;
                    } else {
                        if (feof(file)) {
                            error("unterminated comment");
                        }
                        s += get_semantic_line();
                    }
                } while (!found);
            } else if (i + 1 < s.size() && s[i] == '/' && s[i + 1] == '/') {
                ret.add(' ');
                break; // bye ~
            } else if (s[i] == '"' || s[i] == '\'') {
                size_t j = i + 1;
                bool found = false;
                while (j < s.size()) {
                    if (s[j] == '\\') { // escape character
                        j += 2;
                    } else {
                        if (s[j] == s[i]) {
                            found = true;
                            break;
                        }
                        j++;
                    }
                }
                if (found) {
                    string between;
                    for (size_t k = i; k <= j; k++) {
                        between += s[k];
                    }
                    ret.add(between);
                    i = j + 1;
                } else {
                    error((string)"missing terminate " + s[i]);
                }
            } else {
                ret.add(s[i++]);
            }
        }
        return ret.get_token_sequence();
    }

    void token_sequence_builder::add(const token& s) {
        if (!current_token.empty()) {
            tokens.push_back(current_token);
            current_token = EMPTY;
        }
        if (!tokens.empty() && tokens.back() == SPACE && s == SPACE) return;
        tokens.push_back(s);
    }

    void token_sequence_builder::add(char c)  {
        if (std::isspace(c)) {
            if (current_token != EMPTY) {
                if (current_token == SPACE) return;
                tokens.push_back(current_token);
            }
            current_token = SPACE;
        } else if (is_sep(c)) {
            if (current_token != EMPTY) {
                tokens.push_back(current_token);
            }
            tokens.push_back({c});
            current_token = EMPTY;
        } else {
            if (current_token == SPACE) {
                tokens.push_back(current_token);
                current_token = EMPTY;
            }
            current_token += c;
        }
    }

    void token_sequence_builder::add_all(const token_sequence& tokenSequence) {
        for (const auto& i : tokenSequence) {
            add(i);
        }
    }

    void token_sequence_builder::concatenate(const token_sequence& tokenSequence) {
        // must use as second mode
        if (!tokenSequence.size()) return;
        while (!tokens.empty() && tokens.back() == SPACE) tokens.pop_back();
        if (tokens.empty()) {
            for (auto &i : tokenSequence) {
                add(i);
            }
        } else if(is_sep(tokens.back().back()) || is_sep(tokenSequence.front().front())) {
            error("pasting \"" + tokens.back() + "\" and \"" + tokenSequence.front() + "\" does not give a valid preprocessing token");
        } else {
            tokens.back() += tokenSequence.front();
            for (size_t i = 1; i < tokenSequence.size(); i++) {
                add(tokenSequence[i]);
            }
        }
    }

    token_sequence token_sequence_builder::get_token_sequence() {
        if (!current_token.empty()) {
            tokens.push_back(current_token);
        }
        return tokens;
    }




#ifdef WINDOWS

void cprintf(FILE* stream, unsigned short color, char *s) {
    WORD colorOld;
    HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(handle, &csbi);
    colorOld = csbi.wAttributes;
    SetConsoleTextAttribute(handle, color);
    fprintf(stream, "%s", s);
    SetConsoleTextAttribute(handle, colorOld);
}

void output_colored_warning() {
    cprintf(stderr, 13, (char *) "warning: ");
}

void output_colored_error() {
    cprintf(stderr, 12, (char *) "error: ");
}

#else
void outputColoredWarning() {
    fprintf(stderr, "\033[0;31merror:\033[0m ");
}
void outputColoredError() {
    fprintf(stderr, "\033[0;35mwarning:\033[0m ");
}
#endif
    const char help_msg[] = "Call of the forge god : A simple C-like preprocessor.\nUsage : forge input_file [-o output_file]";
    void show_help_msg() {
        fputs(help_msg, stderr);
    }
} //FORGE_GOD
