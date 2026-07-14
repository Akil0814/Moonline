#define SDL_MAIN_HANDLED

#include "engine/resources/atlas/atlas_build_preparer.h"
#include "tests/support/test_assertions.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace
{
using moonline::tests::require;

void test_named_and_legacy_frame_expansion()
{
	const std::filesystem::path atlas_test_root =
		std::filesystem::temp_directory_path() / "moonline_atlas_build_preparer_tests";
	std::filesystem::remove_all(atlas_test_root);
	std::filesystem::create_directories(atlas_test_root / "named");
	std::filesystem::create_directories(atlas_test_root / "legacy");
	std::ofstream(atlas_test_root / "named" / "RyougiShiki_idle_000.png").put('\0');
	std::ofstream(atlas_test_root / "named" / "RyougiShiki_idle_001.png").put('\0');
	std::ofstream(atlas_test_root / "legacy" / "frame_002.png").put('\0');
	std::ofstream(atlas_test_root / "legacy" / "frame_001.png").put('\0');

	elysia::resources::AtlasBuildPreparer atlas_build_preparer;
	std::vector<elysia::resources::AtlasFramePrepareTask> atlas_tasks;
	elysia::resources::AtlasBuildRequest named_request;
	named_request.atlas_key = "ryougi_shiki.idle";
	named_request.directory_path = atlas_test_root / "named";
	named_request.frame_count = 2;
	named_request.frame_filename_prefix = "RyougiShiki_idle";
	require(atlas_build_preparer.expand_build_request(named_request, atlas_tasks),
		"named atlas frames must be inferred from the request prefix");
	require(atlas_tasks.size() == 2
		&& atlas_tasks[0].frame_path.filename() == "RyougiShiki_idle_000.png"
		&& atlas_tasks[1].frame_path.filename() == "RyougiShiki_idle_001.png",
		"named atlas frames must retain generated numeric order");
	named_request.frame_count = 3;
	require(!atlas_build_preparer.expand_build_request(named_request, atlas_tasks),
		"named atlas loading must fail when an inferred frame is missing");

	elysia::resources::AtlasBuildRequest legacy_request;
	legacy_request.atlas_key = "legacy";
	legacy_request.directory_path = atlas_test_root / "legacy";
	legacy_request.frame_count = 2;
	require(atlas_build_preparer.expand_build_request(legacy_request, atlas_tasks),
		"legacy atlas loading must retain directory-scan compatibility");
	require(atlas_tasks[0].frame_path.filename() == "frame_001.png"
		&& atlas_tasks[1].frame_path.filename() == "frame_002.png",
		"legacy atlas frames must remain filename-sorted");
	std::filesystem::remove_all(atlas_test_root);
}
}

int main()
{
    test_named_and_legacy_frame_expansion();
    std::cout << "atlas build preparer tests passed\n";
    return EXIT_SUCCESS;
}

