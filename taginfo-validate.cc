#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include <osmium/io/pbf_input.hpp>
#include <osmium/visitor.hpp>

#include "argument_parser.hpp"
#include "taginfo_parser.hpp"

#include "qa_handler.hpp"

// TODO somehow store types that are absent from input pbf

using namespace taginfo_validate;

int main(int argc, char **argv) try {
  const auto args = commandline::make_arguments(argc, argv);
  const taginfo_parser taginfo{args.taginfo};
  const std::string osmFile = args.osm.string();

  osmium::io::Reader osmFileReader(osmFile);
  qa_handler handler(taginfo);
  osmium::apply(osmFileReader, handler);
  handler.printUnknowns();

} catch (const std::exception &e) {
  std::cerr << "Error: " << e.what() << std::endl;
  return EXIT_FAILURE;
}
