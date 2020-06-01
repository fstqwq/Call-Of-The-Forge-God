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

    string get_current_time() {
        static auto current_time = time(nullptr);
        static auto current_time_struct = localtime(&current_time);
        static auto ret_val = (string) asctime(current_time_struct);
        /* Mon Jun 01 18:24:12 2020 */
        return ret_val;
    }

    string get_date() {
        return get_current_time().substr(0, 10);
    }

    string get_time() {
        return get_current_time().substr(11, 114514);
    }

    string get_file_modified_time(const string& s) {
        struct stat attr{};
        if (stat(s.c_str(), &attr) == 0) {
            return (string) asctime(localtime(&attr.st_mtime));
        }
        throw std::runtime_error(s + ": No such file or directory");
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
                case '_': // case '$': // not supported, since ispunct accept it
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
                // count the trailing \, if odd, then consider it '\\\n'
                int count = 0;
                for (size_t i = s.size() - 1; ; i--) {
                    if (s[i] == '\\') count++;
                    else break;
                    if (i == 0) break;
                }
                if (count & 1) {
                    s.pop_back();
                    next_line_expected = true;
                }
            } else {
                next_line_expected = false;
            }
            ret += s;
        } while (next_line_expected);
        return ret;
    }

    file_line file_info::get_next_line() {
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
            } else if (s[i] == '"' || s[i] == '\''){
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
                } else {
                    error((string)"missing terminate " + s[i]);
                }
            }
        }
        return {lines.size(), ret.get_token_sequence()};
    }

#ifdef WIN32
#include <windows.h>
#include <stdarg.h>
void outputColoredWarning() {
    WORD colorOld;
    HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(handle, &csbi);
    colorOld = csbi.wAttributes;
    SetConsoleTextAttribute(handle, 13);
    fprintf(stderr, "warning: ");
    SetConsoleTextAttribute(handle, colorOld);
}

void outputColoredError() {
    WORD colorOld;
    HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(handle, &csbi);
    colorOld = csbi.wAttributes;
    SetConsoleTextAttribute(handle, 12);
    fprintf(stderr, "error: ");
    SetConsoleTextAttribute(handle, colorOld);
}
#else
void outputColoredWarning() {
    fprintf(stderr, "\033[0;31merror:\033[0m ");
}
void outputColoredError() {
    fprintf(stderr, "\033[0;35mwarning:\033[0m ");
}
#endif
} //FORGE_GOD
