#include "io/x3dom_writer.h"

#include "scale_axes.h"

#include "io/coordinate_frame.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

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
        case '\'':
            escaped += "&#39;";
            break;
        default:
            escaped += character;
            break;
        }
    }

    return escaped;
}

// An absolute path with "." and ".." resolved and no trailing separator.
[[nodiscard]] std::filesystem::path normalized_directory(const std::filesystem::path &directory) {
    std::error_code filesystem_error;
    std::filesystem::path absolute = std::filesystem::absolute(directory, filesystem_error);
    if (filesystem_error) {
        absolute = directory;
    }
    absolute = absolute.lexically_normal();
    if (!absolute.has_filename() && absolute.has_parent_path() && absolute != absolute.root_path()) {
        absolute = absolute.parent_path();   // "/a/b/" -> "/a/b"
    }
    return absolute;
}

// Percent-encodes what a URL path cannot hold as it is. Slashes are kept.
[[nodiscard]] std::string url_encode(const std::string &text) {
    std::string encoded;
    for (const unsigned char character : text) {
        if (std::isalnum(character) || character == '-' || character == '.' || character == '_' || character == '~' || character == '/') {
            encoded += static_cast<char>(character);
        } else {
            std::ostringstream hex;
            hex << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(character);
            encoded += hex.str();
        }
    }
    return encoded;
}

[[nodiscard]] bool is_glb(const std::filesystem::directory_entry &entry) {
    if (!entry.is_regular_file()) {
        return false;
    }
    std::string extension = entry.path().extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return std::tolower(c); });
    return extension == ".glb";
}

// The names of the .glb files directly in a directory, sorted.
[[nodiscard]] std::vector<std::string> glb_names(const std::filesystem::path &directory) {
    std::vector<std::string> names;
    std::error_code filesystem_error;
    for (std::filesystem::directory_iterator it(directory, filesystem_error), end; !filesystem_error && it != end; it.increment(filesystem_error)) {
        if (is_glb(*it)) {
            names.push_back(it->path().filename().string());
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

constexpr std::size_t maximum_viewer_models = 200;

// The reference the root MetadataSet cites: the document that describes the names.
constexpr const char *metadata_reference = "https://github.com/dwcaress/MB-System/blob/master/src/mbmesh/GEOREFERENCE.md";

// An X3D MFString: each string in double quotes, with \ and " escaped.
[[nodiscard]] std::string mf_string(const std::vector<std::string> &strings) {
    std::string result;
    for (const std::string &text : strings) {
        std::string quoted = "\"";
        for (const char character : text) {
            if (character == '"' || character == '\\') {
                quoted += '\\';
            }
            quoted += character;
        }
        quoted += '"';
        result += (result.empty() ? "" : " ") + quoted;
    }
    return result;
}

[[nodiscard]] bool all_of_type(const MetaValue &array, MetaValue::Type type) {
    for (const MetaValue &item : array.items) {
        if (item.type != type) {
            return false;
        }
    }
    return !array.items.empty();
}

// Writes a value as X3D metadata nodes: an object as a MetadataSet, a string as a
// MetadataString, a number as a MetadataDouble, a boolean as a MetadataBoolean,
// and an array of numbers or strings as one node holding all of them.
void write_metadata_node(std::ostream &out, const MetaValue &value, const std::string &name, int depth, const std::string &extra_attributes = "") {
    const std::string indent(static_cast<std::size_t>(depth) * 2, ' ');
    const std::string head = "name='" + escape_html(name) + "'" + extra_attributes;

    switch (value.type) {
    case MetaValue::Type::String:
        out << indent << "<MetadataString " << head << " value='" << escape_html(mf_string({value.text})) << "'></MetadataString>\n";
        break;
    case MetaValue::Type::Number:
        out << indent << "<MetadataDouble " << head << " value='" << metadata_number_text(value.number) << "'></MetadataDouble>\n";
        break;
    case MetaValue::Type::Bool:
        out << indent << "<MetadataBoolean " << head << " value='" << (value.flag ? "true" : "false") << "'></MetadataBoolean>\n";
        break;
    case MetaValue::Type::Array:
        if (all_of_type(value, MetaValue::Type::Number)) {
            std::string numbers;
            for (const MetaValue &item : value.items) {
                numbers += (numbers.empty() ? "" : " ") + metadata_number_text(item.number);
            }
            out << indent << "<MetadataDouble " << head << " value='" << numbers << "'></MetadataDouble>\n";
        } else if (all_of_type(value, MetaValue::Type::String)) {
            std::vector<std::string> strings;
            for (const MetaValue &item : value.items) {
                strings.push_back(item.text);
            }
            out << indent << "<MetadataString " << head << " value='" << escape_html(mf_string(strings)) << "'></MetadataString>\n";
        } else {
            out << indent << "<MetadataSet " << head << ">\n";
            for (std::size_t i = 0; i < value.items.size(); i++) {
                write_metadata_node(out, value.items[i], std::to_string(i), depth + 1);
            }
            out << indent << "</MetadataSet>\n";
        }
        break;
    case MetaValue::Type::Object:
        out << indent << "<MetadataSet " << head << ">\n";
        for (const auto &member : value.members) {
            if (!member.first.empty() && member.first[0] == '@') {
                continue;   // JSON-LD keywords are for the GLB; the page shows the values
            }
            write_metadata_node(out, member.second, member.first, depth + 1);
        }
        out << indent << "</MetadataSet>\n";
        break;
    }
}

// The style, the same as the page mbgrd2gltf writes.
constexpr const char *page_style = R"CSS(
      /* The scene fills the window. X3DOM sizes its canvas from the CSS size of the canvas, and sets the
         width and height attributes of the canvas, which give it an aspect ratio, from that. With a height
         that is a percentage of a parent, the canvas can end up 2:1 whatever the window is, and in Safari
         the size can grow until the drawing buffer is too large to allocate. A size in viewport units, which
         must win over the style that X3DOM writes, keeps it the size of the window. */
      html, body { overflow: hidden; }
      x3d { display: block; width: 100vw !important; height: 100vh !important; }
      x3d canvas { display: block; width: 100% !important; height: 100% !important; }
      .x3dom-logContainer { bottom: 0px; position: absolute; }
      #log-panel {
        position: fixed;
        bottom: 0;
        left: 0;
        right: 0;
        max-height: 30vh;
        background-color: rgba(240,240,240,0.95);
        border-top: 2px solid #666;
        display: none;
        flex-direction: column;
        z-index: 2000;
      }
      #log-panel.visible { display: flex; }
      #log-header {
        background-color: #666;
        color: white;
        padding: 8px 12px;
        font-weight: bold;
        display: flex;
        justify-content: space-between;
        align-items: center;
        cursor: pointer;
      }
      #log-content {
        flex: 1;
        overflow-y: auto;
        padding: 8px;
        font-family: 'Courier New', monospace;
        font-size: 12px;
      }
      .log-line {
        padding: 2px 0;
        white-space: pre-wrap;
        word-wrap: break-word;
      }
      #toggle-log-btn {
        position: fixed;
        bottom: 10px;
        left: 10px;
        background-color: rgba(102,102,102,0.9);
        color: white;
        border: none;
        padding: 10px 15px;
        border-radius: 4px;
        cursor: pointer;
        z-index: 1500;
        font-weight: bold;
      }
      #toggle-log-btn:hover { background-color: rgba(85,85,85,0.9); }
      #model-title {
        position: absolute;
        top: 0px;
        left: 50%;
        transform: translateX(-50%);
        background-color: rgba(199,202,204,.7);
        padding: 4px 20px;
        margin: 2px;
        margin-top: 2px;
        z-index: 1000;
        font-size: 16px;
        font-weight: bold;
        white-space: nowrap;
      }
)CSS";

