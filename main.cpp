#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "string_utils.hpp"

using namespace std;
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
  size_t parent_id{static_cast<size_t>(-1)};
  vector<size_t> local_goal_numbering;
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
string local_goal_numbering_to_string(vector<size_t> local_goal_numbering) {
  string s;
  for (const auto i : local_goal_numbering) {
    if (!s.empty()) {
      s += ".";
    }
    s += to_string(i);
  }
  return s;
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
        trim(s);
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
  if (!children.empty()) {
    size_t last_parent_id = 0;
    // Update parent_id
    for (size_t i = 0; i < children.size(); ++i) {
      auto &l = children[i];
      if (l.parent_id != static_cast<size_t>(-1)) {
        continue;
      }
      if (l.level <= 1) {
        l.parent_id = 0;
        continue;
      }
      while (l.level <= children[last_parent_id].level) {
        last_parent_id = children[last_parent_id].parent_id;
      }
      l.parent_id = last_parent_id;
      last_parent_id = i;
    }
    // Update local_goal_numbering
    for (size_t i = 0; i < children.size(); ++i) {
      const auto &goal = children[i];
      if (goal.is_goal) {
        const auto goal_id = i;
        const auto goal_level = goal.level;
        vector<size_t> current_numbering{};
        size_t current_level = 0;
        for (++i; i < children.size() && children[i].level > goal_level; ++i) {
          auto &l = children[i];
          l.is_goal = false; // keep only top goal
          if (current_level == l.level) {
            current_numbering.back()++;
          } else if (current_level < l.level) {
            current_numbering.push_back(1);
          } else {
            current_numbering.pop_back();
            current_numbering.back()++;
          }
          current_level = l.level;
          l.local_goal_numbering = current_numbering;
        }
      }
    }
  }
  return children;
}
// main
void display_help(string_view executable_name) {
  cout << "Usage:"
       << "\n";
  cout << executable_name << " TODO input.org"
       << "\n";
  cout << '\t' << " to get all the TODOs"
       << "\n";
  cout << executable_name << " NEXT --compact input.org"
       << "\n";
  cout << '\t' << " to get all the NEXTs, in compact form"
       << "\n";
  cout << executable_name << " GOALS --compact input.org"
       << "\n";
  cout << '\t' << " to list all the GOALS, in compact form"
       << "\n";
  cout << executable_name << " GOAL input.org"
       << "\n";
  cout << '\t' << " to get info for a GOAL"
       << "\n";
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
  if (arguments[1] != "TODO" && arguments[1] != "NEXT" &&
      arguments[1] != "GOALS" && arguments[1] != "GOAL") {
    cout << "Unknown state tag:'" << arguments[1] << "'"
         << "\n";
    display_help(arguments.front());
    return -1;
  }
  ifstream file(arguments.back());
  const auto tokens = tokenize(file);
  const auto tree = parse_tree(tokens, compact);

  if (arguments[1] == "GOAL") {
    if (arguments.size() < 4) {
      cout << "Missing goal name after GOAL option\n";
      display_help(arguments.front());
      return -1;
    }
    const auto goal_name = arguments[2];
    // find goal boundary
    int start_i = -1;
    int end_i = tree.size();
    size_t goal_level = 0;
    for (size_t i = 0; i < tree.size(); ++i) {
      auto &t = tree[i];
      if (start_i == -1) {
        if (auto pos = t.title.find(goal_name); pos == 0) {
          start_i = i;
          goal_level = t.level;
        }
      } else {
        if (t.level <= goal_level) {
          end_i = i;
          break;
        }
      }
    }
    const auto &goal = tree[start_i];
    cout << c_half_bright("           Goal: ") << c_bold(c_green((goal.title)))
         << '\n';
    cout << c_half_bright("Tasks:") << '\n';
    for (size_t i = start_i + 1; i < end_i; ++i) {
      auto &t = tree[i];
      if (t.local_goal_numbering.size() == 1) {
        cout << c_half_bright(to_string(t.local_goal_numbering.front()))
             << c_half_bright(" - ");
        if (t.state == Leaf::State::DONE || t.state == Leaf::State::_) {
          cout << c_half_bright((to_string(t.state))) << " ";
          cout << c_half_bright(":  ") << c_half_bright(t.title) << '\n';
        } else {
          cout << c_bold(c_green(to_string(t.state))) << " ";
          cout << c_half_bright(":  ") << (t.title) << '\n';
        }
      }
    }
  }
  if (arguments[1] == "GOALS") {
    for (size_t i = 0; i < tree.size(); ++i) {
      auto &t = tree[i];
      if (!t.is_goal) {
        continue;
      }
      if (!compact) {
        cout << c_half_bright("GOAL") << " ";
      }
      cout << c_bold(c_magenta(t.title)) << " ";
      if (!compact) {
        cout << '\n';
        if (!t.str.empty()) {
          const string filler = "     ";
          string text = t.str;
          utils::replace_all(text, {"\n", "\n" + c_blue(filler)});
          cout << c_half_bright(c_blue(filler)) << c_half_bright(text);
          cout << '\n';
        }
      }
      for (++i; i < tree.size() && tree[i].level > t.level; ++i) {
        if (tree[i].state == Leaf::TODO || tree[i].state == Leaf::NEXT) {
          cout << (c_half_bright(to_string(tree[i].state))) << " ";
          cout << (tree[i].title);
          if (!compact && !tree[i].str.empty()) {
            const string filler = "     ";
            string text = tree[i].str;
            utils::replace_all(text, {"\n", "\n" + c_blue(filler)});
            cout << '\n';
            cout << c_half_bright(c_blue(filler)) << c_half_bright(text);
          }
          cout << '\n';
          break;
        }
      }
      if (!compact) {
        cout << '\n';
      }
    }
  } else {
    size_t max_level = 0;
    for (const auto &t : tree) {
      if ((arguments[1] == "TODO" && t.has_TODO_child) ||
          (arguments[1] == "NEXT" && t.has_NEXT_child)) {
        max_level = max(max_level, t.level);
      }
    }
    for (size_t i = 0; i < tree.size(); ++i) {
      auto &t = tree[i];
      if ((arguments[1] == "TODO" && t.has_TODO_child) ||
          (arguments[1] == "NEXT" && t.has_NEXT_child)) {
        // cout << "id:" << i << "\t" << "p_id:" << t.parent_id << "\t" <<
        // "g:"
        // << local_goal_numbering_to_string(t.local_goal_numbering) << "\t";
        if (t.level > 0) {
          cout << c_half_bright(string(t.level - 1, '|')) << c_bold({"*"});
          if (t.is_goal) {
            cout << c_magenta(c_bold(string(max_level - t.level, '.'))) << " ";
          } else {
            cout << c_blue(c_half_bright(string(max_level - t.level, '.')))
                 << " ";
          }
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
        if (!t.local_goal_numbering.empty()) {
          cout << c_half_bright(
              local_goal_numbering_to_string(t.local_goal_numbering) + ") ");
        }
        cout << t.title;
        if (!compact && !t.str.empty() && t.level > 0) {
          const string filler = string(t.level - 1, '|') + "|" +
                                string(max_level - t.level, ' ') + " " +
                                "     ";
          string text = t.str;
          utils::replace_all(text, {"\n", "\n" + c_blue(filler)});
          cout << c_half_bright(c_blue(filler)) << c_half_bright(text);
        }
        cout << "\n";
      }
    }
  }
  return 0;
}
