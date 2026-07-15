#include "engine/config/config_service.h"
#include "tests/support/test_assertions.h"

#include <filesystem>
#include <fstream>

namespace
{
using moonline::tests::require;
void write(const std::filesystem::path& path,std::string_view text) { std::ofstream(path,std::ios::trunc) << text; }
void manifest(const std::filesystem::path& dir,std::string_view configs)
{ write(dir/"manifest.json",std::string("{\"schema_version\":1,\"configs\":")+std::string(configs)+"}"); }
}

int main()
{
    const auto dir = std::filesystem::temp_directory_path() / "moonline_config_service_tests";
    std::filesystem::remove_all(dir); std::filesystem::create_directories(dir);
    write(dir/"game.json",R"({
      "count":7,"ratio":2.5,"enabled":true,"name":"hero",
      "position":{"x":1,"y":2},"bounds":{"x":1,"y":2,"width":3,"height":4},
      "ints":[1,2,3],"points":[{"x":1,"y":2},{"x":3,"y":4}],"empty":[]
    })");
    manifest(dir,R"({"game":"game.json"})");
    auto* service = elysia::config::ConfigService::instance(); service->shutdown();
    require(service->initialize(dir/"manifest.json").has_value(),"generic ConfigService must initialize arbitrary namespace");
    require(service->contains("game.position") && service->contains("game.ints.1"),"objects and array elements must be indexed");
    require(service->get_int("game.count").value() == 7,"integer getter must work");
    require(service->get_double("game.count").value() == 7.0 && service->get_double("game.ratio").value() == 2.5,"double getter must accept integer and float");
    require(service->get_bool("game.enabled").value() && service->get_string("game.name").value()=="hero","bool and string getters must work");
    require(service->get_vector2("game.position").value() == elysia::core::Vector2{1,2},"Vector2 getter must use strict shape");
    require(service->get_rect("game.bounds").value() == elysia::core::Rect{1,2,3,4},"Rect getter must use strict shape");
    require(service->get_int_array("game.ints").value().size()==3 && service->get_vector2_array("game.points").value().size()==2,"typed arrays must work");
    require(service->get_string_array("game.empty").value().empty(),"empty typed array must be valid");
    require(!service->get_int("game.ratio") && !service->get_rect("game.position") && !service->get_int("game.missing"),"type mismatch and missing key must return expected errors");

    write(dir/"bad.json",R"({"bad-name":1})"); manifest(dir,R"({"bad":"bad.json"})");
    require(!service->initialize(dir/"manifest.json"),"illegal key component must fail initialization");
    require(service->get_int("game.count").value()==7,"failed reinitialize must preserve previous snapshot");
    write(dir/"overlap.json",R"({"value":1})"); write(dir/"base.json",R"({"rules":{"value":2}})");
    manifest(dir,R"({"game":"base.json","game.rules":"overlap.json"})");
    auto duplicate = service->initialize(dir/"manifest.json");
    require(!duplicate && duplicate.error().error == elysia::config::ConfigLoadError::DuplicateKey,"overlapping namespace must report duplicate full key");
    require(!duplicate.error().first.config_path.empty() && !duplicate.error().second.config_path.empty(),"duplicate failure must retain both origins");
    write(dir/"null.json",R"({"value":null})"); manifest(dir,R"({"nulls":"null.json"})");
    require(!service->initialize(dir/"manifest.json"),"JSON null must be rejected during initialization");
    write(dir/"duplicate.json",R"({"value":1,"value":2})"); manifest(dir,R"({"duplicates":"duplicate.json"})");
    auto duplicate_property = service->initialize(dir/"manifest.json");
    require(!duplicate_property && duplicate_property.error().error == elysia::config::ConfigLoadError::DuplicateKey
        && !duplicate_property.error().first.config_path.empty() && !duplicate_property.error().second.config_path.empty(),
        "duplicate JSON properties must report two source origins");
    service->shutdown(); require(!service->is_initialized() && !service->get_int("game.count"),"shutdown must clear snapshot and make access fail");
    std::filesystem::remove_all(dir);
}