constexpr const char *page_script = R"JS(
      // Show or hide a model. A model that was not shown at the start is loaded the
      // first time it is switched on, so large files cost nothing until they are wanted.
      function toggleModel(index, on) {
        var model = viewerModels[index];
        if (on && !model.loaded) {
          var inline = document.createElement('Inline');
          inline.setAttribute('id', 'inline_' + index);
          inline.setAttribute('url', model.uri);
          inline.setAttribute('nameSpaceName', 'model_' + index);
          inline.setAttribute('mapDEFToID', 'true');
          document.getElementById('transform_' + index).appendChild(inline);
          model.loaded = true;
        }
        document.getElementById('switch_' + index).setAttribute('whichChoice', on ? 0 : -1);
      }

      // The colors of a tileset's tiles, which have one for each vertex (COLOR_0) and a white base color. X3DOM's GeoTileset loads the
      // tiles itself, as nodes that are not in the page, so they are reached through the GeoTileset's own nodes: with the colors off, the
      // vertex colors are not used and the base color is the gray that MeshLab shows a mesh in. Each tileset has its own setting
      // (tilesetColors[index], on unless it says false). Tiles that are loaded later, as the view gets closer, are done as they appear.
      var tilesetColors = {};
      var tilesetGray = 0.75;   // 192 of 255, the gray that MeshLab shows a mesh in

      function collectShapes(node, shapes, seen) {
        if (!node || seen.indexOf(node) >= 0) {
          return;
        }
        seen.push(node);
        if (node.typeName && node.typeName() === 'Shape') {
          shapes.push(node);
          return;
        }
        var children = node._childNodes || [];
        for (var i = 0; i < children.length; i++) {
          collectShapes(children[i], shapes, seen);
        }
        var fields = node._cf || {};
        for (var name in fields) {
          if (fields[name] && fields[name].node) {
            collectShapes(fields[name].node, shapes, seen);
          }
        }
      }

      function applyTilesetColors(index) {
        var on = tilesetColors[index] !== false, shapes = [], changed = false;
        var tileset = document.getElementById('tileset_' + index);
        if (!tileset) {
          return;
        }
        var seen = [];
        var roots = tileset.querySelectorAll('GeoTiles');
        for (var r = 0; r < roots.length; r++) {
          collectShapes(roots[r]._x3domNode, shapes, seen);
        }
        if (roots.length === 0) {
          collectShapes(tileset._x3domNode, shapes, seen);
        }
        for (var s = 0; s < shapes.length; s++) {
          var shape = shapes[s], material = shape._cf && shape._cf.appearance && shape._cf.appearance.node &&
                                            shape._cf.appearance.node._cf.material && shape._cf.appearance.node._cf.material.node;
          var geometry = shape._cf && shape._cf.geometry && shape._cf.geometry.node;
          if (!material || !geometry || !material._vf || shape._colorsOn === on) {
            continue;
          }
          if (shape._colorsOn === undefined && geometry._hasColor !== true) {
            continue;   // a tile without vertex colors
          }
          var gray = on ? 1 : tilesetGray;
          material._vf.baseColorFactor = new x3dom.fields.SFColorRGBA(gray, gray, gray, 1);
          material.fieldChanged('baseColorFactor');
          geometry._hasColor = on;
          shape._dirty.shader = true;
          shape._dirty.colors = true;
          geometry.invalidateVolume();
          shape._colorsOn = on;
          changed = true;
        }
        if (changed) {
          var runtime = document.getElementById('x3dElement').runtime;
          if (runtime && runtime.canvas && runtime.canvas.doc) {
            runtime.canvas.doc.needRender = true;
          }
        }
      }

      function setTilesetColors(index, on) {
        tilesetColors[index] = on;
        applyTilesetColors(index);
      }

      // Tiles come in as the view changes: the ones that arrive while a tileset's colors are off are made gray.
      setInterval(function () {
        for (var index in tilesetColors) {
          if (tilesetColors[index] === false) {
            try { applyTilesetColors(index); } catch (e) { }
          }
        }
      }, 500);

      // Makes one of the page's viewpoints the one in use.
      function bindViewpoint(id) {
        var viewpoint = document.getElementById(id);
        if (viewpoint) {
          viewpoint.setAttribute('set_bind', 'true');
        }
      }

      // WGS 84 ECEF meters to [longitude, latitude, elevation], in degrees and meters.
      function ecefToGeodetic(x, y, z) {
        var f = 1 / 298.257223563, a = 6378137, e2 = f * (2 - f);
        var longitude = Math.atan2(y, x), r = Math.sqrt(x * x + y * y), latitude = Math.atan2(z, r * (1 - e2)), height = 0;
        for (var i = 0; i < 10; i++) {
          var n = a / Math.sqrt(1 - e2 * Math.sin(latitude) * Math.sin(latitude));
          height = r / Math.cos(latitude) - n;
          latitude = Math.atan2(z, r * (1 - e2 * n / (n + height)));
        }
        return [longitude * 180 / Math.PI, latitude * 180 / Math.PI, height];
      }

      // Says for a few seconds, under the button, what the Copy GeoViewpoint button did.
      var copyStatusTimer = null;
      function showCopyStatus(message) {
        var status = document.getElementById('copy-status');
        if (!status) {
          return;
        }
        status.textContent = message;
        status.style.visibility = 'visible';
        clearTimeout(copyStatusTimer);
        copyStatusTimer = setTimeout(function () { status.style.visibility = 'hidden'; }, 5000);
      }

      // Puts the --geoviewpoint option for the current view on the clipboard and in the X3DOM log. It is
      // geodetic, like a GeoViewpoint: longitude, latitude and elevation of the camera, its orientation
      // relative to the local frame there (as a GeoViewpoint takes it), and the center of rotation. It does
      // not depend on the GeoOrigin, the area or the grid, so it can be used in another run.
      function copyGeoViewpoint() {
        var runtime = document.getElementById('x3dElement').runtime;
        var matrix = runtime.viewMatrix().inverse();
        var axes = [matrix.e0(), matrix.e1(), matrix.e2()];
        var camera = matrix.e3();
        var center = null;
        try {
          center = runtime.canvas.doc._scene.getViewpoint().getCenterOfRotation();
        } catch (e) {
          center = null;
        }
        if (!center) {
          var box = runtime.getSceneBBox();
          center = { x: (box.min.x + box.max.x) / 2, y: (box.min.y + box.max.y) / 2, z: (box.min.z + box.max.z) / 2 };
        }
        var position = ecefToGeodetic(camera.x + modelOrigin[0], camera.y + modelOrigin[1], camera.z + modelOrigin[2]);
        var target = ecefToGeodetic(center.x + modelOrigin[0], center.y + modelOrigin[1], center.z + modelOrigin[2]);
        // A page with a lifted tileset shows everything liftMeters higher than it is: the option is for the true heights.
        position[2] -= liftMeters;
        target[2] -= liftMeters;

        // The local frame at the camera in the form X3DOM uses for a GeoViewpoint: X east, Y up, Z south.
        var phi = position[1] * Math.PI / 180, lambda = position[0] * Math.PI / 180;
        var east = [-Math.sin(lambda), Math.cos(lambda), 0];
        var north = [-Math.sin(phi) * Math.cos(lambda), -Math.sin(phi) * Math.sin(lambda), Math.cos(phi)];
        var up = [Math.cos(phi) * Math.cos(lambda), Math.cos(phi) * Math.sin(lambda), Math.sin(phi)];
        var frame = [east, up, [-north[0], -north[1], -north[2]]];
        var camera_axes = [[axes[0].x, axes[0].y, axes[0].z], [axes[1].x, axes[1].y, axes[1].z], [axes[2].x, axes[2].y, axes[2].z]];

        // The orientation is the frame's transpose times the camera's rotation, as an axis and an angle.
        var o = [[0, 0, 0], [0, 0, 0], [0, 0, 0]];
        for (var i = 0; i < 3; i++) {
          for (var j = 0; j < 3; j++) {
            for (var k = 0; k < 3; k++) {
              o[i][j] += frame[i][k] * camera_axes[j][k];
            }
          }
        }
        var cosine = Math.max(-1, Math.min(1, (o[0][0] + o[1][1] + o[2][2] - 1) / 2));
        var angle = Math.acos(cosine);
        var axis = [o[2][1] - o[1][2], o[0][2] - o[2][0], o[1][0] - o[0][1]];
        var length = Math.sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
        if (length < 1e-9) {
          axis = [0, 0, 1];
          angle = 0;
        } else {
          axis = [axis[0] / length, axis[1] / length, axis[2] / length];
        }

        var numbers = [position[0], position[1], position[2], axis[0], axis[1], axis[2], angle, target[0], target[1], target[2]];
        var digits = [7, 7, 3, 8, 8, 8, 8, 7, 7, 3];
        var text = '--geoviewpoint=' + numbers.map(function (value, index) { return value.toFixed(digits[index]); }).join(',');
        x3dom.debug.logInfo(text);
        var failed = function () {
          showCopyStatus('The clipboard is not available here; the option is in the X3DOM log');
          window.prompt('Copy the option:', text);
        };
        if (navigator.clipboard && window.isSecureContext) {
          navigator.clipboard.writeText(text).then(function () {
            showCopyStatus('The --geoviewpoint option was copied to the clipboard');
          }, failed);
        } else {
          failed();
        }
      }

      // Loads the log file, when the page has one instead of holding the log, the first time the panel is shown.
      var logLoaded = false;
      function loadLog() {
        if (logLoaded || !logUrl) {
          return;
        }
        logLoaded = true;
        var content = document.getElementById('log-content');
        var show = function (lines) {
          var count = 0;
          lines.forEach(function (line) {
            var div = document.createElement('div');
            div.className = 'log-line';
            div.textContent = line;
            content.appendChild(div);
            count++;
          });
          document.getElementById('log-title').textContent = 'Processing Log (' + count + ' lines, from ' + logUrl + ')';
        };
        fetch(logUrl).then(function (response) {
          if (!response.ok) {
            throw new Error(response.status + ' ' + response.statusText);
          }
          return response.text();
        }).then(function (text) {
          show(text.split('\n').filter(function (line) { return line.length > 0; }));
        }).catch(function (error) {
          logLoaded = false;
          show(['Could not load ' + logUrl + ': ' + error.message + ' (the page must be served over HTTP)']);
        });
      }
      function toggleLogPanel() {
        var panel = document.getElementById('log-panel');
        var btn = document.getElementById('toggle-log-btn');
        if (panel.classList.contains('visible')) {
          panel.classList.remove('visible');
          btn.textContent = 'Show Processing Log';
        } else {
          panel.classList.add('visible');
          btn.textContent = 'Hide Processing Log';
          loadLog();
        }
      }
)JS";

} // namespace

std::filesystem::path viewer_web_root(const std::filesystem::path &output_directory) {
    return normalized_directory(output_directory).parent_path();
}

std::filesystem::path viewer_page_path(const std::filesystem::path &output_directory) {
    const std::filesystem::path directory = normalized_directory(output_directory);
    return directory.parent_path() / (directory.filename().string() + ".html");
}

std::string viewer_title(const std::filesystem::path &web_root) {
    const std::filesystem::path directory = normalized_directory(web_root);
    std::vector<std::string> names;
    for (const std::filesystem::path &part : directory) {
        const std::string name = part.string();
        if (!name.empty() && name != "/" && name != "\\") {
            names.push_back(name);
        }
    }
    const std::size_t first = names.size() > 3 ? names.size() - 3 : 0;
    std::string title;
    for (std::size_t i = first; i < names.size(); i++) {
        title += (title.empty() ? "" : "/") + names[i];
    }
    return title;
}

std::filesystem::path viewer_inner_page_path(const std::filesystem::path &output_directory) {
    return normalized_directory(output_directory) / "mbmesh.html";
}

std::filesystem::path viewer_log_path(const std::filesystem::path &output_directory) {
    return normalized_directory(output_directory) / "mbmesh.log";
}

