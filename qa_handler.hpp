#ifndef TAGINFO_VALIDATE_QA_HANDLER
#define TAGINFO_VALIDATE_QA_HANDLER

#include <osmium/handler.hpp>
#include <osmium/osm/types.hpp>

#include "tag.hpp"
#include "taginfo_parser.hpp"

// the handler checks if osm objects of any type are present in a given taginfo file

using namespace taginfo_validate;

struct qa_handler : osmium::handler::Handler {
  qa_handler(const taginfo_parser &taginfo)
      : tags_on_areas(taginfo.tags_on_areas()), tags_on_nodes(taginfo.tags_on_nodes()),
        tags_on_ways(taginfo.tags_on_ways()), tags_on_relations(taginfo.tags_on_relations()),
        tags_on_any_object(taginfo.tags_on_any_object()){};

  using tag_range = std::pair<taginfo_parser::tag_iter, taginfo_parser::tag_iter>;

  // if pbf tag's key is not found in taginfo list, store tag to list of unrecognized tags
  void storeIfUnknown(const tag_range &tagRange, const osmium::Tag &thePbfTag) {
    auto it = std::find_if(tagRange.first, tagRange.second,
                           [&](const tag &Tag) { return !(Tag.key.compare(thePbfTag.key())); });
    auto ait = std::find_if(tags_on_any_object.first, tags_on_any_object.second,
                            [&](const tag &Tag) { return !(Tag.key.compare(thePbfTag.key())); });
    if (it == tagRange.second && ait == tags_on_any_object.second) {
      unknown_types.insert(
          tag{thePbfTag.key(), thePbfTag.value(), object::type::node});
    } else if (it->value.empty()) {
    // if matching tag in taginfo also specifies a value, check that the pbf
    // tag's value is also recognized
        if (thePbfTag.value() != it->value) {
          unknown_types.insert(
              tag{thePbfTag.key(), thePbfTag.value(), object::type::node});
        }
    }
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

  std::unordered_set<tag, boost::hash<tag>> unknown_types;
};


#endif
