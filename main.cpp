#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
// string utils, from
// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch) && ch != '\n';
          }));
}
inline void rtrim(std::string &s) {
  s.erase(std::find_if(
              s.rbegin(), s.rend(),
              [](unsigned char ch) { return !std::isspace(ch) && ch != '\n'; })
              .base(),
          s.end());
}
// end copy of
// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
// string utils from
// https://codereview.stackexchange.com/questions/239310/improving-function-that-replaces-all-instances-of-x-with-y-in-string
namespace utils {
using WordPair = std::pair<std::string_view, std::string_view>;
inline void replace_all(std::string &string, const WordPair &pr) {
  for (std::size_t start_pos{0};
       (start_pos = string.find(pr.first, start_pos)) != std::string::npos;
       start_pos += pr.second.length()) {
    string.replace(start_pos, pr.first.length(), pr.second);
  }
}
} // namespace utils
// end copy
// ansi colors
inline string c_black(string s) { return "\033[30m" + s + "\033[39m"; }
inline string c_red(string s) { return "\033[31m" + s + "\033[39m"; }
inline string c_green(string s) { return "\033[32m" + s + "\033[39m"; }
inline string c_brown(string s) { return "\033[33m" + s + "\033[39m"; }
inline string c_blue(string s) { return "\033[34m" + s + "\033[39m"; }
inline string c_magenta(string s) { return "\033[35m" + s + "\033[39m"; }
inline string c_cyan(string s) { return "\033[36m" + s + "\033[39m"; }
inline string c_white(string s) { return "\033[37m" + s + "\033[39m"; }
inline string c_bold(string s) { return "\033[1m" + s + "\033[22m"; }
inline string c_half_bright(string s) { return "\033[2m" + s + "\033[22m"; }
// tokenizer
struct Token {
  enum Type { literal, tree_node_nesting_value, todo, next, done } type;
  string str_value;
  size_t int_value;
};
vector<Token> tokenize(istream &s) {
  vector<Token> tokens;
  string line;
  while (getline(s, line)) {
    if (auto it =
            find_if(line.begin(), line.end(), [](char c) { return c != '*'; });
        it != line.begin()) {
      const string before(line.begin(), it);
      const size_t nest_level = count(before.begin(), before.end(), '*');
      tokens.emplace_back(Token::tree_node_nesting_value, before, nest_level);
      if (it == line.end())
        continue;
      ++it;
      if (it == line.end())
        continue;
      const string after(it, line.end());
      line = after;
    }
    const auto extract_status_token = [&](const string &token_str,
                                          Token::Type type) {
      if (line.find(token_str) == 0) {
        tokens.emplace_back(type, string{}, 0);
        return true;
      }
      return false;
    };
    bool has_status = extract_status_token("TODO", Token::todo);
    has_status |= extract_status_token("NEXT", Token::next);
    has_status |= extract_status_token("DONE", Token::done);
    if (has_status) {
      const auto after_begin = next(line.begin(), 5);
      if (after_begin < line.end()) {
        const string after(after_begin, line.end());
        line = after;
      } else {
        line = "";
      }
    }
    tokens.emplace_back(Token::literal, line, 0);
  }
  return tokens;
}
// parser
struct Leaf {
  enum State { _, TODO, NEXT, DONE } state{_};
  string title{};
  string str{};
  size_t level{0};
  bool has_TODO_child{false};
  bool has_NEXT_child{false};
  bool is_goal{false};
};
string to_string(Leaf::State s) {
  switch (s) {
  case Leaf::State::_:
    return "    ";
  case Leaf::State::TODO:
    return "TODO";
  case Leaf::State::NEXT:
    return "NEXT";
  case Leaf::State::DONE:
    return "DONE";
  }
  return "*UNIMPLEMENTED*";
}
vector<Leaf> parse_tree(const vector<Token> &tokens, const bool compact) {
  vector<Leaf> children;
  {
    Leaf root;
    children.push_back(root);
  }
  const auto mark_parents = [&](Leaf::State s) {
    size_t marking_level = children.back().level;
    size_t current_level = marking_level + 1;
    bool found_goal = false;
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      auto &l = *it;
      if (l.level >= current_level)
        continue;
      if (l.level == current_level)
        continue;
      if (s == Leaf::NEXT) {
        l.has_NEXT_child = true;
      } else if (s == Leaf::TODO) {
        l.has_TODO_child = true;
      }
      if (!found_goal && l.level < marking_level) {
        l.is_goal = true;
        found_goal = true;
      }
      current_level = l.level;
    }
  };
  bool parsing_title{true};
  for (const auto &t : tokens) {
    Leaf &current = children.back();
    if (t.type == Token::literal) {
      if (t.str_value.empty()) {
        parsing_title = false;
        continue;
      }
      string s = parsing_title ? current.title : current.str;
      if (!s.empty()) {
        s += parsing_title ? " " : "\n";
      }
      s.append(t.str_value);
      if (parsing_title) {
        current.title = s;
      } else {
        current.str = s;
      }
    } else if (t.type == Token::todo) {
      current.state = Leaf::TODO;
      mark_parents(Leaf::TODO);
    } else if (t.type == Token::next) {
      current.state = Leaf::NEXT;
      mark_parents(Leaf::NEXT);
    } else if (t.type == Token::done) {
      current.state = Leaf::DONE;
    } else if (t.type == Token::tree_node_nesting_value) {
      if (!current.str.empty()) {
        rtrim(current.str);
        ltrim(current.str);
      }
      Leaf l{};
      l.level = t.int_value;
      children.push_back(l);
      parsing_title = true;
    }
  }
  return children;
}
// main
void display_help(string_view executable_name) {
  cout << "Usage:" << endl;
  cout << executable_name << " TODO input.org" << endl;
  cout << '\t' << " to get all the TODOs" << endl;
  cout << executable_name << " NEXT --compact input.org" << endl;
  cout << '\t' << " to get all the NEXTs, in compact form" << endl;
}
int main(int argc, char **argv) {
  vector<string> arguments(argv, argv + argc);
  if (arguments.size() < 3) {
    display_help(arguments.front());
    return -1;
  }
  bool compact = false;
  for (const auto &a : arguments) {
    if (a == "--help" || a == "-h") {
      display_help(arguments.front());
      return 0;
    }
    if (a == "--compact") {
      compact = true;
      continue;
    }
  }
  if (arguments[1] != "TODO" && arguments[1] != "NEXT") {
    cout << "Unknown state tag:'" << arguments[1] << "'" << endl;
    display_help(arguments.front());
    return -1;
  }
  ifstream file(arguments.back());
  const auto tokens = tokenize(file);
  const auto tree = parse_tree(tokens, compact);

  size_t max_level = 0;
  for (const auto &t : tree) {
    if ((arguments[1] == "TODO" && t.has_TODO_child) ||
        (arguments[1] == "NEXT" && t.has_NEXT_child)) {
      max_level = max(max_level, t.level);
    }
  }
  for (const auto &t : tree) {
    if ((arguments[1] == "TODO" && t.has_TODO_child) ||
        (arguments[1] == "NEXT" && t.has_NEXT_child)) {
      if (t.level > 0) {
        cout << c_half_bright(string(t.level - 1, '|')) << c_bold({"*"})
             << c_blue(c_half_bright(string(max_level - t.level, '.'))) << " ";
      } else {
        cout << (string(max_level + 1, ' '));
      }
      if (t.is_goal) {
        cout << c_bold(c_magenta("GOAL")) << " ";
      } else if (t.state == Leaf::State::DONE) {
        cout << c_bold(c_blue(to_string(t.state))) << " ";
      } else {
        cout << c_bold(c_green(to_string(t.state))) << " ";
      }
      cout << t.title;
      if (!compact && !t.str.empty() && t.level > 0) {
        const string filler = string(t.level - 1, '|') + "|" +
                              string(max_level - t.level, ' ') + " " + "     ";
        string text = t.str;
        utils::replace_all(text, {"\n", "\n" + c_blue(filler)});
        cout << endl << c_half_bright(c_blue(filler)) << c_half_bright(text);
      }
      cout << endl;
    }
  }
  return 0;
}
