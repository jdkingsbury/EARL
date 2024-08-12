/** @file */

// MIT License

// Copyright (c) 2023 malloc-nbytes

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <vector>
#include <iostream>
#include <memory>

#include "repl.hpp"
#include "parser.hpp"
#include "utils.hpp"
#include "interpreter.hpp"
#include "intrinsics.hpp"
#include "err.hpp"
#include "token.hpp"
#include "ast.hpp"
#include "ctx.hpp"
#include "common.hpp"
#include "earl.hpp"
#include "lexer.hpp"

#define EARL_REPL_HISTORY_FILENAME ".earl_history"
#define REPL_HISTORY_MAX_FILESZ 1024 * 1024

#define YELLOW "\033[93m"
#define GREEN "\033[32m"
#define GRAY "\033[90m"
#define RED "\033[31m"
#define BLUE "\033[94m"
#define NOC "\033[0m"

#define QUIT ":q"
#define CLEAR ":c"
#define SKIP ":skip"
#define HELP ":help"
#define RM_ENTRY ":rm"
#define EDIT_ENTRY ":e"
#define LIST_ENTRIES ":show"
#define IMPORT ":i"
#define DISCARD ":d"

static std::string REPL_HIST = "";

void
try_clear_repl_history() {
    const char* home_dir = std::getenv("HOME");
    if (home_dir == nullptr) {
        std::cerr << "Unable to get home directory path. Will not clear REPL history." << std::endl;
        return;
    }

    std::string fp = std::string(home_dir) + "/" + EARL_REPL_HISTORY_FILENAME;

    std::ifstream file(fp, std::ios::binary);
    if (!file) {
        std::cerr << "Unable to open file for reading. Will not clear REPL history." << std::endl;
        return;
    }

    file.seekg(0, std::ios::end);
    std::streampos filesz = file.tellg();
    file.close();

    if (filesz >= REPL_HISTORY_MAX_FILESZ) {
        std::ofstream file(fp, std::ios::trunc | std::ios::binary);
        if (!file) {
            std::cerr << "Unable to open file for clearing. Will not clear REPL history." << std::endl;
            return;
        }
        file.close();
        std::cout << "REPL history file cleared." << std::endl;
    }
}

void
yellow(void) {
    if ((flags & __REPL_NOCOLOR) == 0)
        std::cout << YELLOW;
}

void
blue(void) {
    if ((flags & __REPL_NOCOLOR) == 0)
        std::cout << BLUE;
}

void
green(void) {
    if ((flags & __REPL_NOCOLOR) == 0)
        std::cout << GREEN;
}

void
gray(void) {
    if ((flags & __REPL_NOCOLOR) == 0)
        std::cout << GRAY;
}

void
red(void) {
    if ((flags & __REPL_NOCOLOR) == 0)
        std::cout << RED;
}

void
noc(void) {
    if ((flags & __REPL_NOCOLOR) == 0)
        std::cout << NOC;
}

void
save_repl_history() {
    const char *home_dir = std::getenv("HOME");
    if (home_dir == nullptr)
        std::cerr << "Unable to get home directory path. Will not save REPL history." << std::endl;
    std::string fp = std::string(home_dir) + "/" EARL_REPL_HISTORY_FILENAME;
    std::ofstream of(fp, std::ios::app);
    if (!of)
        std::cerr << "Unable to open file for writing. Will not save REPL history." << std::endl;
    of << REPL_HIST;
    of.close();
}

void
log(std::string msg, void(*color)(void) = nullptr) {
    blue();
    std::cout << "[EARL] ";
    noc();
    if (color)
        color();
    std::cout << msg;
    noc();
}

std::vector<std::string>
split_on_space(std::string &line) {
    std::vector<std::string> res = {};
    std::stringstream ss(line);
    std::string word;
    while (ss >> word)
        res.push_back(word);
    return res;
}

std::vector<std::string>
split_on_newline(std::string &line) {
    std::vector<std::string> res = {};
    std::string buf = "";
    for (auto &c : line) {
        if (c == '\n') {
            res.push_back(buf);
            buf = "";
        }
        else
            buf += c;
    }
    return res;
}

