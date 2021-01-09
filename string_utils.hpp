#pragma once
#include <string>

namespace utils {
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
inline void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}
// end copy of
// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
// string utils from
// https://codereview.stackexchange.com/questions/239310/improving-function-that-replaces-all-instances-of-x-with-y-in-string
using WordPair = std::pair<std::string_view, std::string_view>;
inline void replace_all(std::string &string, const WordPair &pr) {
  for (std::size_t start_pos{0};
       (start_pos = string.find(pr.first, start_pos)) != std::string::npos;
       start_pos += pr.second.length()) {
    string.replace(start_pos, pr.first.length(), pr.second);
  }
}
// end copy
} // namespace utils
// ansi colors
inline std::string c_black(std::string s) {
  return "\033[30m" + s + "\033[39m";
}
inline std::string c_red(std::string s) { return "\033[31m" + s + "\033[39m"; }
inline std::string c_green(std::string s) {
  return "\033[32m" + s + "\033[39m";
}
inline std::string c_brown(std::string s) {
  return "\033[33m" + s + "\033[39m";
}
inline std::string c_blue(std::string s) { return "\033[34m" + s + "\033[39m"; }
inline std::string c_magenta(std::string s) {
  return "\033[35m" + s + "\033[39m";
}
inline std::string c_cyan(std::string s) { return "\033[36m" + s + "\033[39m"; }
inline std::string c_white(std::string s) {
  return "\033[37m" + s + "\033[39m";
}
inline std::string c_bold(std::string s) { return "\033[1m" + s + "\033[22m"; }
inline std::string c_half_bright(std::string s) {
  return "\033[2m" + s + "\033[22m";
}
