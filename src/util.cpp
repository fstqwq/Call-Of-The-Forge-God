
//
// Created by fstqwq on 2020/6/1.
//
#include <string>
#include <map>
#include <ctime>
#include "util.h"
#include <sys/stat.h>

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
        static auto ret_val = ((string) asctime(current_time_struct)).substr(0, 24);
        /* Mon Jun 01 18:24:12 2020 */
        return ret_val;
    }

    string get_date() {
        return get_current_time().substr(4, 7) + get_current_time().substr(20, 4);
    }

    string get_time() {
        return get_current_time().substr(11, 8);
    }


    bool test_file_exist(const string &s) {
        struct stat attr{};
        return stat(s.c_str(), &attr) == 0;
    }

    string get_file_modified_time(const string& s) {
        struct stat attr{};
        if (stat(s.c_str(), &attr) == 0) {
            return ((string) asctime(localtime(&attr.st_mtime))).substr(0, 24);
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
        return firstLetter;
    }
    bool is_string(const token& s) {
        return s.size() > 1 && s.front() == '"' && s.back() == '"';
    }

    bool is_number_literal(const token& s) {
        // not precise, but a quick classification
        return !s.empty() && isdigit(s.front());
    }

    int64_t parse_positive_decimal(const token &s) {
        if (!s.size()) error("not a number");
        int64_t ret = 0;
        for (size_t j = 0; j < s.size(); j++) {
            switch (s[j]) {
                case '0'...'9':
                    if (static_cast<unsigned long long int>(ret) >= -1llu / 20) {
                        warning("overflow detected");
                    }
                    ret = ret * 10 + s[j] - '0';
                    break;
                default:
                    error("\"" + s + "\" after #line is not a positive integer");
            }
        }
        return ret;
    }

    int64_t parse_integer(const token& s) {
        size_t n = s.size();
        if (s[n - 1] == 'L' || s[n - 1] == 'l') {
            n--;
            if (s[n - 1] == s[n]) {
                n--;
            }
        }
        int64_t ret = 0;
        static char err_msg[] = "user-defined literal in preprocessor expression";
        if (s.front() == '0') {
            if (n == 1) return 0;
            switch (s[1]) {
                case 'b':
                    if (n == 2) error(err_msg);
                    for (size_t i = 2; i < n; i++) {
                        switch (s[i]) {
                            case '0': case '1':
                                ret = ret * 2 + s[i] - '0';
                                break;
                            default: error(err_msg);
                        }
                    }
                    break;
                case 'x':
                    if (n == 2) error(err_msg);
                    for (size_t i = 2; i < n; i++) {
                        switch (s[i]) {
                            case '0'...'9':
                                ret = ret * 16 + s[i] - '0';
                                break;
                            case 'A'...'F':
                                ret = ret * 16 + 10 + s[i] - 'A';
                                break;
                            case 'a'...'f':
                                ret = ret * 16 + 10 + s[i] - 'a';
                                break;
                            default: error(err_msg);
                        }
                    }
                    break;
                case '0'...'7':
                    for (size_t i = 1; i < n; i++) {
                        switch (s[i]) {
                            case '0'...'7':
                                ret = ret * 8 + s[i] - '0';
                                break;
                            default: error(err_msg);
                        }
                    }
                    break;
                default: error(err_msg);
            }
        } else {
            for (size_t i = 0; i < n; i++) {
                switch (s[i]) {
                    case '0'...'9':
                        ret = ret * 10 + s[i] - '0';
                        break;
                    default: error(err_msg);
                }
            }
        }
        return ret;
    }

    namespace expr {

        /*
        expression:
                <assoc=right>   op=('+' | '-')       expression                         #unaryExpr
            |   <assoc=right>   op=('!' | '~')       expression                         #unaryExpr
            |   src1=expression op=('*' | '/' | '%') src2=expression                    #binaryExpr
            |   src1=expression op=('+' | '-')       src2=expression                    #binaryExpr
            |   src1=expression op=('<<' | '>>')     src2=expression                    #binaryExpr
            |   src1=expression op=('<' | '<=' )     src2=expression                    #binaryExpr
            |   src1=expression op=('>' | '>=' )     src2=expression                    #binaryExpr
            |   src1=expression op=('!=' | '==')     src2=expression                    #binaryExpr
            |   src1=expression op='&'               src2=expression                    #binaryExpr
            |   src1=expression op='^'               src2=expression                    #binaryExpr
            |   src1=expression op='|'               src2=expression                    #binaryExpr
            |   src1=expression op='&&'              src2=expression                    #binaryExpr
            |   src1=expression op='||'              src2=expression                    #binaryExpr
            |   src1=expression ? src2=expression :  src3=expression                    #ternaryExpr
            |   src1=expression op=','               src2=expression                    #binaryExpr
            |   constant                                                                #literal
            |   Identifier                                                              #identifier
            |   '(' expression ')'                                                      #bracketExpr
            ;
        */

        enum op { // binary and unary
            NOT, COMPL, POS, NEG,
            ADD, SUB, MUL, DIV, MOD, SHL, SHR,
            LT, LEQ, GT, GEQ, EQ, NEQ,
            AND, OR, XOR, ANDL, ORL,
            QUS,
            COM,
        };

        const std::unordered_set<op> unary_operators = {
            NOT, COMPL, POS, NEG,
        };

        const std::unordered_set<op> binary_operators = {
            ADD, SUB, MUL, DIV, MOD, SHL, SHR,
            LT, LEQ, GT, GEQ, EQ, NEQ,
            AND, OR, XOR, ANDL, ORL,
            QUS,
            COM,
        };

        const std::unordered_map<string, op> unary_operator_map = {
                {"!", NOT}, {"~", COMPL},
                {"+", POS}, {"-", NEG}
        };

        const std::unordered_map<string, op> binary_operator_map = {
                {"+", ADD}, {"-", SUB},
                {"*", MUL}, {"/", DIV}, {"%", MOD},
                {"<<", SHL}, {">>", SHR},
                {"<", LT}, {"<=", LEQ}, {">", GT}, {">=", GEQ},
                {"==", EQ}, {"!=", NEQ},
                {"&", AND},
                {"^", XOR},
                {"|", OR},
                {"&&", ANDL},
                {"||", ORL},
                {"?", QUS},
                {",", COM}
        };

        constexpr int precedence(op o) {
            switch (o) {
                case MUL: case DIV: case MOD: return 0;
                case ADD: case SUB: return 1;
                case SHL: case SHR: return 2;
                case LT: case LEQ: case GT: case GEQ: return 3;
                case EQ: case NEQ: return 4;
                case AND: return 5;
                case XOR: return 6;
                case OR: return 7;
                case ANDL: return 8;
                case ORL: return 9;
                case QUS: return 10;
                case COM: return 11;
                default: return -1;
            }
        }

        constexpr auto SHORTCUT_LIMIT = precedence(ANDL);

        string op2string(op o) {
            // rarely used, so high complexity is ok
            for (const auto & i : unary_operator_map)
                if (i.second == o) return i.first;
            for (const auto & i : binary_operator_map)
                if (i.second == o) return i.first;
            return "";
        }

        token_sequence current_line;
        size_t current_line_counter = 0;
        bool has_next() {
            return current_line_counter != current_line.size();
        }
        const token& get_next() {
            return current_line[current_line_counter++];
        }

        void collapse(vector <int64_t>& values, vector <op>& ops, int delimiter_precedence, string op_str = "");

        void read_until(const string& expected_ending);

        int64_t do_eval(string expected_ending) {
            vector <int64_t> values;
            vector <op> ops;
            bool safe_exit = false;
            while (has_next()) {
                const auto &cur = get_next();
                if (is_identifier(cur)) {
                    error("unexpected " + cur); // impossible
                } else if (cur == RIGHT_BRACKET) {
                    if (expected_ending != RIGHT_BRACKET) {
                        error("missing '(' in expression");
                    }
                    safe_exit = true;
                    break;
                } else if (cur == ":") {
                    if (expected_ending != ":") {
                        error("missing '?' in expression");
                    }
                    safe_exit = true;
                    break;
                } else if (cur == LEFT_BRACKET || is_number_literal(cur)) {
                    int64_t value = cur == LEFT_BRACKET ? do_eval(RIGHT_BRACKET) : parse_positive_decimal(cur);
                    while (!ops.empty() && unary_operators.count(ops.back())) {
                        switch (ops.back()) {
                            case NOT:   value = !value; break;
                            case COMPL: value = ~value; break;
                            case NEG:   value = -value; break;
                            default:                    break;
                        }
                        ops.pop_back();
                    }
                    values.push_back(value);
                    if (values.size() != ops.size() + 1) {
                    /*    for (auto &s : current_line) {
                            debugstd(s);
                        }*/
                        error("missing binary operator before token " + cur); // not possible ?
                    }
                } else if (cur == "+" || cur == "-") {
                    // possible for unary and binary
                    if (ops.size() >= values.size()) { // parse as unary
                        ops.push_back(unary_operator_map.at(cur));
                    } else if (ops.size() + 1 == values.size()) { // balanced, parse as binary
                        op cur_op = binary_operator_map.at(cur);
                        collapse(values, ops, precedence(cur_op), cur);
                        ops.push_back(cur_op);
                    } else {
                        error("unexpected " + cur); // impossible
                    }
                } else if (unary_operator_map.count(cur)) {
                    if (ops.size() + 1 < values.size()) {
                        error("missing binary operator before token " + cur);
                    }
                    ops.push_back(unary_operator_map.at(cur));
                } else if (binary_operator_map.count(cur)) {
                    op cur_op = binary_operator_map.at(cur);
                    if (ops.size() + 1 == values.size()) {
                        collapse(values, ops, precedence(cur_op), cur);
                        if (precedence(cur_op) >= SHORTCUT_LIMIT) {
                            if (values.size() != 1) {
                                error("unexpected value size " + std::to_string(values.size())); // not possible ?
                            }
                            switch (cur_op) {
                                case ANDL:
                                    if (!values.back()) {
                                        read_until(expected_ending);
                                        return 0;
                                    }
                                    break;
                                case ORL:
                                    if (values.back()) {
                                        read_until(expected_ending);
                                        return 1;
                                    }
                                    break;
                                case QUS:
                                    if (values.back()) {
                                        int64_t ret = do_eval(":");
                                        read_until(expected_ending);
                                        return ret;
                                    }
                                    read_until(":");
                                    break;
                                case COM:
                                    break;
                                default:
                                    error("unexpected error [unknown shortcut operator]");
                            }
                            values.clear();
                            ops.clear();
                        } else {
                            ops.push_back(binary_operator_map.at(cur));
                        }
                    } else {
                        error("operator '" + op2string(ops.back()) + "' has no right expected_ending");
                    }
                }
            }
            if (!safe_exit && expected_ending != EMPTY) {
                error("missing '" + expected_ending + "' in expression");
            }
            if (!values.size()) {
                error("value before " + expected_ending + " expected");
            }
            collapse(values, ops, SHORTCUT_LIMIT);
            if (values.size() != 1) {
                error("unexpected value size " + std::to_string(values.size())); // not possible ?
            }
            return values.back();
        }

        void read_until(const string& expected_ending) {
            if (expected_ending == EMPTY);
            else if (expected_ending == RIGHT_BRACKET) {
                int layer = 1;
                while (layer) {
                    if (!has_next()) {
                        error("missing ')' in expression");
                    }
                    const auto & next = get_next();
                    if (next == LEFT_BRACKET) layer++;
                    else if (next == RIGHT_BRACKET) layer--;
                }
            } else if (expected_ending == ":") {
                int layer = 1;
                while (layer) {
                    if (!has_next()) {
                        error("missing ':' in ternary expression");
                    }
                    const auto & next = get_next();
                    if (next == "?") layer++;
                    else if (next == ":") layer--;
                }
            } else {
                error("unknown expected ending " + expected_ending);
            }
        }


        void collapse(vector <int64_t>& values, vector <op>& ops, int delimiter_precedence, string op_str) {
            if (values.size() <= ops.size()) { // == ?
                error(ops.empty()
                      ? "operator" + (op_str == "" ? string("") : " '" + op_str + "'") + " has no left operand"
                      : "operator '" + op2string(ops.back()) + "' has no right operand");
            }
            while (!ops.empty() && precedence(ops.back()) <= delimiter_precedence) {
                auto rhs = values.back(); values.pop_back();
                auto lhs = values.back(); values.pop_back();
                auto cur_op = ops.back(); ops.pop_back();
                int64_t res;
                switch (cur_op) {
                    case ADD: res = lhs + rhs; break;
                    case SUB: res = lhs - rhs; break;
                    case MUL: res = lhs * rhs; break;
                    case DIV: if (rhs == 0) error("division by zero"); res = lhs / rhs; break;
                    case MOD: if (rhs == 0) error("division by zero"); res = lhs % rhs; break;
                    case SHL: res = lhs << rhs; break;
                    case SHR: res = lhs >> rhs; break;
                    case LT : res = lhs < rhs; break;
                    case LEQ: res = lhs <= rhs; break;
                    case GT : res = lhs > rhs; break;
                    case GEQ: res = lhs >= rhs; break;
                    case EQ : res = lhs == rhs; break;
                    case NEQ: res = lhs != rhs; break;
                    case AND: res = lhs & rhs; break;
                    case OR : res = lhs | rhs; break;
                    case XOR: res = lhs ^ rhs; break;
                    default:
                        // unary , && || ? (impossible)
                        error("unexpected error [unknown binary operator]");
                        res = -1;
                }
                values.push_back(res);
            }
        }

        /* eval expressions.
         * note that we should accept ill-formed expression sometimes, like:
         * >    #if defined(is_builtin) && is_builtin(bool)
         * it will be a evaluable but (in the normal sense) ill-formed expression when "is_builtin" is not defined.
         * so normal ways of parsing expressions will fail, and we should process it carefully.
         */

        int eval(const token_sequence &tokens) {
            token_sequence purified;
            // purify
            for (const auto &cur : tokens) {
                if (cur == SPACE || cur == EMPTY);
                else {
                    purified.push_back(cur);
                }
            }
            // check validity
            for (auto &cur : purified) {
                if (is_identifier(cur)) {
                    cur = "0";
                } else if (is_number_literal(cur)) {
                    cur = std::to_string(parse_integer(cur));
                } else {
                    // happy, do nothing
                }
            }
            current_line = std::move(purified);
            current_line_counter = 0;
            int64_t ret = do_eval(EMPTY);
//            debugstd("eval = " + std::to_string(ret));
            return ret != 0;
        }
    }

    bool is_sep(char c) {
        switch (c) {
            case 'A'...'Z':
            case 'a'...'z':
            case '0'...'9':
            case '_': case '$':
            case '\0'...' ':
                return false;
            default:
                return true;
        }
    }

    bool start_with_hash(const token_sequence &line) {
        return line.size() >= 1 && line[line[0] == SPACE && line.size() >= 2] == HASH;
    }

    token get_directive(const token_sequence &line) {
        // should check 'start_with_hash' before call
        size_t i = 1 + (line[0] == SPACE);
        if (i < line.size() && line[i] == SPACE) i++;
        if (i < line.size()) {
            if (directive_set.count(line[i])) {
                return line[i];
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
        return ret + "\"";
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
        line_no++;
        return ret;
    }

    string file_info::get_logical_line() {
        string ret;
        bool next_line_expected;
        do {
            if (eof()) break;
            string s = getline_std_string();
            if (!s.empty() && s.back() == '\\') {
                s.back() = ' ';
                next_line_expected = true;
            } else {
                next_line_expected = false;
            }
            ret += s;
        } while (next_line_expected);
        last_line = ret; // replace for better reading
        return ret;
    }

    token_sequence file_info::get_next_line() {
        token_sequence_builder ret;
        string s = get_logical_line();
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
                        j++;
                    }
                    if (found) {
                        ret.add(' ');
                        i = j + 2;
                    } else {
                        if (feof(file)) {
                            error("unterminated comment");
                        }
                        s += get_logical_line();
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

    file_info::file_info(const string &file_name) {
        name = file_name;
        time_stamp = get_file_modified_time(file_name);
        line_no = 0;
        if ((file = fopen(file_name.c_str(), "r")) == nullptr) {
            error(file_name + ": Failed to open file");
        }
    }

    file_info::~file_info() {
        if (file != nullptr) {
            fclose(file);
        }
    }

    bool file_info::eof() {
        int ch;
        bool ret = (ch = fgetc(file)) == EOF;
        if (!ret) ungetc(ch, file);
        return ret;
    }

    int64_t file_info::get_line_number() {
        return line_no;
    }

    void file_info::set_line_number(int64_t no) {
        line_no = no - 1;
    }

    const std::unordered_set<token> define_symbol_table::disabled_names = {
        "__VA_ARGS__",
        "__VA_OPT__",
        "defined"
    };

    define_symbol_table::define_symbol_table(string _name, std::unordered_map<token, token_sequence> _predefined) : name(std::move(_name)) {
        for (const auto &i : _predefined) {
            add_object(i.first, i.second);
        }
    }

    void define_symbol_table::add_object(const token &s, const token_sequence &tokenSequence) {
        add(s, define_info(true, {}, tokenSequence));
    }

    void define_symbol_table::add(const token &s, const define_info &defineInfo) {
        if (disabled_names.count(s)) {
            error("\"" + s + "\" cannot be used as a macro name");
        }
        auto iter = table.find(s);
        if (iter != table.end() && (iter->second.is_object_like != defineInfo.is_object_like || iter->second.params != defineInfo.params || iter->second.content != defineInfo.content)) {
            warning("\"" + s + "\" redefined");
        }
        table[s] = defineInfo;
    }

    bool define_symbol_table::contains(const token &s) const {
        return table.count(s) == 1;
    }

    define_info define_symbol_table::get(const token &s) {
        if (disabled_names.count(s)) {
            error("\"" + s + "\" cannot be used as a macro name");
        }
        auto x = table.find(s);
        if (x == table.end()) {
            error("failed to find element" + s + " in " + name);
        }
        return x->second;
    }

    void define_symbol_table::remove(const token &s) {
        if (disabled_names.count(s)) {
            error("\"" + s + "\" cannot be used as a macro name");
        }
        auto x = table.find(s);
        if (x != table.end()) {
            table.erase(x);
        }
    }

    void define_symbol_table::add_function(const token &s, const token_sequence &params, const token_sequence &content) {
        add(s, define_info(false, params, content));
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
            if (operators.count(current_token + c)) {
                current_token += c;
            } else {
                if (current_token != EMPTY) {
                    tokens.push_back(current_token);
                }
                current_token = {c};
            }
        } else {
            if (current_token != EMPTY && (current_token == SPACE || is_sep(current_token.back()))) {
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
        } /*else if(is_sep(tokens.back().back()) || is_sep(tokenSequence.front().front())) {
            error("pasting \"" + tokens.back() + "\" and \"" + tokenSequence.front() + "\" does not give a valid preprocessing token");
        } */else {
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

    bool token_sequence_builder::empty() {
        return current_token.empty() && tokens.empty();
    }

    void token_sequence_builder::clear() {
        tokens.clear();
        current_token.clear();
    }

    void output_buffer::push_back(const token_sequence& tokens) {
        if (tokens.size() == 0 || (tokens.size() == 1 && tokens.back() == SPACE)) {
            if (!last_empty) buffer += '\n';
            last_empty = true;
            return;
        }
        last_empty = false;
        for (const auto &i : tokens) {
            buffer += i;
        }
        buffer += '\n';
    }
    void output_buffer::flush(FILE* file) {
        fputs(buffer.c_str(), file);
        buffer.clear();
    }

    void output_buffer::clear() {
        buffer.clear();
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
void output_colored_error() {
    fprintf(stderr, "\033[1;31merror:\033[0m ");
}
void output_colored_warning() {
    fprintf(stderr, "\033[0;35mwarning:\033[0m ");
}
#endif
    const char help_msg[] = "Call of the forge god : A simple C / C++ preprocessor.\n"
                            "Usage : forge input_file [-o output_file] [-I include_dir]\n"
                            "Read configs from __predefined.cpp and include_dir.cfg, modify them as you need.\n";
    void show_help_msg() {
        fputs(help_msg, stderr);
    }

    char get_path_sep() {
#ifdef WINDOWS
        return ';';
#else
        return ':';
#endif
    }
} //FORGE_GOD
