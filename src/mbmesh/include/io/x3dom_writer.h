#pragma once

#include <filesystem>
#include <string>

struct X3DomWriterOptions {
    // Text displayed as the generated HTML document title.
    std::string title = "MB-System mbmesh Mesh Viewer";
};

[[nodiscard]] bool write_glb_x3dom_file(
    const std::filesystem::path &html_path,
    const std::string &glb_uri,
    const X3DomWriterOptions &options,
    std::string *error);
