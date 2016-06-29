#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include <osmium/handler.hpp>
#include <osmium/io/pbf_input.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/visitor.hpp>

#include "argument_parser.hpp"
#include "taginfo_parser.hpp"

// read pbf file into handler
// iterate over nodes, ways, relations in handler
// check that each one is of a type in given taginfo
// first: store types that are not recognized
// later: somehow store types that are absent from input pbf
//      (does this need another field in taginfo_parser?)

using namespace taginfo_validate;

struct qa_handler : osmium::handler::Handler {
  qa_handler(const taginfo_parser &taginfo)
      : tags_on_areas(taginfo.tags_on_areas()), tags_on_nodes(taginfo.tags_on_nodes()),
        tags_on_ways(taginfo.tags_on_ways()), tags_on_relations(taginfo.tags_on_relations()),
        tags_on_any_object(taginfo.tags_on_any_object()){};

  using tag_range = std::pair<taginfo_parser::tag_iter, taginfo_parser::tag_iter>;

  // if a tag on the object isn't found in the taginfo.json, store in list of unknowns
  void storeIfUnknown(const tag_range &tagRange, const osmium::Tag &thePbfTag) {
    auto it = std::find_if(tagRange.first, tagRange.second,
                           [&](const taginfo_parser::tag &Tag) { return !(Tag.key.compare(thePbfTag.key())); });
    auto ait = std::find_if(tags_on_any_object.first, tags_on_any_object.second,
                            [&](const taginfo_parser::tag &Tag) { return !(Tag.key.compare(thePbfTag.key())); });
    if (it == tagRange.second && ait == tags_on_any_object.second)
      unknown_types.insert(
          taginfo_parser::tag{thePbfTag.key(), thePbfTag.value(), taginfo_parser::object::type::node});
  };

  void area(const osmium::Area &area) {
    for (const auto &pbfTag : area.tags()) {
      storeIfUnknown(tags_on_areas, pbfTag);
    }
  }

  void node(const osmium::Node &node) {
    for (const auto &pbfTag : node.tags()) {
      storeIfUnknown(tags_on_nodes, pbfTag);
    }
  }

  void way(const osmium::Way &way) {
    for (const auto &pbfTag : way.tags()) {
      storeIfUnknown(tags_on_ways, pbfTag);
    }
  }

  void relation(const osmium::Relation &rel) {
    for (const auto &pbfTag : rel.tags()) {
      storeIfUnknown(tags_on_relations, pbfTag);
    }
  }

  void printUnknowns() {
    for (const auto &unknown : unknown_types) {
      std::cout << unknown.key << "=" << unknown.value << std::endl;
    }
  }

  // member attributes
  tag_range tags_on_areas;
  tag_range tags_on_nodes;
  tag_range tags_on_ways;
  tag_range tags_on_relations;
  tag_range tags_on_any_object;

  std::unordered_set<taginfo_parser::tag, boost::hash<taginfo_parser::tag>> unknown_types;
};

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
