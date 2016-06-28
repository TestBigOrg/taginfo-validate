#include <stdexcept>

#include <osmium/handler.hpp>
#include <osmium/io/pbf_input.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/visitor.hpp>

#include <rapidjson/rapidjson.h>

#include "argument_parser.hpp"

int main(int argc, char **argv) try {
  const auto args = taginfo_validate::commandline::make_arguments(argc, argv);

} catch (const std::exception &e) {
  std::cerr << "Error: " << e.what() << std::endl;
  return EXIT_FAILURE;
}