void
help(void) {
    log(HELP " -> show this message\n");
    log(QUIT " -> quit the session\n");
    log(SKIP " -> skip current action\n");
    log(CLEAR " -> clear the screen\n");
    log(IMPORT " [files...] -> import local EARL files\n");
    log(RM_ENTRY " [lineno...] -> remove entries (previous if blank)\n");
    log(EDIT_ENTRY " [lineno...] -> edit previous entries (previous if blank)\n");
    log(LIST_ENTRIES " -> list all entries in the current session\n");
    log(DISCARD " -> discard the current session\n");
    log("You can also issue bash commands by typing `$` followed by the command\n");
}

void
import_file(std::vector<std::string> &args, std::vector<std::string> &lines) {
    if (args.size() == 0) {
        log("No files supplied\n");
        return;
    }
    for (auto &f : args) {
        const char *src_c = read_file(f.c_str());
        std::string src = std::string(src_c);
        auto src_lines = split_on_newline(src);
        std::for_each(src_lines.begin(), src_lines.end(), [&](auto &l){lines.push_back(l);});
        // green();
        log("Imported " + f + "\n", green);
        noc();
    }
}

std::string
get_special_input(void) {
    std::cout << ">>> ";
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void
rm_entries(std::vector<std::string> &args, std::vector<std::string> &lines) {
    if (lines.size() == 0) {
        log("No previous entry exists\n");
        return;
    }

    std::vector<std::string> hist = {};

    if (args.size() == 0) {
        hist.push_back(lines.back());
        lines.pop_back();
    }
    else {
        int i = 0;
        for (auto &arg : args) {
            int lnum = i == 0 ? std::stoi(arg) : std::stoi(arg)-i;
            if (lnum < 0 || lnum >= (int)lines.size()) {
                log("Line number is out of range for session\n");
                return;
            }
            hist.push_back(lines[lnum]);
            lines.erase(lines.begin()+lnum);
            ++i;
        }
    }

    log("Removed:\n");
    for (auto &h : hist) {
        red();
        std::cout << h;
        noc();
        std::cout << std::endl;
    }
}

void
edit_entry(std::vector<std::string> &args, std::vector<std::string> &lines) {
    log("`" SKIP "` to skip current selection or `" QUIT "` to cancel the session editing\n", gray);
    if (args.size() > 0) {
        for (auto arg : args) {
            int lnum = std::stoi(arg);
            if (lnum < 0 || lnum >= (int)lines.size()) {
                log("Line number is out of range for session\n");
                return;
            }
            log("Editing [", gray);
            yellow();
            std::cout << lines.at(lnum);
            gray();
            std::cout << "]" << std::endl;
            noc();
            std::string newline = get_special_input();
            if (newline == QUIT) {
                log("Quitting session editor\n", gray);
                return;
            }
            if (newline == SKIP)
                log("Skipping current selection\n", gray);
            else
                lines.at(lnum) = newline;
        }
    }
    else {
        if (lines.size() == 0) {
            log("No lines to edit\n", gray);
            return;
        }
        log("Editing [", gray);
        yellow();
        std::cout << lines.back();
        gray();
        std::cout << "]" << std::endl;
        noc();
        std::string newline = get_special_input();
        lines.at(lines.size()-1) = newline;
    }
}

void
clearscrn(void) {
    if (system("clear") != 0)
        log("warning: failed to clearscrn\n", yellow);
}

void
ls_entries(std::vector<std::string> &lines) {
    if (lines.size() == 0) {
        log("Nothing appropriate\n", gray);
        return;
    }
    for (size_t i = 0; i < lines.size(); ++i) {
        green();
        std::cout << i << ": ";
        std::cout << lines[i];
        noc();
        std::cout << std::endl;
    }
}

void
bash(std::string &line) {
    std::string cmd = line.substr(1, line.size());
    if (system(cmd.c_str()) != 0)
        log("warning: failed to ls\n", yellow);
}

void
discard(std::vector<std::string> &lines) {
    auto to_lower = [](std::string &str) {
        for (char& c : str)
            c = std::tolower(static_cast<unsigned char>(c));
    };

    std::string line = "";
    while (true) {
        log("Discard the current session? [Y/n]: ", red);
        std::getline(std::cin, line);
        to_lower(line);
        if (line == "" || line == "y" || line == "yes") {
            lines.clear();
            return;
        }
        else if (line == "n" || line == "no") {
            return;
        }
        else
            log("invalid input\n");
    }
}

void
handle_repl_arg(std::string &line, std::vector<std::string> &lines) {
    std::vector<std::string> lst = split_on_space(line);
    std::vector<std::string> args(lst.begin()+1, lst.end());

    if (line.size() != 0 && line[0] == '$')
        bash(line);
    else if (lst[0] == RM_ENTRY)
        rm_entries(args, lines);
    else if (lst[0] == EDIT_ENTRY)
        edit_entry(args, lines);
    else if (lst[0] == LIST_ENTRIES)
        ls_entries(lines);
    else if (lst[0] == IMPORT)
        import_file(args, lines);
    else if (lst[0] == CLEAR)
        clearscrn();
    else if (lst[0] == QUIT)
        exit(0);
    else if (lst[0] == DISCARD)
        discard(lines);
    else if (lst[0] == HELP)
        help();
    else
        log("unknown command sequence `" + lst[0] + "`\n", gray);
}

void
analyze_eol(std::string &line, int &brace, int &bracket, int &paren) {
    for (auto it = line.rbegin(); it != line.rend(); ++it) {
        switch (*it) {
        case '{': ++brace; break;
        case '}': --brace; break;
        case '[': ++bracket; break;
        case ']': --bracket; break;
        case '(': ++paren; break;
        case ')': --paren; break;
        case ';': break;
        default: break;
        }
    }
}

std::shared_ptr<Ctx>
Repl::run(void) {
    try_clear_repl_history();

    std::vector<std::string> keywords = COMMON_EARLKW_ASCPL;
    std::vector<std::string> types    = {};
    std::string comment               = COMMON_EARL_COMMENT;

    std::shared_ptr<Ctx> ctx = std::make_shared<WorldCtx>();

    while (true) {
        std::string line;
        std::vector<std::string> lines;

        int brace = 0;
        int bracket = 0;
        int paren = 0;
        int i = 0;

        while (1) {
            i = lines.size();
            std::cout << i << ": ";
            std::getline(std::cin, line);
            if (line.size() > 0 && (line[0] == ':' || line[0] == '$')) {
                handle_repl_arg(line, lines);
                --i;
            }
            else {
                analyze_eol(line, brace, bracket, paren);
                if (line.size() == 0 && !brace && !bracket && !paren)
                    break;
                lines.push_back(line);
            }
        }

        std::string combined = "";
        std::for_each(lines.begin(), lines.end(), [&](auto &s) {combined += s + "\n"; });
        REPL_HIST += combined;

        std::unique_ptr<Program> program = nullptr;
        std::unique_ptr<Lexer> lexer = nullptr;
        lexer = lex_file(combined.c_str(), "", keywords, types, comment);
        try {
            program = Parser::parse_program(*lexer.get());
        } catch (const ParserException &e) {
            std::cerr << "Parser error: " << e.what() << std::endl;
            continue;
        }
        WorldCtx *wctx = dynamic_cast<WorldCtx*>(ctx.get());
        wctx->add_repl_lexer(std::move(lexer));
        wctx->add_repl_program(std::move(program));

        for (size_t i = 0; i < wctx->stmts_len(); ++i) {
            Stmt *stmt = wctx->stmt_at(i);
            if (!stmt->m_evald) {
                try {
                    auto val = Interpreter::eval_stmt(stmt, ctx);
                    if (val) {
                        std::vector<std::shared_ptr<earl::value::Obj>> params = {val};
                        green();
                        (void)Intrinsics::intrinsic_print(params, ctx);
                        std::cout << " -> ";
                        gray();
                        std::cout << earl::value::type_to_str(val->type()) << std::endl;
                        noc();
                    }
                }
                catch (InterpreterException &e) {
                    std::cerr << "Interpreter error: " << e.what() << std::endl;
                }
            }
        }

        save_repl_history();
    }

    return nullptr;
}

