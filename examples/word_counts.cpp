#include "sequence.h"
#include "get_time.h"
#include "strings/string_basics.h"
#include "parse_command_line.h"
#include "group_by.h"

using namespace pbbs;

int main (int argc, char *argv[]) {
  commandLine P(argc, argv, "[-r <rounds>] infile");
  int rounds = P.getOptionIntValue("-r", 3);
  char* filename = P.getArgument(0);

  timer t("word counts", true);
  
  pbbs::sequence<char> str = pbbs::char_seq_from_file(filename);
  t.next("read file");

  auto is_line_break = [&] (char a) {return a == '\n' || a == '\r';};
  auto is_space = [&] (char a) {return std::isspace(a);};

  size_t lines, words;
  for (int i=0; i < rounds; i++) {
    lines = count_if(str, is_line_break);
    auto x = seq(str.size()-1, [&] (size_t i) {
	return is_space(str[i]) && !is_space(str[i+1]);});
    words = count(x, true);
  }
  t.next("calculate counts");
  
  cout << "\t" << lines << "\t" << words << "\t"
       << str.size() << "\t" << filename << endl;
}

