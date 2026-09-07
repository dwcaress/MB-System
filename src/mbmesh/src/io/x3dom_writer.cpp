#include "io/x3dom_writer.h"

#include <fstream>
#include <string>

namespace {

void set_error(std::string *error, const std::string &message) {
    if (error != nullptr) {
        *error = message;
    }
}

[[nodiscard]] std::string escape_html(const std::string &text) {
    std::string escaped;
    escaped.reserve(text.size());

    for (const char character : text) {
        switch (character) {
        case '&':
            escaped += "&amp;";
            break;
        case '<':
            escaped += "&lt;";
            break;
        case '>':
            escaped += "&gt;";
            break;
        case '"':
            escaped += "&quot;";
            break;
        default:
            escaped += character;
            break;
        }
    }

    return escaped;
}

} // namespace

bool write_glb_x3dom_file(const std::filesystem::path &html_path, const std::string &glb_uri, const X3DomWriterOptions &options, std::string *error) {
    if (error != nullptr) {
        error->clear();
    }

    if (html_path.empty()) {
        set_error(error, "X3DOM output path is empty");
        return false;
    }

    if (glb_uri.empty()) {
        set_error(error, "X3DOM GLB URI is empty");
        return false;
    }

    std::ofstream output(html_path);
    if (!output) {
        set_error(error, "Unable to create X3DOM file: " + html_path.string());
        return false;
    }

    output
        << "<!doctype html>\n"
        << "<html lang=\"en\">\n"
        << "<head>\n"
        << "  <meta charset=\"utf-8\">\n"
        << "  <meta name=\"viewport\" "
           "content=\"width=device-width, initial-scale=1\">\n"
        << "  <title>" << escape_html(options.title) << "</title>\n"
        << "  <script src=\"https://www.x3dom.org/download/x3dom.js\">"
           "</script>\n"
        << "  <link rel=\"stylesheet\" "
           "href=\"https://www.x3dom.org/download/x3dom.css\">\n"
        << "  <style>\n"
        << "    html, body { margin: 0; width: 100%; height: 100%; "
           "overflow: hidden; }\n"
        << "    .viewer-name { position: fixed; left: 16px; top: 12px; "
           "z-index: 1; margin: 0; padding: 6px 10px; font: 600 14px "
           "system-ui, sans-serif; background: rgba(255,255,255,0.86); "
           "color: #1f2933; border: 1px solid rgba(31,41,51,0.18); }\n"
        << "    x3d { display: block; width: 100vw; height: 100vh; "
           "background: #f6f7f8; }\n"
        << "  </style>\n"
        << "</head>\n"
        << "<body>\n"
        << "  <h1 class=\"viewer-name\">" << escape_html(options.title)
        << "</h1>\n"
        << "  <x3d>\n"
        << "    <scene>\n"
        << "      <navigationInfo type='\"EXAMINE\" \"ANY\"'>"
           "</navigationInfo>\n"
        << "      <inline url=\"" << escape_html(glb_uri)
        << "\"></inline>\n"
        << "    </scene>\n"
        << "  </x3d>\n"
        << "</body>\n"
        << "</html>\n";

    output.close();
    if (!output) {
        set_error(
            error,
            "Error while writing X3DOM file: " + html_path.string());
        return false;
    }

    return true;
}