std::string viewer_log_url(const std::filesystem::path &output_directory) {
    return url_encode(normalized_directory(output_directory).filename().string() + "/mbmesh.log");
}

bool write_log_file(const std::filesystem::path &log_path, const std::vector<std::string> &messages, std::size_t omitted, std::string *error) {
    std::ofstream output(log_path);
    if (!output) {
        set_error(error, "Unable to create log file: " + log_path.string());
        return false;
    }
    for (const std::string &message : messages) {
        output << message << "\n";
    }
    if (omitted > 0) {
        output << "... " << omitted << " further messages are not shown\n";
    }
    output.close();
    if (!output) {
        set_error(error, "Error while writing log file: " + log_path.string());
        return false;
    }
    return true;
}

std::filesystem::path survey_page_path() {
    std::error_code filesystem_error;
    const std::filesystem::path current = std::filesystem::current_path(filesystem_error);
    return current / "mbmesh_survey.html";
}

static // The liftMeters that a tileset.json written by mbmesh or mbglb2tiles has in its extras; zero if it has none or the file cannot be read.
double read_tileset_lift(const std::filesystem::path &path) {
    std::ifstream in(path);
    if (!in) {
        return 0.0;
    }
    const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    const std::string key = "\"liftMeters\"";
    const std::size_t at = text.find(key);
    const std::size_t colon = at == std::string::npos ? std::string::npos : text.find(':', at + key.size());
    if (colon == std::string::npos) {
        return 0.0;
    }
    char *end = nullptr;
    const double value = std::strtod(text.c_str() + colon + 1, &end);
    return end != text.c_str() + colon + 1 && std::isfinite(value) ? value : 0.0;
}

static // Whether a tileset.json written by mbmesh or mbglb2tiles says in its extras that its tiles are colored by height.
bool read_tileset_height_colors(const std::filesystem::path &path) {
    std::ifstream in(path);
    const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return text.find("\"heightColors\"") != std::string::npos;
}

// The link to the right of a model's label that opens the log of the program that made it, or nothing.
static std::string log_link(const X3DomModel &model) {
    if (model.log_uri.empty()) {
        return std::string();
    }
    return " <a href='" + escape_html(model.log_uri) + "' target='_blank' style='font-size:smaller; margin-left:0.6em;'>" + escape_html(model.log_label) + "</a>";
}

std::vector<X3DomModel> find_viewer_models(const std::filesystem::path &web_root, const std::filesystem::path &output_directory, bool tileset,
                                           bool page_in_output_directory) {
    std::vector<X3DomModel> models;
    const std::string prefix = page_in_output_directory ? std::string() : normalized_directory(output_directory).filename().string() + "/";
    const std::filesystem::path root = normalized_directory(web_root);
    const std::filesystem::path directory = normalized_directory(output_directory);
    const std::string directory_name = directory.filename().string();

    // The tileset, if this run wrote one, is shown in place of mesh.glb, which is the same surface.
    if (tileset) {
        X3DomModel model;
        model.uri = url_encode(prefix + "tileset/tileset.json");
        model.label = "tileset (OGC 3D Tiles)";
        model.group = directory_name;
        model.visible = true;
        model.is_tileset = true;
        model.max_screen_space_error = 1.0;
        model.lift_meters = read_tileset_lift(output_directory / "tileset" / "tileset.json");
        model.height_colors = read_tileset_height_colors(output_directory / "tileset" / "tileset.json");
        models.push_back(model);
    }

    // The output directory: mesh.glb, then track.glb, then the rest by name.
    std::vector<std::string> names = glb_names(directory);
    std::stable_sort(names.begin(), names.end(), [](const std::string &a, const std::string &b) {
        const auto rank = [](const std::string &name) { return name == "mesh.glb" ? 0 : (name == "track.glb" ? 1 : 2); };
        return rank(a) < rank(b);
    });
    for (const std::string &name : names) {
        X3DomModel model;
        model.uri = url_encode(prefix + name);
        model.label = name;
        model.group = directory_name;
        model.visible = (name == "mesh.glb" && !tileset) || name == "track.glb";
        models.push_back(model);
    }

    // The web root: other GLBs, such as the product of mbgrd2gltf.
    if (!page_in_output_directory && directory.parent_path() == root) {
        for (const std::string &name : glb_names(root)) {
            X3DomModel model;
            model.uri = url_encode(name);
            model.label = name;
            model.group = "Other GLBs in this directory";
            model.group_note = "They line up with this run only if they use the same GeoOrigin.";
            model.visible = false;
            models.push_back(model);
        }
    }

    if (models.size() > maximum_viewer_models) {
        models.resize(maximum_viewer_models);
    }
    return models;
}

namespace {

// The number after "key": in a JSON text, searching from start; false if there is none.
bool json_number_after(const std::string &text, const std::string &key, std::size_t start, double *value, std::size_t *end = nullptr) {
    const std::size_t found = text.find("\"" + key + "\"", start);
    if (found == std::string::npos) {
        return false;
    }
    std::size_t at = text.find(':', found);
    if (at == std::string::npos) {
        return false;
    }
    const char *begin = text.c_str() + at + 1;
    char *stop = nullptr;
    const double number = std::strtod(begin, &stop);
    if (stop == begin || !std::isfinite(number)) {
        return false;
    }
    *value = number;
    if (end != nullptr) {
        *end = static_cast<std::size_t>(stop - text.c_str());
    }
    return true;
}

// The JSON chunk of a GLB, or empty.
std::string glb_json(const std::filesystem::path &path) {
    std::ifstream input(path, std::ios::binary);
    unsigned char header[20];
    if (!input.read(reinterpret_cast<char *>(header), sizeof(header))) {
        return std::string();
    }
    const std::uint32_t length = header[12] | (header[13] << 8) | (header[14] << 16) | (static_cast<std::uint32_t>(header[15]) << 24);
    if (length == 0 || length > 16u * 1024u * 1024u) {
        return std::string();
    }
    std::string json(length, '\0');
    if (!input.read(json.data(), static_cast<std::streamsize>(length))) {
        return std::string();
    }
    return json;
}

void geodetic_to_ecef_array(double longitude, double latitude, double elevation, double ecef[3]);

// What the asset.extras of a GLB that mbmesh wrote say about where it is.
struct GlbGeoreference {
    bool relative = false;       // vertices are relative to center, the ECEF position of the GeoOrigin
    double center[3] = {0.0, 0.0, 0.0};
    bool has_geodetic = false;   // geodeticCenter: longitude, latitude and height of the center
    double longitude = 0.0, latitude = 0.0, height = 0.0;
    bool has_extent = false;     // extent, in degrees
    double west = 0.0, east = 0.0, south = 0.0, north = 0.0;
    std::string where;           // the path of the GLB beneath the survey directory, for messages
};

// Reads the georeference block of a GLB. False if the GLB has none.
bool glb_georeference(const std::filesystem::path &path, GlbGeoreference *georeference) {
    const std::string json = glb_json(path);
    const std::size_t block = json.find("\"georeference\"");
    if (block == std::string::npos) {
        return false;
    }
    GlbGeoreference result;
    result.relative = json.find("\"relativeToCenter\"", block) != std::string::npos;
    const std::size_t array = json.find("\"center\"", block);
    const std::size_t at = array == std::string::npos ? std::string::npos : json.find('[', array);
    if (result.relative && at != std::string::npos) {
        const char *cursor = json.c_str() + at + 1;
        for (int i = 0; i < 3; i++) {
            char *stop = nullptr;
            result.center[i] = std::strtod(cursor, &stop);
            if (stop == cursor) {
                result.relative = false;
                break;
            }
            cursor = stop;
            while (*cursor == ',' || *cursor == ' ') {
                cursor++;
            }
        }
    } else {
        result.relative = false;
    }
    const std::size_t geodetic = json.find("\"geodeticCenter\"", block);
    result.has_geodetic = geodetic != std::string::npos && json_number_after(json, "longitude", geodetic, &result.longitude) &&
                          json_number_after(json, "latitude", geodetic, &result.latitude) && json_number_after(json, "height", geodetic, &result.height);
    const std::size_t extent = json.find("\"extent\"", block);
    result.has_extent = extent != std::string::npos && json_number_after(json, "west", extent, &result.west) &&
                        json_number_after(json, "east", extent, &result.east) && json_number_after(json, "south", extent, &result.south) &&
                        json_number_after(json, "north", extent, &result.north);
    if (result.relative && !result.has_geodetic) {
        result.relative = false;   // the GeoOrigin's geodetic position is needed for the page
    }
    if (!result.has_geodetic && !result.has_extent) {
        return false;
    }
    *georeference = result;
    return true;
}

} // namespace

