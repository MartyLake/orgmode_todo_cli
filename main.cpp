#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main() {
  using namespace std;
  auto ss = std::ostringstream{};
  std::ifstream file("input.org");
  ss << file.rdbuf();
  cout << ss.str() << endl;
}
