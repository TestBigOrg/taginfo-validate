#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include <osmium/io/any_input.hpp>
#include <osmium/visitor.hpp>

#include "argument_parser.hpp"
#include "taginfo_parser.hpp"

#include "qa_handler.hpp"

using namespace taginfo_validate;

int main(int argc, char **argv) try {
  std::unordered_map<std::string, uint32_t> string_catalogue;

  const auto args = commandline::make_arguments(argc, argv);
  const taginfo_parser taginfo{args.taginfo, string_catalogue};
  const std::string osmFile = args.osm.string();
  bool unknowns = args.print_unknowns;

  osmium::io::Reader osmFileReader(osmFile);
  qa_handler handler(taginfo, string_catalogue);
  osmium::apply(osmFileReader, handler);
  if (unknowns) {
    handler.printUnknowns();
  }
  handler.printMissing();

} catch (const std::exception &e) {
  std::cerr << "Error: " << e.what() << std::endl;
  return EXIT_FAILURE;
}
