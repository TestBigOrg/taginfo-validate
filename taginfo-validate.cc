#include <algorithm>
#include <iterator>
#include <stdexcept>

#include <osmium/handler.hpp>
#include <osmium/io/pbf_input.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/visitor.hpp>

#include "argument_parser.hpp"
#include "taginfo_parser.hpp"

using namespace taginfo_validate;

int main(int argc, char **argv) try {
  const auto args = commandline::make_arguments(argc, argv);
  const taginfo_parser taginfo{args.taginfo};

  std::ostream_iterator<taginfo_parser::tag> out{std::cout, "\n"};

  const auto node_tags = taginfo.node_tags();
  const auto way_tags = taginfo.way_tags();
  const auto relation_tags = taginfo.relation_tags();
  const auto all_tags = taginfo.all_tags();

  std::cout << ">>> nodes only\n";
  std::copy(node_tags.first, node_tags.second, out);

  std::cout << "\n\n>>> ways only\n";
  std::copy(way_tags.first, way_tags.second, out);

  std::cout << "\n\n>>> relation only\n";
  std::copy(relation_tags.first, relation_tags.second, out);

  std::cout << "\n\n>>> all only\n";
  std::copy(all_tags.first, all_tags.second, out);

} catch (const std::exception &e) {
  std::cerr << "Error: " << e.what() << std::endl;
  return EXIT_FAILURE;
}
