//
// Created by fstqwq on 2020/6/2.
//

#include "util.h"
#include "forge.h"

namespace forge_god {

    output_buffer output_lines;
    std::unordered_map<token, token_sequence> predefined_marco({
       { "__DATE__", stringify(get_date())},
       { "__TIME__", stringify(get_time())},
       { "__STDC__", {"1"}},
       { "__STDC_VERSION__", {"201112L"}},
       {"__cplusplus", {"201103L"}},
#ifdef WINDOWS
       { "WIN32", {}},
#else
       { "unix", {}},
       { "__unix__", {}},
#endif
       {"and", {"&&"}},
       {"and_eq", {"&="}},
       {"bitand", {"&"}},
       {"bitor", {"|"}},
       {"compl", {"~"}},
       {"not", {"!"}},
       {"not_eq", {"!="}},
       {"or", {"||"}},
       {"or_eq", {"|="}},
       {"xor", {"^"}},
       {"xor_eq", {"^="}},
    });

    vector <file_info*> file_stack;
    vector <vector<pair<token, int>>*> if_stack;
    vector <int> if_false_count;
    define_symbol_table symbol_table;
    std::unordered_set <string> pragma_once;
    std::unordered_set <token> disabled_marco;
    token_sequence current_line;
    size_t current_line_counter = 0;

    void dump_stack() {
        if (!file_stack.empty()) {
            if (file_stack.size() > 1) {
                bool is_first = true;
                for (size_t i = file_stack.size() - 2; ; i--) {
                    if (is_first) {
                        fprintf(stderr, "In file included ");
                        is_first = false;
                    } else {
                        fprintf(stderr, "                 ");
                    }
                    fprintf(stderr, "from %s:%lld:\n", file_stack[i]->name.c_str(), file_stack[i]->get_line_number());
                    if (i == 0) break;
                }
            }
            fprintf(stderr, "%s:%lld: ", file_stack.back()->name.c_str(), file_stack.back()->get_line_number());
        }
    }

    void dump_last_line() {
        if (!file_stack.empty()) {
            fprintf(stderr, ">\t%s\n", file_stack.back()->lines.back().c_str());
        }
    }

    void error(const string &msg) {
        dump_stack();
        output_colored_error();
        fprintf(stderr, "%s\n", msg.c_str());
        dump_last_line();
        exit(1);
    }

    void warning(const string &msg) {
        dump_stack();
        output_colored_warning();
        fprintf(stderr, "%s\n", msg.c_str());
        dump_last_line();
    }

    void set_file_define(const file_info& file) {
        symbol_table.remove("__FILE__");
        symbol_table.remove("__TIMESTAMP__");
        symbol_table.add_object("__FILE__", stringify(file.name));
        symbol_table.add_object("__TIMESTAMP__", stringify(file.time_stamp));
    }

    void enter_file(file_info& file, vector< pair<token, int> >& ifs) {
        file_stack.push_back(&file);
        if_stack.push_back(&ifs);
        if_false_count.push_back(0);
        set_file_define(file);
    }

    void exit_file() {
        if (!if_stack.back()->empty()) {
            error("unterminated " + if_stack.back()->back().first);
        }
        if_stack.pop_back();
        file_stack.pop_back();
        if_false_count.pop_back();
        if (!file_stack.empty()) {
            set_file_define(*file_stack.back());
        }
    }