std::vector<X3DomModel> find_survey_models(const std::filesystem::path &survey_directory, const std::filesystem::path &page_directory,
                                           bool *use_geoorigin, X3DomGeoOrigin *geo, std::vector<std::string> *geoorigin_mismatches) {
    constexpr std::size_t maximum_survey_models = 1000;

    const std::filesystem::path root = normalized_directory(survey_directory);
    const std::filesystem::path page = normalized_directory(page_directory);
    const std::string root_name = root.filename().string().empty() ? root.string() : root.filename().string();

    struct Found {
        std::vector<std::string> directories;   // the directory names beneath the survey directory
        std::string name;                       // the file name; for a tileset, the name of its directory
        std::filesystem::path path;
        bool is_tileset = false;
    };
    std::vector<Found> found;

    std::error_code filesystem_error;
    for (std::filesystem::recursive_directory_iterator it(root, filesystem_error), end; !filesystem_error && it != end; it.increment(filesystem_error)) {
        const std::filesystem::path &path = it->path();
        const std::string name = path.filename().string();
        if (it->is_directory(filesystem_error)) {
            // The chunk files repeat mesh.glb, and there can be hundreds ("tiles" is what the directory was called before it was
            // called "chunks", so that old output is still left out).
            if (name == "chunks" || name == "tiles" || (!name.empty() && name[0] == '.')) {
                it.disable_recursion_pending();
                continue;
            }
            // A directory with a tileset.json is a tileset, whatever it is called (tileset, from mbmesh --tileset, or mbglb2tiles_output):
            // it is offered as one tileset, and its tiles, which can be thousands of GLBs, are left out.
            if (std::filesystem::exists(path / "tileset.json", filesystem_error)) {
                it.disable_recursion_pending();
                Found entry;
                entry.is_tileset = true;
                entry.name = name;
                entry.path = path / "tileset.json";
                const std::filesystem::path relative = path.lexically_relative(root);
                for (const std::filesystem::path &part : relative.parent_path()) {
                    if (!part.empty() && part != ".") {
                        entry.directories.push_back(part.string());
                    }
                }
                found.push_back(entry);
            }
            continue;
        }
        if (!is_glb(*it)) {
            continue;
        }
        Found entry;
        const std::filesystem::path relative = path.lexically_relative(root);
        for (const std::filesystem::path &part : relative.parent_path()) {
            if (!part.empty() && part != ".") {
                entry.directories.push_back(part.string());
            }
        }
        entry.name = name;
        entry.path = path;
        found.push_back(entry);
    }

    // Directories in order of their names, component by component, so that a directory's accordion is
    // contiguous with its subdirectories'; within one, the tileset, then mesh.glb, then track.glb, then by name.
    const auto rank = [](const Found &f) { return f.is_tileset ? 0 : (f.name == "mesh.glb" ? 1 : (f.name == "track.glb" ? 2 : 3)); };
    std::stable_sort(found.begin(), found.end(), [&](const Found &a, const Found &b) {
        if (a.directories != b.directories) {
            return a.directories < b.directories;
        }
        if (rank(a) != rank(b)) {
            return rank(a) < rank(b);
        }
        return a.name < b.name;
    });
    std::vector<X3DomModel> models;
    std::vector<GlbGeoreference> references;
    for (const Found &entry : found) {
        if (models.size() >= maximum_survey_models) {
            break;
        }
        X3DomModel model;
        std::string group = root_name;
        for (const std::string &directory : entry.directories) {
            group += "/" + directory;
        }
        std::filesystem::path relative_to_page = entry.path.lexically_relative(page);
        if (relative_to_page.empty()) {
            relative_to_page = entry.path;
        }
        model.uri = url_encode(relative_to_page.generic_string());
        // A tileset is named by the directory that holds its tiles: tileset for mbmesh, whatever the user called it for mbglb2tiles.
        model.label = entry.is_tileset ? entry.name + " (OGC 3D Tiles)" : entry.name;
        model.group = group;
        model.is_tileset = entry.is_tileset;
        if (entry.is_tileset) {
            model.max_screen_space_error = 1.0;
            model.lift_meters = read_tileset_lift(entry.path);
            model.height_colors = read_tileset_height_colors(entry.path);
        }
        model.visible = !entry.is_tileset && entry.name == "track.glb";

        // The log of the program that made it, if it is there: mbglb2tiles writes mbglb2tiles.log into its tileset directory, mbmesh writes
        // mbmesh.log into its output directory (the parent of its tileset directory, and the directory of its mesh.glb and track.glb), and
        // mbgrd2gltf writes a .log file that has the name of its GLB beside it.
        {
            const std::filesystem::path directory = entry.path.parent_path();
            std::filesystem::path log;
            std::string log_label;
            std::error_code log_error;
            const auto exists = [&](const std::filesystem::path &candidate) { return std::filesystem::is_regular_file(candidate, log_error); };
            if (entry.is_tileset && exists(directory / "mbglb2tiles.log")) {
                log = directory / "mbglb2tiles.log";
                log_label = "mbglb2tiles log";
            } else if (entry.is_tileset && exists(directory.parent_path() / "mbmesh.log")) {
                log = directory.parent_path() / "mbmesh.log";
                log_label = "mbmesh log";
            } else if (!entry.is_tileset && (entry.name == "mesh.glb" || entry.name == "track.glb") && exists(directory / "mbmesh.log")) {
                log = directory / "mbmesh.log";
                log_label = "mbmesh log";
            } else if (!entry.is_tileset && exists(std::filesystem::path(entry.path).replace_extension(".log"))) {
                log = std::filesystem::path(entry.path).replace_extension(".log");
                log_label = "mbgrd2gltf log";
            }
            if (!log.empty()) {
                std::filesystem::path relative_log = log.lexically_relative(page);
                if (relative_log.empty()) {
                    relative_log = log;
                }
                model.log_uri = url_encode(relative_log.generic_string());
                model.log_label = log_label;
            }
        }
        models.push_back(model);

        if (!entry.is_tileset) {
            GlbGeoreference georeference;
            if (glb_georeference(entry.path, &georeference)) {
                georeference.where = entry.path.lexically_relative(root).generic_string();
                references.push_back(georeference);
            }
        }
    }

    // The initial view, as a run's page has: the middle of the data, framed to fill a 45 degree view. The data
    // are all the georeferenced GLBs, so the view takes in the whole survey. The GeoOrigin is that of the first
    // GLB with vertices relative to one.
    if (!references.empty()) {
        double west = 0.0, east = 0.0, south = 0.0, north = 0.0, height_sum = 0.0;
        std::size_t extents = 0, heights = 0;
        double lon_sum = 0.0, lat_sum = 0.0;
        const GlbGeoreference *origin = nullptr;
        for (const GlbGeoreference &reference : references) {
            if (origin == nullptr && reference.relative) {
                origin = &reference;
            }
            if (reference.has_extent) {
                west = extents == 0 ? reference.west : std::min(west, reference.west);
                east = extents == 0 ? reference.east : std::max(east, reference.east);
                south = extents == 0 ? reference.south : std::min(south, reference.south);
                north = extents == 0 ? reference.north : std::max(north, reference.north);
                extents++;
            }
            if (reference.has_geodetic) {
                height_sum += reference.height;
                lon_sum += reference.longitude;
                lat_sum += reference.latitude;
                heights++;
            }
        }
        if (extents == 0) {
            // No extent: the middle of the centers, over a small area.
            west = east = heights > 0 ? lon_sum / static_cast<double>(heights) : 0.0;
            south = north = heights > 0 ? lat_sum / static_cast<double>(heights) : 0.0;
        }
        constexpr double pi_value = 3.14159265358979323846;
        geo->center_lon = (west + east) / 2.0;
        geo->center_lat = (south + north) / 2.0;
        geo->mean_elevation = heights > 0 ? height_sum / static_cast<double>(heights) : 0.0;
        const double lat_extent_m = (north - south) * 111000.0;
        const double lon_extent_m = (east - west) * 111000.0 * std::cos(geo->center_lat * pi_value / 180.0);
        constexpr double field_of_view = 0.785398;
        geo->view_altitude = std::max(std::max(lat_extent_m, lon_extent_m), 100.0) / 2.0 / std::tan(field_of_view / 2.0);

        // Every GLB has to agree with the GeoOrigin GLB about where the GeoOrigin is, or about having none.
        if (geoorigin_mismatches != nullptr) {
            for (const GlbGeoreference &reference : references) {
                std::ostringstream message;
                message << std::fixed << std::setprecision(8);
                if (reference.relative && origin != nullptr) {
                    const double dx = reference.center[0] - origin->center[0];
                    const double dy = reference.center[1] - origin->center[1];
                    const double dz = reference.center[2] - origin->center[2];
                    if (std::sqrt(dx * dx + dy * dy + dz * dz) > 0.01) {
                        message << reference.where << " has the GeoOrigin " << reference.longitude << "," << reference.latitude << ","
                                << std::setprecision(2) << reference.height << ", but " << origin->where << " has " << std::setprecision(8)
                                << origin->longitude << "," << origin->latitude << "," << std::setprecision(2) << origin->height
                                << " (" << std::setprecision(1) << std::sqrt(dx * dx + dy * dy + dz * dz) << " m apart): they do not line up";
                        geoorigin_mismatches->push_back(message.str());
                    }
                } else if (!reference.relative && origin != nullptr) {
                    message << reference.where << " has absolute vertices, but " << origin->where
                            << " is relative to a GeoOrigin: they do not line up";
                    geoorigin_mismatches->push_back(message.str());
                }
            }
        }

        double origin_ecef[3] = {0.0, 0.0, 0.0};
        if (origin != nullptr) {
            *use_geoorigin = true;
            geo->origin_lon = origin->longitude;
            geo->origin_lat = origin->latitude;
            geo->origin_elevation = origin->height;
            for (int i = 0; i < 3; i++) {
                origin_ecef[i] = origin->center[i];
            }
        }
        double middle_ecef[3];
        geodetic_to_ecef_array(geo->center_lon, geo->center_lat, geo->mean_elevation, middle_ecef);
        set_local_viewpoint(geo, middle_ecef, origin_ecef);
    }
    return models;
}

namespace {

// n numbers from the array that follows the first occurrence of "key" at or after start, whatever brackets and commas are between them.
bool json_numbers_after(const std::string &text, const std::string &key, std::size_t start, int n, double *values) {
    const std::size_t found = text.find("\"" + key + "\"", start);
    const std::size_t array = found == std::string::npos ? std::string::npos : text.find('[', found);
    if (array == std::string::npos) {
        return false;
    }
    const char *cursor = text.c_str() + array;
    for (int i = 0; i < n; i++) {
        while (*cursor == '[' || *cursor == ']' || *cursor == ',' || *cursor == ' ' || *cursor == '\n') {
            cursor++;
        }
        char *stop = nullptr;
        values[i] = std::strtod(cursor, &stop);
        if (stop == cursor || !std::isfinite(values[i])) {
            return false;
        }
        cursor = stop;
    }
    return true;
}

} // namespace

