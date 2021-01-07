#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// trim from start (in place)
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch) && ch != '\n';
          }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(
              s.rbegin(), s.rend(),
              [](unsigned char ch) { return !std::isspace(ch) && ch != '\n'; })
              .base(),
          s.end());
}

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
    const auto extract_status_token = [&](const char *token_str,
                                          Token::Type type) {
      if (const auto pos = line.find(token_str); pos == 0) {
        const string before(line.begin(), next(line.begin(), pos));
        const string empty{};
        tokens.emplace_back(Token::todo, empty, 0);
        const auto after_begin = next(line.begin(), pos + 5);
        if (after_begin < line.end()) {
          const string after(after_begin, line.end());
          line = after;
        } else {
          line = "";
        }
      }
    };
    extract_status_token("TODO", Token::todo);
    extract_status_token("NEXT", Token::next);
    extract_status_token("DONE", Token::done);
    if (!line.empty()) {
      tokens.emplace_back(Token::literal, line, 0);
    }
  }
  return tokens;
}

struct Leaf {
  enum State { _, TODO, NEXT, DONE } state{_};
  string str{};
  size_t level{0};
  bool has_TODO_child{false};
  bool has_NEXT_child{false};
};

string to_string(Leaf::State s) {
  switch (s) {
  case Leaf::State::_:
    return "";
  case Leaf::State::TODO:
    return "TODO";
  case Leaf::State::NEXT:
    return "NEXT";
  case Leaf::State::DONE:
    return "DONE";
  }
  assert("unimplemented");
  return "UNIMPLEMENTED";
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
      current_level = l.level;
    }
  };
  for (const auto &t : tokens) {
    Leaf &current = children.back();
    // cout << "///" << endl;
    if (t.type == Token::literal) {
      string s = current.str;
      // cout << "current = " << s << ", with size " << s.size() << endl;
      // cout << "t.s = " << t.str_value << ", with size" << t.str_value.size()
      // << endl;
      if (!s.empty()) {
        s += "\n";
      }
      s.append(t.str_value);
      current.str = s;
      // cout << "current = " << current.str << endl;
    } else if (t.type == Token::todo) {
      current.state = Leaf::TODO;
      mark_parents(Leaf::TODO);
    } else if (t.type == Token::next) {
      current.state = Leaf::NEXT;
      mark_parents(Leaf::NEXT);
    } else if (t.type == Token::done) {
      current.state = Leaf::DONE;
    } else if (t.type == Token::tree_node_nesting_value) {
      if (compact && !current.str.empty()) {
        rtrim(current.str);
        ltrim(current.str);
      }
      Leaf l{};
      l.level = t.int_value;
      children.push_back(l);
      // cout << "NEW LEAF:" << l.level << endl;
    }
  }
  return children;
}

void display_help(string_view executable_name) {
  cout << "Usage:" << endl;
  cout << executable_name << " input.org TODO" << endl;
  cout << '\t' << " to get all the TODOs" << endl;
  cout << executable_name << " input.org NEXT" << endl;
  cout << '\t' << " to get all the NEXTs" << endl;
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
  ifstream file(arguments[1]);
  const auto tokens = tokenize(file);
  const auto tree = parse_tree(tokens, compact);

  for (const auto &t : tree) {
    if ((arguments[2] == "TODO" && t.has_TODO_child) ||
        (arguments[2] == "NEXT" && t.has_NEXT_child)) {
      cout << string(t.level, '*') << " ";
      if (t.state != Leaf::State::_) {
        cout << to_string(t.state) << " ";
      }
      cout << t.str << endl;
    }
  }
  return 0;
}