    token_sequence replace_line(const token_sequence& line, bool in_if) {
        token_sequence_builder tokens;
        size_t n = line.size();
        for (size_t i = 0; i < n; i++) {
            const auto & cur = line[i];
            if (is_identifier(cur) && symbol_table.contains(cur) && !disabled_marco.count(cur)) {
                disabled_marco.insert(cur);
                const auto & info = symbol_table.get(cur);
                if (info.is_object_like) {
                    // unconditionally, do replacement
                    tokens.add_all(replace_line(info.content, in_if));
                } else {
                    size_t j = i + 1;
                    if (j < n && line[j] == SPACE) j++;
                    if (j < n && line[j++] == LEFT_BRACKET) {
                        vector <token_sequence> params;
                        token_sequence_builder current_tokens;
                        bool has_comma = false;
                        for ( ; j < n && line[j] != RIGHT_BRACKET; j++) {
                            if (line[j] == COMMA) {
                                has_comma = true;
                                params.push_back(current_tokens.get_token_sequence());
                                current_tokens.clear();
                            } else {
                                if (line[j] != SPACE || !current_tokens.empty()) {
                                    current_tokens.add(line[j]);
                                }
                            }
                        }
                        if (j == n || line[j] != RIGHT_BRACKET) {
                            error("unterminated argument list invoking macro " + cur);
                        }
                        i = j;
                        if (current_tokens.empty() && !has_comma) {
                            if (info.params.size() == 0) {
                                // happy
                            } else {
                                params.push_back({});
                            }
                        } else {
                            auto token_seq = current_tokens.get_token_sequence();
                            if (!token_seq.empty() && token_seq.back() == SPACE) token_seq.pop_back();
                            params.push_back(token_seq);
                        }
                        if (info.is_variadic) {
                            if (params.size() < info.params.size()) {
                                if (params.size() == info.params.size() - 1
                                && !(current_tokens.empty() && !has_comma)) { // empty can only accept one arg
                                    params.push_back({});
                                } else {
                                    error("macro \"" + cur + "\" requires " + std::to_string(info.params.size())
                                          + " arguments, but only " + std::to_string(params.size()) + " given");
                                }
                            }
                            token_sequence_builder va_arg;
                            va_arg.add_all(params[info.params.size() - 1]);
                            for (size_t k = info.params.size(); k < params.size(); k++) {
                                va_arg.add(COMMA);
                                va_arg.add(SPACE);
                                va_arg.add_all(params[k]);
                            }
                            while (params.size() >= info.params.size()) params.pop_back();
                            params.push_back(va_arg.get_token_sequence());
                        }
                        if (params.size() != info.params.size()) {
                            error("macro \"" + cur + "\" requires " + std::to_string(info.params.size())
                            + " arguments, but only " + std::to_string(params.size()) + " given");
                        }
                        std::unordered_map <token, token_sequence*> marco_map;
                        for (size_t k = 0; k < params.size(); k++) {
                            marco_map[info.params[k]] = &params[k];
                        }
                        token_sequence_builder next;
                        bool in_hash = false, in_double_hash = false;
                        int in_va_opt = 0;
                        for (const auto &content : info.content) {
                            if (content == HASH) {
                                in_hash = true;
                            } else if (content == DOUBLE_HASH) {
                                in_double_hash = true;
                            } else {
                                if (in_va_opt) {
                                    if (content == LEFT_BRACKET) {
                                        in_va_opt++;
                                        continue;
                                    } else if (content == RIGHT_BRACKET) {
                                        if (--in_va_opt == 1) {
                                            in_va_opt = 0;
                                        }
                                        continue;
                                    }
                                }
                                if (in_va_opt && params.back().size() == 0) {
                                    continue;
                                }
                                if (is_identifier(content)) {
                                    if (in_hash) {
                                        in_hash = false;
                                        next.add(get_string(*marco_map.find(content)->second));
                                    } else if (in_double_hash) {
                                        in_double_hash = false;
                                        next.concatenate(replace_line(*marco_map.find(content)->second, in_if));
                                    } else {
                                        assert(!in_hash && !in_double_hash);
                                        if (marco_map.count(content)) {
                                            next.add_all(replace_line(*marco_map.find(content)->second, in_if));
                                        } else if (info.is_variadic && content == "__VA_OPT__") {
                                            in_va_opt = 1;
                                        } else {
                                            next.add_all(replace_line({content}, in_if));
                                        }
                                    }
                                } else {
                                    assert(!in_hash && !in_double_hash);
                                    next.add(content);
                                }
                            }
                        }
                        assert(!in_hash && !in_double_hash);
                        tokens.add_all(next.get_token_sequence());
                    } else {
                        // not matched
                        tokens.add(cur);
                    }
                }
                disabled_marco.erase(cur);
            } else if (in_if && cur == Defined) {
                size_t j = i + 1; int ret;
                if (j < n && line[j] == SPACE) j++;
                if (j < n && line[j] == LEFT_BRACKET) {
                    j++;
                    if (j < n && line[j] == SPACE) j++;
                    if (j == n || !is_identifier(line[j])) {
                        error("operator \"defined\" requires an identifier");
                    }
                    ret = symbol_table.contains(line[j++]);
                    if (j < n && line[j] == SPACE) j++;
                    if (j == n || line[j] != RIGHT_BRACKET) {
                        error("missing ')' after \"defined\"");
                    }
                } else {
                    if (!is_identifier(line[j])) {
                        error("operator \"defined\" requires an identifier");
                    }
                    ret = symbol_table.contains(line[j]);
                }
                i = j;
                tokens.add(std::to_string(ret));
            } else {
                tokens.add(cur);
            }
        }
        return tokens.get_token_sequence();
    }