std::vector<X3DomChunkLabel> find_survey_chunk_labels(const std::filesystem::path &survey_directory, bool use_geoorigin, const X3DomGeoOrigin &geo,
                                                      std::size_t *without_labels) {
    constexpr std::size_t maximum_labels = 5000;
    if (without_labels != nullptr) {
        *without_labels = 0;
    }
    double origin_ecef[3] = {0.0, 0.0, 0.0};
    if (use_geoorigin) {
        geodetic_to_ecef_array(geo.origin_lon, geo.origin_lat, geo.origin_elevation, origin_ecef);
    }
    // The chunk files, in order of path, so that the page does not depend on the order the file system lists them in.
    std::vector<std::filesystem::path> files;
    std::error_code filesystem_error;
    for (std::filesystem::recursive_directory_iterator it(normalized_directory(survey_directory), filesystem_error), end; !filesystem_error && it != end;
         it.increment(filesystem_error)) {
        const std::filesystem::path &path = it->path();
        const std::string name = path.filename().string();
        if (it->is_directory(filesystem_error)) {
            if (!name.empty() && name[0] == '.') {
                it.disable_recursion_pending();
            }
            continue;
        }
        if (path.parent_path().filename() == "chunks" && name.rfind("chunk_", 0) == 0 && is_glb(*it)) {
            files.push_back(path);
        }
    }
    std::sort(files.begin(), files.end());

    std::vector<X3DomChunkLabel> labels;
    for (const std::filesystem::path &path : files) {
        const std::string json = glb_json(path);
        const std::size_t at = json.find("\"chunkLabel\"");
        double corners[12], center[3], rise = 0.0, text_size = 1.0;
        if (at == std::string::npos || !json_numbers_after(json, "corners", at, 12, corners) || !json_numbers_after(json, "center", at, 3, center) ||
            !json_number_after(json, "rise", at, &rise) || !json_number_after(json, "textSize", at, &text_size)) {
            if (without_labels != nullptr) {
                (*without_labels)++;
            }
            continue;
        }
        if (labels.size() >= maximum_labels) {
            break;
        }
        X3DomChunkLabel label;
        label.name = path.filename().string();
        for (int corner = 0; corner < 4; corner++) {
            double ecef[3];
            geodetic_to_ecef_array(corners[3 * corner], corners[3 * corner + 1], corners[3 * corner + 2], ecef);
            for (int k = 0; k < 3; k++) {
                label.corners[corner][k] = ecef[k] - origin_ecef[k];
            }
        }
        double ecef[3];
        geodetic_to_ecef_array(center[0], center[1], center[2], ecef);
        for (int k = 0; k < 3; k++) {
            label.center[k] = ecef[k] - origin_ecef[k];
        }
        label.longitude = center[0];
        label.latitude = center[1];
        label.rise = rise;
        label.text_size = text_size;
        labels.push_back(label);
    }
    return labels;
}

namespace {

// The rotation with the given matrix, as an axis and an angle.
void matrix_to_axis_angle(const double r[3][3], double axis[3], double *angle) {
    const double trace = r[0][0] + r[1][1] + r[2][2];
    const double cosine = std::max(-1.0, std::min(1.0, (trace - 1.0) / 2.0));
    *angle = std::acos(cosine);
    double a[3] = {r[2][1] - r[1][2], r[0][2] - r[2][0], r[1][0] - r[0][1]};
    double length = std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    if (length < 1.0e-9) {
        // No rotation, or a half turn: keep the answer defined.
        if (cosine > 0.0) {
            axis[0] = 0.0;
            axis[1] = 0.0;
            axis[2] = 1.0;
            *angle = 0.0;
            return;
        }
        // A half turn about the axis with the largest diagonal term.
        int k = 0;
        for (int i = 1; i < 3; i++) {
            if (r[i][i] > r[k][k]) {
                k = i;
            }
        }
        double v[3];
        v[k] = std::sqrt(std::max(0.0, (r[k][k] + 1.0) / 2.0));
        for (int i = 0; i < 3; i++) {
            if (i != k) {
                v[i] = (v[k] > 0.0) ? (r[i][k] + r[k][i]) / (4.0 * v[k]) : 0.0;
            }
        }
        const double n = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        for (int i = 0; i < 3; i++) {
            axis[i] = n > 0.0 ? v[i] / n : (i == 2 ? 1.0 : 0.0);
        }
        return;
    }
    for (int i = 0; i < 3; i++) {
        axis[i] = a[i] / length;
    }
}

constexpr double pi = 3.14159265358979323846;

// The local east, north and up unit vectors in ECEF at a latitude and longitude in degrees.
void local_axes(double latitude, double longitude, double east[3], double north[3], double up[3]) {
    const double phi = latitude * pi / 180.0;
    const double lambda = longitude * pi / 180.0;
    east[0] = -std::sin(lambda);
    east[1] = std::cos(lambda);
    east[2] = 0.0;
    north[0] = -std::sin(phi) * std::cos(lambda);
    north[1] = -std::sin(phi) * std::sin(lambda);
    north[2] = std::cos(phi);
    up[0] = std::cos(phi) * std::cos(lambda);
    up[1] = std::cos(phi) * std::sin(lambda);
    up[2] = std::sin(phi);
}

// geodetic_to_ecef() from coordinate_frame.h, for the plain arrays used here.
void geodetic_to_ecef_array(double longitude, double latitude, double elevation, double ecef[3]) {
    GeodeticPosition position;
    position.longitude = longitude;
    position.latitude = latitude;
    position.elevation = elevation;
    const Vec3 converted = geodetic_to_ecef(position);
    ecef[0] = converted.x;
    ecef[1] = converted.y;
    ecef[2] = converted.z;
}

void normalize3(double v[3]) {
    const double length = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (length > 0.0) {
        for (int i = 0; i < 3; i++) {
            v[i] /= length;
        }
    }
}

void cross3(const double a[3], const double b[3], double out[3]) {
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}

} // namespace

bool parse_user_viewpoint(const std::string &text, X3DomUserViewpoint *viewpoint, std::string *error) {
    std::vector<double> numbers;
    std::size_t start = 0;
    while (true) {
        const std::size_t comma = text.find(',', start);
        const std::string field = text.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
        char *end = nullptr;
        const double value = std::strtod(field.c_str(), &end);
        if (field.empty() || end != field.c_str() + field.size() || !std::isfinite(value)) {
            if (error != nullptr) {
                *error = "viewpoint value '" + field + "' is not a number in '" + text + "'";
            }
            return false;
        }
        numbers.push_back(value);
        if (comma == std::string::npos) {
            break;
        }
        start = comma + 1;
    }

    if (numbers.size() != 6 && numbers.size() != 10) {
        if (error != nullptr) {
            *error = "a viewpoint takes 6 numbers, lon,lat,elev,clon,clat,celev, or 10, lon,lat,elev,ax,ay,az,angle,clon,clat,celev, "
                     "with no spaces; got " + std::to_string(numbers.size()) + " in '" + text + "'";
        }
        return false;
    }

    X3DomUserViewpoint result;
    result.longitude = numbers[0];
    result.latitude = numbers[1];
    result.elevation = numbers[2];
    result.center_longitude = numbers[numbers.size() - 3];
    result.center_latitude = numbers[numbers.size() - 2];
    result.center_elevation = numbers[numbers.size() - 1];
    if (std::fabs(result.latitude) > 90.0 || std::fabs(result.center_latitude) > 90.0) {
        if (error != nullptr) {
            *error = "a latitude of the viewpoint '" + text + "' is beyond 90 degrees; the order is lon,lat,elev";
        }
        return false;
    }
    if (numbers.size() == 10) {
        result.has_orientation = true;
        for (int i = 0; i < 3; i++) {
            result.axis[i] = numbers[3 + static_cast<std::size_t>(i)];
        }
        result.angle = numbers[6];
        if (result.axis[0] * result.axis[0] + result.axis[1] * result.axis[1] + result.axis[2] * result.axis[2] <= 0.0) {
            if (error != nullptr) {
                *error = "the orientation axis of the viewpoint '" + text + "' is zero";
            }
            return false;
        }
    }
    *viewpoint = result;
    return true;
}

bool geoviewpoint_orientation(const X3DomUserViewpoint &viewpoint, double axis[3], double *angle, std::string *error) {
    if (viewpoint.has_orientation) {
        for (int i = 0; i < 3; i++) {
            axis[i] = viewpoint.axis[i];
        }
        normalize3(axis);
        *angle = viewpoint.angle;
        return true;
    }

    double position[3], center[3];
    geodetic_to_ecef_array(viewpoint.longitude, viewpoint.latitude, viewpoint.elevation, position);
    geodetic_to_ecef_array(viewpoint.center_longitude, viewpoint.center_latitude, viewpoint.center_elevation, center);

    // The camera looks along its -Z with +Y at the top and X to the right: Z points back from the view.
    double back[3] = {position[0] - center[0], position[1] - center[1], position[2] - center[2]};
    if (back[0] * back[0] + back[1] * back[1] + back[2] * back[2] < 1.0e-12) {
        if (error != nullptr) {
            *error = "the viewpoint and its center of rotation are the same point";
        }
        return false;
    }
    normalize3(back);

    double east[3], north[3], up[3];
    local_axes(viewpoint.latitude, viewpoint.longitude, east, north, up);
    double right[3];
    cross3(up, back, right);
    if (right[0] * right[0] + right[1] * right[1] + right[2] * right[2] < 1.0e-12) {
        cross3(north, back, right);   // looking straight up or down
    }
    normalize3(right);
    double top[3];
    cross3(back, right, top);

    // X3DOM's local frame for a GeoViewpoint has X east, Y up and Z south (minus north). The orientation is
    // the rotation from that frame to the camera: the frame's transpose times the camera's axes.
    const double frame[3][3] = {{east[0], east[1], east[2]}, {up[0], up[1], up[2]}, {-north[0], -north[1], -north[2]}};
    const double camera[3][3] = {{right[0], top[0], back[0]}, {right[1], top[1], back[1]}, {right[2], top[2], back[2]}};
    double o[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            o[i][j] = frame[i][0] * camera[0][j] + frame[i][1] * camera[1][j] + frame[i][2] * camera[2][j];
        }
    }
    matrix_to_axis_angle(o, axis, angle);

    // The same rotation has two spellings, an axis and angle or the opposite axis and the opposite angle.
    // Choose the one whose largest axis component is positive, so that looking straight down is written
    // as 1 0 0 -1.5708, and write values that are zero to the digits written as zero.
    int largest = 0;
    for (int i = 1; i < 3; i++) {
        if (std::fabs(axis[i]) > std::fabs(axis[largest])) {
            largest = i;
        }
    }
    if (axis[largest] < 0.0) {
        for (int i = 0; i < 3; i++) {
            axis[i] = -axis[i];
        }
        *angle = -*angle;
    }
    for (int i = 0; i < 3; i++) {
        if (std::fabs(axis[i]) < 5.0e-9) {
            axis[i] = 0.0;
        }
    }
    return true;
}

