#ifndef TAGINFO_VALIDATE_ARGUMENT_PARSER
#define TAGINFO_VALIDATE_ARGUMENT_PARSER

#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>

namespace taginfo_validate {
namespace commandline {

struct arguments {
  boost::filesystem::path taginfo;
  boost::filesystem::path osm;
};

arguments make_arguments(int argc, char **argv) {
  boost::program_options::options_description options{"Options"};

  options.add_options()                                                                     //
      ("help", "Help on usage")                                                             //
      ("taginfo", boost::program_options::value<std::string>()->required(), "taginfo.json") //
      ("osm", boost::program_options::value<std::string>()->required(), "extract.osm.pbf")  //
      ;

  boost::program_options::variables_map map;
  boost::program_options::store(boost::program_options::command_line_parser{argc, argv}.options(options).run(), map);

  if (map.count("help")) {
    std::cerr << options << std::endl;
    std::exit(EXIT_FAILURE);
  }

  boost::program_options::notify(map);

  return arguments{map["taginfo"].as<std::string>(), map["osm"].as<std::string>()};
}
}
}

#endif