    bool has_next() {
        return current_line_counter != current_line.size();
    }

    const token& get_next() {
        return current_line[current_line_counter++];
    }

    const token& peek_next() {
        return current_line_counter == current_line.size() ? EMPTY : current_line[current_line_counter];
    }

    void replace_current_line(bool in_if = false) {
        token_sequence tokens;
        while (has_next()) {
            tokens.push_back(get_next());
        }
        current_line = replace_line(tokens, in_if);
        current_line_counter = 0;
    }

    void visit_define() {
        if (if_false_count.back()) {current_line.clear(); current_line_counter = 0; return;}
        if (!has_next()) error("no macro name given in #define directive");
        token marco = get_next();
        if (!is_identifier(marco)) error("macro names must be identifiers");
        define_info info;
        if (has_next() && peek_next() == LEFT_BRACKET) {
            get_next();
            info.is_object_like = false;
            token last_token = EMPTY;
            while (has_next()) {
                const token& cur = get_next();
                if (cur == SPACE) {
                    continue;
                }
                else if (cur == COMMA) {
                    if (last_token == EMPTY || last_token == COMMA) error("parameter name missing");
                    last_token = COMMA;
                } else if (cur == RIGHT_BRACKET) {
                    if (last_token == COMMA) error("parameter name missing");
                    last_token = RIGHT_BRACKET;
                    break;
                } else if (is_identifier(cur)) {
                    if (last_token != EMPTY && last_token != COMMA) error("macro parameters must be comma-separated");
                    info.params.push_back(cur);
                    last_token = cur;
                } else if (cur == DOT) {
                    if (!is_identifier(last_token)) {
                        info.params.push_back("__VA_ARGS__");
                    }
                    info.is_variadic = true;
                    // next two are dot, or fuck it
                    if (!(peek_next() == DOT)) error("\".\" may not appear in macro parameter list");
                    get_next();
                    if (!(peek_next() == DOT)) error("\".\" may not appear in macro parameter list");
                    get_next();
                    while (peek_next() == SPACE) get_next();
                    if (peek_next() != RIGHT_BRACKET) {
                        error("missing ')' after variadic macros in macro parameter list");
                    }
                    last_token = info.params.back();
                } else {
                    error("unexpected " + cur);
                }
            }
            if (last_token != ")") error("missing ')' in macro parameter list");
        } else {
            info.is_object_like = true;
        }
        std::unordered_set <string> param_set;
        for (const auto &i : info.params) {
            param_set.insert(i);
        }
        token_sequence_builder content;
        while (has_next()) {
            const token &cur = get_next();
            if (content.empty() && cur == SPACE) continue;
            if (info.is_object_like) {
                content.add(cur);
            } else {
                if (cur == HASH) {
                    string prefix = HASH;
                    if (peek_next() == HASH) {
                        get_next();
                        prefix += HASH;
                    }
                    if (!param_set.count(peek_next())) {
                        error(prefix + " is not followed by a macro parameter");
                    }
                    content.add(prefix); content.add(get_next());
                } else {
                    content.add(cur);
                }
            }
        }
        info.content = content.get_token_sequence();
        symbol_table.add(marco, info);
        output_lines.push_back({});
    }