void set_local_viewpoint(X3DomGeoOrigin *geo, const double center_ecef[3], const double origin_ecef[3]) {
    // The local east, north and up axes at the center, in ECEF.
    double east[3], north[3], up[3];
    local_axes(geo->center_lat, geo->center_lon, east, north, up);

    for (int i = 0; i < 3; i++) {
        geo->view_center[i] = center_ecef[i] - origin_ecef[i];
        geo->view_position[i] = geo->view_center[i] + up[i] * geo->view_altitude;
    }

    // A viewpoint looks along its -Z with +Y at the top. Making its X east, Y north and
    // Z up looks straight down with north at the top. The rotation that does that has
    // east, north and up as its columns; X3D wants it as an axis and an angle.
    const double r[3][3] = {{east[0], north[0], up[0]}, {east[1], north[1], up[1]}, {east[2], north[2], up[2]}};
    matrix_to_axis_angle(r, geo->view_axis, &geo->view_angle);
    geo->has_local_view = true;
}

bool write_glb_x3dom_file(const std::filesystem::path &html_path, const X3DomWriterOptions &options, std::string *error) {
    if (error != nullptr) {
        error->clear();
    }

    if (html_path.empty()) {
        set_error(error, "X3DOM output path is empty");
        return false;
    }

    if (options.models.empty()) {
        set_error(error, "X3DOM page has no models to show");
        return false;
    }

    std::ofstream output(html_path);
    if (!output) {
        set_error(error, "Unable to create X3DOM file: " + html_path.string());
        return false;
    }

    const std::string title = options.title.empty() ? options.models.front().uri : options.title;

    // The stylesheet is beside the script, in the same build of X3DOM.
    std::string x3dom_stylesheet_url = options.x3dom_url;
    {
        const std::size_t slash = x3dom_stylesheet_url.find_last_of('/');
        x3dom_stylesheet_url = (slash == std::string::npos ? std::string() : x3dom_stylesheet_url.substr(0, slash + 1)) + "x3dom.css";
    }

    // GeoViewpoint position and centre of rotation, as latitude, longitude, elevation
    // (the X3D geospatial order), looking straight down with north up.
    const X3DomGeoOrigin &geo = options.geo;
    // The Viewpoint menu: the two overview viewpoints and any the user supplied.
    std::string viewpoint_menu;
    if (geo.has_local_view || !options.user_viewpoints.empty()) {
        viewpoint_menu =
            "          <tr>\n"
            "            <td>Viewpoint:\n"
            "            </td>\n"
            "            <td align='right'>\n"
            "              <select style='float:right;' onchange='bindViewpoint(this.value)'>\n";
        if (geo.has_local_view) {
            viewpoint_menu +=
                "                <option value='OVERVIEW_LOCAL'>Viewpoint (model coordinates)\n                </option>\n"
                "                <option value='OVERVIEW_GEO'>GeoViewpoint (with GeoOrigin)\n                </option>\n";
        }
        for (std::size_t i = 0; i < options.user_viewpoints.size(); i++) {
            const std::string number = std::to_string(i + 1);
            viewpoint_menu += "                <option value='USER_GEOVIEW_" + number + "'>User GeoViewpoint " + number + "\n                </option>\n";
        }
        viewpoint_menu +=
            "              </select>\n"
            "            </td>\n"
            "          </tr>\n";
    }
    // The ECEF position of the GeoOrigin, which the models are relative to; zeros if they are absolute.
    double origin_ecef[3] = {0.0, 0.0, 0.0};
    if (options.use_geoorigin) {
        geodetic_to_ecef_array(geo.origin_lon, geo.origin_lat, geo.origin_elevation, origin_ecef);
    }
    // The page lifts what is not a tileset by the lift of the tileset, along the ellipsoid normal: the models with a GeoTransform, and
    // the viewpoints by adding it to their heights, so that the view stays where the tileset is.
    double lift = 0.0;
    for (const X3DomModel &model : options.models) {
        if (model.is_tileset) {
            lift = std::max(lift, model.lift_meters);
        }
    }
    std::ostringstream position, center, origin_coordinates;
    position << std::fixed << std::setprecision(6) << geo.center_lat << " " << geo.center_lon << " " << std::setprecision(1)
             << (geo.mean_elevation + geo.view_altitude + lift);
    center << std::fixed << std::setprecision(6) << geo.center_lat << " " << geo.center_lon << " " << std::setprecision(2)
           << (geo.mean_elevation + lift);
    std::ostringstream lift_center;   // where the lifting GeoTransforms are: only the latitude and longitude matter
    lift_center << std::fixed << std::setprecision(6) << geo.center_lat << " " << geo.center_lon << " 0";
    double local_lift[3] = {0.0, 0.0, 0.0};   // the lift along the local up at the center, in the models' coordinates
    if (lift != 0.0) {
        double east[3], north[3], up[3];
        local_axes(geo.center_lat, geo.center_lon, east, north, up);
        for (int i = 0; i < 3; i++) {
            local_lift[i] = up[i] * lift;
        }
    }
    origin_coordinates << std::fixed << std::setprecision(6) << geo.origin_lat << " " << geo.origin_lon << " " << std::setprecision(2)
                       << geo.origin_elevation;

    output << "<!DOCTYPE html>\n"
           << "<html style='width:100%; height:100%; border:0; margin:0; padding:0;'>\n"
           << "  <head>\n"
           << "    <meta http-equiv='X-UA-Compatible' content='chrome=1'></meta>\n"
           << "    <meta http-equiv='Content-Type' content='text/html;charset=utf-8'></meta>\n"
           << "    <title>" << escape_html(title) << "</title>\n"
           << "    <link type='text/css' href='" << escape_html(x3dom_stylesheet_url) << "' rel='stylesheet'>\n"
           << "    <script type='text/javascript' src='" << escape_html(options.x3dom_url) << "'></script>\n"
           << "    <style>" << page_style << "    </style>\n"
           << "  </head>\n"
           << "  <body style='width:100%; height:100%; border:0; margin:0; padding:0;'>\n"
           << "    <div id='HUDs_Div'>\n"
           << "      <div id='X3DOM_MBSystem_Links' class='group' style='margin:2px; margin-top:26px; padding:4px; "
              "background-color:rgba(199,202,204,.7); position:absolute; float:center; z-index:1000;'>\n"
           << "        <a href='https://www.x3dom.org'>X3DOM</a> output created with\n"
           << "        <a href='https://www.mbari.org/products/research-software/mb-system/'>MB-System</a> mbmesh.\n"
           << "      </div>\n"
           << "      <div id='model-title'>" << escape_html(title) << "</div>\n"
           << "      <div id='Interaction_Toolbox' style='margin:2px; padding:4px; "
              "background-color:rgba(199,202,204,.7);position:absolute; z-index:1000; right:0px; top:0px;'>\n"
           << "        <table>\n"
           << "          <tr>\n"
           << "            <td>Navigation Mode:\n"
           << "            </td>\n"
           << "            <td align='right'>\n"
           << "              <select style='float:right;' onchange='if (this.selectedIndex !== undefined) { "
              "var e = document.getElementById(&apos;x3dElement&apos;); "
              "if (this.options[this.selectedIndex].value === &apos;examine&apos;) { e.runtime.examine(); } "
              "else if (this.options[this.selectedIndex].value === &apos;lookat&apos;) { e.runtime.lookAt(); } "
              "else if (this.options[this.selectedIndex].value === &apos;walk&apos;) { e.runtime.walk(); } "
              "else if (this.options[this.selectedIndex].value === &apos;fly&apos;) { e.runtime.fly(); } "
              "else if (this.options[this.selectedIndex].value === &apos;helicopter&apos;) { e.runtime.helicopter(); } "
              "else if (this.options[this.selectedIndex].value === &apos;none&apos;) { e.runtime.noNav(); } }'>\n"
           << "                <option value='examine'>Examine\n                </option>\n"
           << "                <option value='lookat'>LookAt\n                </option>\n"
           << "                <option value='walk'>Walk\n                </option>\n"
           << "                <option value='fly'>Fly\n                </option>\n"
           << "                <option value='helicopter'>Helicopter\n                </option>\n"
           << "                <option value='none'>None\n                </option>\n"
           << "              </select>\n"
           << "            </td>\n"
           << "          </tr>\n"
           << viewpoint_menu
           << "          <tr>\n"
           << "            <td>Debug Display:\n"
           << "            </td>\n"
           << "            <td align='right'>\n"
           << "              <input type='checkbox' onclick='if (this.checked) { "
              "document.getElementById(&apos;x3dElement&apos;).runtime.statistics(true); } else { "
              "document.getElementById(&apos;x3dElement&apos;).runtime.statistics(false); }'>\n"
           << "              </input> Stats\n"
           << "              <input type='checkbox' onclick='document.getElementById(&apos;x3dElement&apos;).runtime.debug();'>\n"
           << "              </input> Log\n"
           << "            </td>\n"
           << "          </tr>\n";
    if (!options.chunk_labels.empty()) {
        output << "          <tr>\n"
               << "            <td>Chunks:\n"
               << "            </td>\n"
               << "            <td align='right'>\n"
               << "              <input type='checkbox' checked onclick='document.getElementById(&apos;switch_chunks&apos;)"
                  ".setAttribute(&apos;whichChoice&apos;, this.checked ? 0 : -1);'>\n"
               << "              </input> Outlines and names\n"
               << "            </td>\n"
               << "          </tr>\n";
    }

    // With nested groups: an accordion for each directory, inside the accordion of its parent, in one cell.
    if (options.nested_groups) {
        const auto split = [](const std::string &path) {
            std::vector<std::string> parts;
            std::size_t start = 0;
            while (start <= path.size()) {
                const std::size_t slash = path.find('/', start);
                parts.push_back(path.substr(start, slash == std::string::npos ? std::string::npos : slash - start));
                if (slash == std::string::npos) {
                    break;
                }
                start = slash + 1;
            }
            return parts;
        };
        output << "          <tr>\n"
               << "            <td colspan='2' style='max-width:22em; max-height:60vh; overflow:auto;'>\n";
        std::vector<std::string> open_groups;
        for (std::size_t i = 0; i < options.models.size(); i++) {
            const X3DomModel &model = options.models[i];
            const std::vector<std::string> parts = split(model.group);
            std::size_t common = 0;
            while (common < open_groups.size() && common < parts.size() && open_groups[common] == parts[common]) {
                common++;
            }
            while (open_groups.size() > common) {
                output << std::string(open_groups.size() * 2 + 12, ' ') << "</div></details>\n";
                open_groups.pop_back();
            }
            for (std::size_t level = common; level < parts.size(); level++) {
                // The survey directory itself starts open; the accordions inside it start closed.
                const bool open = level == 0;
                output << std::string(level * 2 + 12, ' ') << "<details class='group-accordion'" << (open ? " open" : "") << "><summary><b>"
                       << escape_html(parts[level]) << "</b></summary><div style='margin-left:1.2em;'>\n";
                open_groups.push_back(parts[level]);
            }
            output << std::string(open_groups.size() * 2 + 12, ' ') << "<div><input id='model-checkbox-" << i << "' type='checkbox'"
                   << (model.visible ? " checked" : "") << " onclick='toggleModel(" << i << ", this.checked);'></input> "
                   << escape_html(model.label) << log_link(model) << "</div>\n";
            if (model.height_colors) {
                output << std::string(open_groups.size() * 2 + 12, ' ') << "<div style='margin-left:1.5em;'><input id='color-checkbox-" << i
                       << "' type='checkbox' checked onclick='setTilesetColors(" << i << ", this.checked);'></input> color by height</div>\n";
            }
        }
        while (!open_groups.empty()) {
            output << std::string(open_groups.size() * 2 + 12, ' ') << "</div></details>\n";
            open_groups.pop_back();
        }
        output << "            </td>\n"
               << "          </tr>\n";
    }

    // A check box for each model, under a heading for each group.
    std::string current_group;
    for (std::size_t i = 0; i < options.models.size() && !options.nested_groups; i++) {
        const X3DomModel &model = options.models[i];
        if (i == 0 || model.group != current_group) {
            current_group = model.group;
            output << "          <tr>\n"
                   << "            <td colspan='2'><b>" << escape_html(current_group) << "</b></td>\n"
                   << "          </tr>\n";
            if (!model.group_note.empty()) {
                output << "          <tr>\n"
                       << "            <td colspan='2' style='max-width:18em; font-size:smaller;'>" << escape_html(model.group_note) << "</td>\n"
                       << "          </tr>\n";
            }
        }
        output << "          <tr>\n"
               << "            <td colspan='2'>\n"
               << "              <input id='model-checkbox-" << i << "' type='checkbox'" << (model.visible ? " checked" : "")
               << " onclick='toggleModel(" << i << ", this.checked);'>\n"
               << "              </input> " << escape_html(model.label) << log_link(model) << "\n"
               << "            </td>\n"
               << "          </tr>\n";
        if (model.height_colors) {
            output << "          <tr>\n"
                   << "            <td colspan='2' style='padding-left:1.5em;'>\n"
                   << "              <input id='color-checkbox-" << i << "' type='checkbox' checked onclick='setTilesetColors(" << i << ", this.checked);'>\n"
                   << "              </input> color by height\n"
                   << "            </td>\n"
                   << "          </tr>\n";
        }
    }

    // The three buttons are in one row, in one cell so that the table keeps its two columns, with the scale axes check box under them. The status message is
    // as wide as the buttons and wraps, and width:0 with min-width:100% keeps it from making the table wider.
    output << "          <tr>\n"
           << "            <td colspan='2'>\n"
           << "              <button onclick='document.getElementById(&apos;x3dElement&apos;).runtime.showAll();'> Show Everything\n"
           << "              </button>\n"
           << "              <button onclick='document.getElementById(&apos;x3dElement&apos;).runtime.resetView();'> Reset View\n"
           << "              </button>\n"
           << "              <button onclick='copyGeoViewpoint()' title='Puts the option that gives the current view on the clipboard and in the X3DOM log'> Copy GeoViewpoint\n"
           << "              </button>\n"
           << "              <div id='copy-status' style='visibility:hidden; font-size:smaller; width:0; min-width:100%; min-height:1.2em;'>&nbsp;</div>\n"
           << "            </td>\n"
           << "          </tr>\n";
    if (options.scale_axes) {
        scale_axes::write_checkbox_row(output);
    }
    output << "        </table>\n"
           << "      </div>\n"
           << "    </div>\n"
           << "    <x3d id='x3dElement' showStat='false' showLog='false' style='width:100%; height:100%; border:0; margin:0; padding:0;'>\n"
           << "      <scene DEF='scene'>\n";

    // Two overview viewpoints, which are meant to show the same view, so that each can be checked
    // against the other. The first is bound when the page opens. The Viewpoint is in the coordinates
    // of the models, ECEF minus the GeoOrigin's ECEF position. The GeoViewpoint gives the same place
    // as a latitude, longitude and elevation, and the GeoOrigin it is relative to is a child node:
    // geoOrigin is an SFNode field, so X3DOM takes it from a child element and ignores an attribute
    // that names a DEF, in which case the GeoViewpoint is placed in absolute ECEF, millions of meters
    // from models that are relative to the GeoOrigin. Page Down steps from one viewpoint to the other.
    // Each viewpoint tells the page when the camera moves, for the scale object.
    const std::string viewpoint_hook = options.scale_axes ? scale_axes::viewpoint_hook() : std::string();
    if (geo.has_local_view) {
        output << std::fixed << std::setprecision(3)
               << "        <Viewpoint DEF='OVERVIEW_LOCAL' id='OVERVIEW_LOCAL' description='Overview, Viewpoint in model coordinates' position='"
               << (geo.view_position[0] + local_lift[0]) << " " << (geo.view_position[1] + local_lift[1]) << " "
               << (geo.view_position[2] + local_lift[2]) << "' orientation='"
               << std::setprecision(8) << geo.view_axis[0] << " " << geo.view_axis[1] << " " << geo.view_axis[2] << " " << geo.view_angle
               << "' fieldOfView='0.785398' centerOfRotation='" << std::setprecision(3) << (geo.view_center[0] + local_lift[0]) << " "
               << (geo.view_center[1] + local_lift[1]) << " " << (geo.view_center[2] + local_lift[2]) << "'" << viewpoint_hook << "></Viewpoint>\n";
    }
    output << "        <GeoViewpoint DEF='OVERVIEW_GEO' id='OVERVIEW_GEO' description='Overview, GeoViewpoint with geoOrigin' position='"
           << position.str() << "' orientation='1 0 0 -1.5708' fieldOfView='0.785398' centerOfRotation='" << center.str() << "'"
           << viewpoint_hook << " geoSystem='\"GD\" \"WE\"'>";
    if (options.use_geoorigin) {
        output << "\n          <GeoOrigin DEF='GRID_ORIGIN' geoSystem='\"GD\" \"WE\"' geoCoords='" << origin_coordinates.str()
               << "'></GeoOrigin>\n        ";
    }
    output << "</GeoViewpoint>\n";

    // GeoViewpoints the user supplied. The GeoOrigin is the one defined in OVERVIEW_GEO, used again.
    for (std::size_t i = 0; i < options.user_viewpoints.size(); i++) {
        const X3DomUserViewpoint &user = options.user_viewpoints[i];
        double axis[3];
        double angle = 0.0;
        std::string viewpoint_error;
        if (!geoviewpoint_orientation(user, axis, &angle, &viewpoint_error)) {
            set_error(error, "geoviewpoint " + std::to_string(i + 1) + ": " + viewpoint_error);
            return false;
        }
        const std::string number = std::to_string(i + 1);
        output << std::fixed << "        <GeoViewpoint DEF='USER_GEOVIEW_" << number << "' id='USER_GEOVIEW_" << number
               << "' description='User GeoViewpoint " << number << "' position='" << std::setprecision(7) << user.latitude << " "
               << user.longitude << " " << std::setprecision(3) << (user.elevation + lift) << "' orientation='" << std::setprecision(8) << axis[0] << " "
               << axis[1] << " " << axis[2] << " " << angle << "' fieldOfView='0.785398' centerOfRotation='" << std::setprecision(7)
               << user.center_latitude << " " << user.center_longitude << " " << std::setprecision(3) << (user.center_elevation + lift)
               << "'" << viewpoint_hook << " geoSystem='\"GD\" \"WE\"'>";
        if (options.use_geoorigin) {
            output << "\n          <GeoOrigin USE='GRID_ORIGIN'></GeoOrigin>\n        ";
        }
        output << "</GeoViewpoint>\n";
    }

    // Documentation of the scene, from the same metadata that is in each GLB. The
    // geospatial facts go in the GeoMetadata summary, which takes only the keywords
    // X3D defines; everything else is a MetadataSet in its metadata field. WorldInfo
    // repeats the entries as readable lines, which X3DOM shows in its log.
    if (!options.metadata.members.empty()) {
        std::vector<std::string> lines;
        for (const auto &entry : flatten_metadata(options.metadata)) {
            lines.push_back(entry.first + ": " + entry.second);
        }
        output << "        <WorldInfo title='" << escape_html(title) << "' info='" << escape_html(mf_string(lines)) << "'></WorldInfo>\n"
               << "        <GeoMetadata DEF='GEO_METADATA' summary='" << escape_html(mf_string(options.geo_summary)) << "'>\n";
        write_metadata_node(output, options.metadata, "extras", 5, " containerField='metadata' reference='" + std::string(metadata_reference) + "'");
        output << "        </GeoMetadata>\n";
    }

    // X3DOM's GeoTransform has X east, Y up and Z south in the local frame at its geoCenter, so a translation
    // along Y is along the ellipsoid normal.
    std::ostringstream lift_text;
    lift_text << std::fixed << std::setprecision(3) << lift;
    const std::string lift_translation = lift_text.str();
    if (lift != 0.0) {
        output << "        <!-- The tileset is lifted " << lift_translation << " m above the ellipsoid in its tileset.json: every other model is in a GeoTransform\n"
               << "             that translates it up the same, and the viewpoints are that much higher. -->\n";
    }
    for (std::size_t i = 0; i < options.models.size(); i++) {
        const X3DomModel &model = options.models[i];
        const bool wrap_model = lift != 0.0 && !model.is_tileset;   // a tileset is lifted by its own file
        output << "        <switch id='switch_" << i << "' whichChoice='" << (model.visible ? 0 : -1) << "'>\n"
               << (wrap_model ? "          <GeoTransform id='transform_" + std::to_string(i) + "' geoSystem='\"GD\" \"WE\"' geoCenter='" +
                                          lift_center.str() + "' translation='0 " + lift_translation + " 0'>\n"
                                : "          <transform id='transform_" + std::to_string(i) + "'>\n");
        if (model.is_tileset) {
            // Written whether or not it is shown: chunks are only loaded while the switch shows it.
            output << "            <GeoTileset id='tileset_" << i << "'";
            if (model.max_screen_space_error > 0.0) {
                output << " maximumScreenSpaceError='" << metadata_number_text(model.max_screen_space_error) << "'";
            }
            output << " url='&quot;" << escape_html(model.uri) << "&quot;'>";
            if (options.use_geoorigin) {
                output << "\n              <GeoOrigin USE='GRID_ORIGIN'></GeoOrigin>\n            ";
            }
            output << "</GeoTileset>\n";
        } else if (model.visible) {
            output << "            <Inline id='inline_" << i << "' url='" << escape_html(model.uri) << "' nameSpaceName='model_" << i
                   << "' mapDEFToID='true'></Inline>\n";
        }
        output << (wrap_model ? "          </GeoTransform>\n" : "          </transform>\n") << "        </switch>\n";
    }

    // The outline and the file name of each chunk (--label-chunks), in the coordinates of the models and lifted with them.
    // The names always face the viewer, whatever the orientation of the page's axes.
    if (!options.chunk_labels.empty()) {
        output << std::fixed << std::setprecision(3);
        output << "        <switch id='switch_chunks' whichChoice='0'>\n"
               << (lift != 0.0 ? "          <GeoTransform id='transform_chunks' geoSystem='\"GD\" \"WE\"' geoCenter='" + lift_center.str() +
                                          "' translation='0 " + lift_translation + " 0'>\n"
                                : "          <transform id='transform_chunks'>\n")
               << "            <shape>\n"
               << "              <appearance><material emissiveColor='1 0.85 0'></material></appearance>\n"
               << "              <indexedLineSet coordIndex='";
        for (std::size_t n = 0; n < options.chunk_labels.size(); n++) {
            const std::size_t first = 4 * n;
            output << first << " " << first + 1 << " " << first + 2 << " " << first + 3 << " " << first << " -1" << (n + 1 < options.chunk_labels.size() ? " " : "");
        }
        output << "'>\n                <coordinate point='";
        for (std::size_t n = 0; n < options.chunk_labels.size(); n++) {
            for (int corner = 0; corner < 4; corner++) {
                const double *c = options.chunk_labels[n].corners[corner];
                output << c[0] << " " << c[1] << " " << c[2] << ((n + 1 < options.chunk_labels.size() || corner < 3) ? ", " : "");
            }
        }
        output << "'></coordinate>\n              </indexedLineSet>\n            </shape>\n";
        for (const X3DomChunkLabel &label : options.chunk_labels) {
            // Raised along the local vertical at the chunk, which X3DOM's GeoTransform has as Y, as the lift of the models is.
            if (label.rise != 0.0) {
                output << "            <GeoTransform geoSystem='&quot;GD&quot; &quot;WE&quot;' geoCenter='" << std::setprecision(7) << label.latitude << " "
                       << label.longitude << " 0' translation='0 " << std::setprecision(3) << label.rise << " 0'>\n";
            }
            output << "            <transform translation='" << label.center[0] << " " << label.center[1] << " " << label.center[2] << "'>\n"
                   << "              <billboard axisOfRotation='0 0 0'>\n"
                   << "                <shape>\n"
                   << "                  <appearance><material diffuseColor='0 0 0' emissiveColor='1 0.85 0'></material></appearance>\n"
                   << "                  <text string='&quot;" << escape_html(label.name) << "&quot;'>\n"
                   << "                    <fontStyle family='&quot;SANS&quot;' size='" << label.text_size << "' justify='&quot;MIDDLE&quot;'></fontStyle>\n"
                   << "                  </text>\n"
                   << "                </shape>\n"
                   << "              </billboard>\n"
                   << "            </transform>\n";
            if (label.rise != 0.0) {
                output << "            </GeoTransform>\n";
            }
        }
        output << (lift != 0.0 ? "          </GeoTransform>\n" : "          </transform>\n") << "        </switch>\n";
    }

    if (options.scale_axes) {
        scale_axes::write_scene(output);
    }
    output << "      </scene>\n"
           << "    </x3d>\n"
           << "    \n"
           << "    <button id='toggle-log-btn' onclick='toggleLogPanel()'>Show Processing Log</button>\n"
           << "    \n"
           << "    <div id='log-panel'>\n"
           << "      <div id='log-header' onclick='toggleLogPanel()'>\n"
           << "        <span id='log-title'>Processing Log"
           << (options.log_url.empty() ? " (" + std::to_string(options.log_messages.size() + options.log_messages_omitted) + " messages)" : std::string())
           << "</span>\n"
           << "        <span id='close-log'>&#10005;</span>\n"
           << "      </div>\n"
           << "      <div id='log-content'>\n";

    for (const std::string &message : options.log_url.empty() ? options.log_messages : std::vector<std::string>()) {
        output << "        <div class='log-line'>" << escape_html(message) << "</div>\n";
    }
    if (options.log_url.empty() && options.log_messages_omitted > 0) {
        output << "        <div class='log-line'>... " << options.log_messages_omitted << " further messages are not shown</div>\n";
    }

    output << "      </div>\n"
           << "    </div>\n"
           << "    \n";
    // A GeoTileset is an unknown node to a build of X3DOM that lacks it, which ignores it without a word and shows nothing: say so on the page.
    bool has_tileset = false;
    for (const X3DomModel &model : options.models) {
        has_tileset = has_tileset || model.is_tileset;
    }
    if (has_tileset) {
        output << "    <div id='geotileset-warning' style='display:none; position:absolute; bottom:20px; left:50%; transform:translateX(-50%); z-index:30; max-width:40em; "
               << "background:#fff3f3; color:#b00020; border:1px solid #b00020; border-radius:4px; padding:8px 12px; font-size:14px;'>\n"
               << "      The X3DOM script of this page, " << escape_html(options.x3dom_url) << ", has no GeoTileset node, so the tileset is not shown. "
               << "Use a build of X3DOM that has it (the <code>--x3dom-url</code> option gives the script of the page).\n"
               << "    </div>\n"
               << "    <script>\n"
               << "      if (!(window.x3dom && x3dom.nodeTypes && x3dom.nodeTypes.GeoTileset)) {\n"
               << "        document.getElementById('geotileset-warning').style.display = 'block';\n"
               << "        console.error('This X3DOM build has no GeoTileset node: the tileset is not shown. Use --x3dom-url.');\n"
               << "      }\n"
               << "    </script>\n";
    }
    output << "    <script>\n"
           << "      var logUrl = '" << (options.log_url.empty() ? std::string() : escape_html(options.log_url)) << "';\n"
           << "      var viewerModels = [\n";
    for (std::size_t i = 0; i < options.models.size(); i++) {
        output << "        { uri: '" << options.models[i].uri << "', loaded: " << (options.models[i].visible || options.models[i].is_tileset ? "true" : "false") << " }"
               << (i + 1 < options.models.size() ? "," : "") << "\n";
    }
    output << "      ];\n"
           << std::fixed << std::setprecision(3)
           << "      // The WGS 84 ECEF position of the GeoOrigin that the models are relative to (zeros if they are absolute).\n"
           << "      // How far above the ellipsoid the page lifts the tileset and the rest with it, in meters; zero if it does not.\n"
           << "      var liftMeters = " << std::fixed << std::setprecision(3) << lift << ";\n"
           << "      var modelOrigin = [" << origin_ecef[0] << ", " << origin_ecef[1] << ", " << origin_ecef[2] << "];\n"
           << page_script << (options.scale_axes ? scale_axes::script() : "") << "    </script>\n"
           << "  </body>\n"
           << "</html>\n";

    output.close();
    if (!output) {
        set_error(error, "Error while writing X3DOM file: " + html_path.string());
        return false;
    }

    return true;
}
