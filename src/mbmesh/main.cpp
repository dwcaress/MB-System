#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include "bathymetry.h"
#include "geometry_3d.h"
#include "gltf_writer.h"

namespace fs = std::filesystem;

// Find next available filename by incrementing version number
std::string find_next_available_filename(const std::string &output_file) {
	// Check if file exists
	if (!fs::exists(output_file)) {
		return output_file;
	}

	// Get just the filename without path
	fs::path file_path(output_file);
	std::string filename = file_path.filename().string();
	fs::path directory = file_path.parent_path();

	// Parse filename to extract base name and version
	// Format assumed: test-mbgrid2.0.glb
	// Match: (base)(digits).(digits).glb, where base ends right before first digit
	std::regex version_regex("^(.+?)(\\d+)\\.(\\d+)\\.glb$");
	std::smatch match;

	if (std::regex_match(filename, match, version_regex)) {
		std::string base = match[1].str();     // e.g., "test-mbgrid"
		int major = std::stoi(match[2].str()); // e.g., 2
		std::string minor = match[3].str();    // e.g., "0"

		// Increment major version until we find available filename
		major++;
		while (true) {
			std::string new_filename = base + std::to_string(major) + "." + minor + ".glb";
			fs::path new_path = directory / new_filename;
			if (!fs::exists(new_path)) {
				std::cout << "INFO: Output file exists, using: " << new_path.string() << std::endl;
				return new_path.string();
			}
			major++;
		}
	}

	// If regex doesn't match, just return original
	return output_file;
}

int main(int argc, char *argv[]) {
	std::string input_file = "";
	std::string output_file = "";

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		if (arg == "--input" || arg == "-i") {
			input_file = argv[++i];
		}
		else if (arg == "--output" || arg == "-o") {
			output_file = argv[++i];
		}
	}

	if (input_file.empty() || output_file.empty()) {
		std::cerr << "Usage: " << argv[0] << " --input <file> --output <file>" << std::endl;
		return 1;
	}

	// Find next available output filename if it exists
	output_file = find_next_available_filename(output_file);

	try {
		std::cout << "INFO: Loading bathymetry grid..." << std::endl;
		Bathymetry bathymetry(input_file);
		std::cout << "INFO: Loaded grid: " << bathymetry.width() << " x " << bathymetry.height() << " points" << std::endl;

		std::cout << "INFO: Generating 3D geometry..." << std::endl;
		Geometry3D geometry(bathymetry);

		std::cout << "INFO: Extracting 3D coordinates..." << std::endl;
		std::vector<float> vertices_3d = geometry.get_vertices_3d();
		std::vector<uint32_t> indices = geometry.get_indices();

		std::cout << "INFO: Extracted " << vertices_3d.size() / 3 << " vertices" << std::endl;
		std::cout << "INFO: Extracted " << indices.size() / 3 << " triangles" << std::endl;

		std::cout << "INFO: Writing glTF file..." << std::endl;
		GLTFWriter writer;
		writer.write_gltf(output_file, vertices_3d, indices, geometry.get_geo_origin());

		std::cout << "INFO: Successfully wrote glTF file to " << output_file << std::endl;
	} catch (const std::exception &e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