    void visit_undef() {
        if (if_false_count.back()) {current_line.clear(); current_line_counter = 0; return;}
        if (!has_next()) error("no macro name given in #define directive");
        token marco = get_next();
        if (!is_identifier(marco)) error("macro names must be identifiers");
        symbol_table.remove(marco);
        output_lines.push_back({});
    }

    void visit_include() {
        if (if_false_count.back()) {current_line.clear(); current_line_counter = 0; return;}
        string file_name;
        for (int retry = 0; retry < 2; retry++) {
            if (!is_string(peek_next())) {
                if (peek_next() == "<") {
                    get_next();
                    while (has_next()) {
                        if (peek_next() == ">") break;
                        else if (peek_next() == SPACE && file_name.empty());
                        else file_name += get_next();
                    }
                    if (peek_next() == ">") {
                        get_next();
                        while (!file_name.empty() && file_name.back() == ' ') file_name.pop_back();
                        if (file_name.empty()) {
                            error("empty filename in #include");
                        }
                        break;
                    } else {
                        error("missing terminating > character");
                    }
                } else {
                    replace_current_line();
                    continue;
                }
            } else {
                file_name = peek_next().substr(1, peek_next().size() - 2);
                get_next();
                break;
            }
        }
        token_sequence _current_line;
        size_t _current_line_counter;
        std::swap(current_line, _current_line);
        std::swap(current_line_counter, _current_line_counter);
        run(file_name);
        std::swap(current_line, _current_line);
        std::swap(current_line_counter, _current_line_counter);
    }

    void visit_error() {
        if (if_false_count.back()) {current_line.clear(); current_line_counter = 0; return;}
        error(file_stack.back()->lines.back());
        while (has_next()) get_next();
        output_lines.push_back({});
    }

    void visit_warning() {
        if (if_false_count.back()) {current_line.clear(); current_line_counter = 0; return;}
        warning(file_stack.back()->lines.back());
        while (has_next()) get_next();
        output_lines.push_back({});
    }

    void visit_ifdef() {
        if (if_false_count.back()) {
            if_stack.back()->push_back(make_pair("#ifdef (at line " + std::to_string(
                    file_stack.back()->get_line_number()) + ")", 1));
            current_line.clear(); current_line_counter = 0; return;
        }
        if (!has_next()) {
            error("no macro name given in #ifdef directive");
        }
        if (!is_identifier(peek_next())) {
            error("macro names must be identifiers");
        }

        int val = symbol_table.contains(get_next());
        if_stack.back()->push_back(make_pair("#ifdef (at line " + std::to_string(file_stack.back()->get_line_number()) + ")", val));
        if_false_count.back() += !val;
    }

    void visit_ifndef() {
        if (if_false_count.back()) {
            if_stack.back()->push_back(make_pair("#ifndef (at line " + std::to_string(
                    file_stack.back()->get_line_number()) + ")", 1));
            current_line.clear(); current_line_counter = 0; return;
        }
        if (!has_next()) {
            error("no macro name given in #ifdef directive");
        }
        if (!is_identifier(peek_next())) {
            error("macro names must be identifiers");
        }
        int val = !symbol_table.contains(get_next());
        if_stack.back()->push_back(make_pair("#ifndef (at line " + std::to_string(file_stack.back()->get_line_number()) + ")", val));
        if_false_count.back() += !val;
    }

