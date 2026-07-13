#include "../../tools/logger.h"
#include "atlas_build_preparer.h"

#include "../texture/surface_loader.h"

#include <algorithm>
#include <cctype>
namespace elysia::resources
{
namespace
{
bool is_png_file(const std::filesystem::path& file_path)
{
	std::string extension = file_path.extension().string();
	for (char& character : extension)
	{
		character = static_cast<char>(
			std::tolower(static_cast<unsigned char>(character))
		);
	}

	return extension == ".png";
}
}

bool AtlasBuildPreparer::expand_build_request(
	const AtlasBuildRequest& request,
	std::vector<AtlasFramePrepareTask>& out_tasks
) const
{
	out_tasks.clear();

	if (!request.is_valid())
	{
		ELYSIA_LOG_WARN("resource","Expand atlas build request failed: request is invalid: "
			<< request.atlas_key);
		return false;
	}

	if (!std::filesystem::is_directory(request.directory_path))
	{
		ELYSIA_LOG_WARN("resource","Expand atlas build request failed: directory does not exist: "
			<< request.directory_path);
		return false;
	}

	std::vector<std::filesystem::path> frame_paths;
	for (const std::filesystem::directory_entry& entry :
		std::filesystem::directory_iterator(request.directory_path))
	{
		if (!entry.is_regular_file())
			continue;

		if (!is_png_file(entry.path()))
			continue;

		frame_paths.push_back(entry.path());
	}

	std::sort(
		frame_paths.begin(),
		frame_paths.end(),
		[](const std::filesystem::path& lhs, const std::filesystem::path& rhs)
		{
			return lhs.filename().string() < rhs.filename().string();
		}
	);

	if (frame_paths.size() != request.frame_count)
	{
		ELYSIA_LOG_WARN("resource","Expand atlas build request failed: frame count mismatch: "
			<< request.atlas_key << ", expected " << request.frame_count
			<< ", actual " << frame_paths.size());
		return false;
	}

	out_tasks.reserve(frame_paths.size());
	for (size_t index = 0; index < frame_paths.size(); ++index)
	{
		AtlasFramePrepareTask task;
		task.atlas_key = request.atlas_key;
		task.frame_path = frame_paths[index];
		task.frame_index = index;
		task.expected_frame_count = request.frame_count;
		out_tasks.push_back(std::move(task));
	}

	return true;
}

AtlasFramePreparedResult AtlasBuildPreparer::prepare_frame(
	const AtlasFramePrepareTask& task
) const
{
	AtlasFramePreparedResult result;
	result.task = task;

	if (task.atlas_key.empty())
	{
		ELYSIA_LOG_WARN("resource","Prepare atlas frame failed: atlas key is empty.");
		return result;
	}

	if (task.frame_path.empty())
	{
		ELYSIA_LOG_WARN("resource","Prepare atlas frame failed: frame path is empty: "
			<< task.atlas_key);
		return result;
	}

	if (task.expected_frame_count == 0)
	{
		ELYSIA_LOG_WARN("resource","Prepare atlas frame failed: expected frame count is zero: "
			<< task.atlas_key);
		return result;
	}

	SurfaceLoadRequest surface_request;
	surface_request._asset_key = task.atlas_key;
	surface_request._frame_path = task.frame_path;
	surface_request._frame_index = task.frame_index;

	SurfaceLoader surface_loader;
	result.surface_result = surface_loader.load_surface(surface_request);
	return result;
}

}