    void visit_if() {
        if (!has_next()) {
            error("#if with no expression");
        }
        replace_current_line(true);
        int val = expr::eval(current_line);
        if_stack.back()->push_back(make_pair(file_stack.back()->lines.back(), val));
        if_false_count.back() += !val;
        current_line.clear(); current_line_counter = 0;
    }

    void visit_else() {
        if (if_stack.empty()) {
            error("#else without #if");
        }
        if (if_stack.back()->back().second & 2) {
            error("#else after #else");
        }
        if_false_count.back() += if_stack.back()->back().second & 1 ? 1 : -1;
        if_stack.back()->back().second ^= 3;
    }

    void visit_endif() {
        if (if_stack.empty()) {
            error("#endif without #if");
        }
        if_false_count.back() -= !(if_stack.back()->back().second & 1);
        if_stack.back()->pop_back();
    }

    void visit_pragma() {
        if (if_false_count.back()) { current_line.clear(); current_line_counter = 0; return; }
        if (!has_next()) warning("ignoring pragma");
        else if (get_next() == Once) {
            pragma_once.insert(file_stack.back()->name);
        } else {
            warning("ignoring unknown pragma");
            while (has_next()) get_next();
        }
        output_lines.push_back({});
    }

    void visit_line() {
        if (if_false_count.back()) { current_line.clear(); current_line_counter = 0; return; }
        if (!has_next()) error("unexpected end of line after #line");
        replace_current_line();
        int64_t line_no = parse_positive_decimal(get_next());
        file_stack.back()->set_line_number(line_no);
        if (has_next() && peek_next() == SPACE) get_next();
        if (has_next()) {
            token name = get_next();
            if (!is_string(name)) {
                file_stack.back()->name = name.substr(1, name.size() - 2);
            } else {
                error("\"" + name + "\" is not a valid filename");
            }
        }
        output_lines.push_back({});
    }

    void visit_statements() {
        replace_current_line();
        output_lines.push_back(current_line);
    }

    void reset() {
        file_stack.clear();
        if_stack.clear();
        if_false_count.clear();
        symbol_table = define_symbol_table("symbol table", predefined_marco);
        pragma_once.clear();
        disabled_marco.clear();
    }

    void run(const string &name) {
        if (pragma_once.count(name)) {
            return;
        }
        file_info current_file(name);
        vector <pair<token, int>> current_if_stack;
        enter_file(current_file, current_if_stack);
        while (!current_file.eof()) {
            current_line = current_file.get_next_line();
            current_line_counter = 0;
            if (start_with_hash(current_line)) {
                token directive = get_directive(current_line);
                while (get_next() != directive);
                if (has_next() && peek_next() == SPACE) get_next();
                if (directive == Define) {
                    visit_define();
                } else if (directive == Undef) {
                    visit_undef();
                } else if (directive == Include) {
                    visit_include();
                } else if (directive == Error) {
                    visit_error();
                } else if (directive == Warning) {
                    visit_warning();
                } else if (directive == Ifdef) {
                    visit_ifdef();
                } else if (directive == Ifndef) {
                    visit_ifndef();
                } else if (directive == Else) {
                    visit_else();
                } else if (directive == If) {
                    visit_if();
                } else if (directive == Endif) {
                    visit_endif();
                } else if (directive == Pragma) {
                    visit_pragma();
                } else if (directive == Line) {
                    visit_line();
                } else {
                    if (!if_false_count.back()) error("unknown directive");
                    continue;
                }
                if (has_next()) warning("extra tokens at end of #" + directive + " directive");
            } else {
                visit_statements();
            }
        }
        exit_file();
    }

    void output(const string &output_file_name) {
        FILE *output = output_file_name == "" ? stdout : fopen(output_file_name.c_str(), "w");
        if (output == nullptr) {
            throw std::runtime_error("Failed to open output file " + output_file_name);
        }
        output_lines.flush(output);
        fclose(output);
    }
}