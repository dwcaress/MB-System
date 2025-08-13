/*--------------------------------------------------------------------
 *    The MB-system:	options.h	5/11/2023
 *
 *    Copyright (c) 2023-2024 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
 /*
  *    The program MBgrd2gltf, including this source file, was created
  *    by a Capstone Project team at the California State University
  *    Monterey Bay (CSUMB) including Kyle Dowling, Julian Fortin,
  *    Jesse Benavides, Nicolas Porras Falconio. This team was mentored by:
  *    Mike McCann
  *      Monterey Bay Aquarium Research Institute
  *      Moss Landing, California, USA
  *--------------------------------------------------------------------*/

#include "options.h"

#include <vector>
#include <stdexcept>
#include <iostream>
#include <stdio.h>

#if defined(_WIN32) || defined(WIN32) || defined(WIN64)
#define OS_IS_WINDOWS true
#else
#define OS_IS_WINDOWS false
#endif

const char* const usage_str = "usage: mbgrd2gltf <filepath> [-b | --binary] [(-o | --output) <output folder>]"\
"\n                              [(-e | --exaggeration) <vertical exaggeration>]"\
"\n                              [(-m | --max-size) <max size>]"\
"\n                              [(-s | --stride) <stride ratio>]"\
"\n                              [(-d | --draco) <draco compression>]"\
"\n                              [(-qp | -qn | -qt | -qc) <quantization>]";

const char* const help_str = "\nvariables:"\
"\n"\
"\n    <filepath>                path to the input GMT GRD file"\
"\n"\
"\n    <output folder>           path to folder where the output glTF file will be"\
"\n                              written"\
"\n"\
"\n    <vertical exaggeration>   decimal number representing the coefficient by"\
"\n                              which the vertex altitudes will be multiplied"\
"\n"\
"\n    <max size>                decimal number representing the max size of the"\
"\n                              output buffer data in MB (the actual size may vary"\
"\n                              depending on a number of factors including whether"\
"\n                              the output is in binary form or not)"\
"\n"\
"\n    <stride ratio>       	 decimal number representing the the amount"\
"\n                              of stride('Skip') to apply to the buffer data of the"\
"\n                              output as a ratio of normal to"\
"\n                              running. As you stride, you skip over portions of data."\
"\n                              The bigger the stride, the more points are skipped."\
"\n"\
"\n    <draco>                   reduces the file size of the output by using the"\
"\n                              Draco compression algorithm applied to the buffer data"\
"\n								 while encoding the mesh."\
"\n"\
"\n    <quantization>            Draco quantization settings. The quantization"\
"\n                              value must be between 2 and 30. The quantization"\
"\n                              settings are applied in the following order:"\
"\n                              -qp: Position, -qn: Normal, -qt: TexCoord, -qc: Color"\
"\n                              Lower quantization parameters yield higher compression,"\
"\n                              but lower visual quality. The default quantization"\
"\n                              value is 16 for position, 7 for normal, 10 for texcoord,"\
"\n                              and 8 for color."\
"\n";

const char* const try_help_str = "try 'mbgrd2gltf [-h | --help]' for more information";

#if OS_IS_WINDOWS
const char dir_delim = '\\';
#else
const char dir_delim = '/';
#endif

namespace mbgrd2gltf {
	struct PathInfo {
		std::string folder;
		std::string file_basename;
	};

	const std::unordered_map<std::string, Options::ArgCallback> Options::arg_callbacks =
	{
		{ "-b", &Options::arg_binary },
		{ "--binary", &Options::arg_binary },
		{ "-e", &Options::arg_exaggeration },
		{ "--exaggeration", &Options::arg_exaggeration },
		{ "-s", &Options::arg_stride },
		{ "--stride", &Options::arg_stride },
		{ "-m", &Options::arg_max_size },
		{ "--max-size", &Options::arg_max_size },
		{ "-o", &Options::arg_output },
		{ "--output", &Options::arg_output },
		{ "-d", &Options::arg_draco_compression},
		{ "--draco", &Options::arg_draco_compression},
		{ "-qp", &Options::arg_draco_quantization},
		{ "-qn", &Options::arg_draco_quantization},
		{ "-qt", &Options::arg_draco_quantization},
		{ "-qc", &Options::arg_draco_quantization},
		{ "--quantiposition", &Options::arg_draco_quantization},
		{ "--quantinormal", &Options::arg_draco_quantization},
		{ "--quantitexcoord", &Options::arg_draco_quantization},
		{ "--quanticolor", &Options::arg_draco_quantization}
	};

	double parse_value(const char* token, const char* var_name) {
		double value;

		int return_value = sscanf(token, "%lf", &value);

		if (return_value == EOF || return_value == 0)
			throw std::invalid_argument("invalid "
				+ std::string(var_name)
				+ ": "
				+ std::string(token));

		return value;
	}

	const char* get_value_token(const char** args, unsigned size, unsigned& i, const char* var_name) {
		if (i + 1 >= size)
			throw std::invalid_argument(std::string(var_name)
				+ " is required after '"
				+ std::string(args[i])
				+ "'");

		return args[++i];
	}

	double get_value_double(const char** args, unsigned size, unsigned& i, const char* var_name) {
		const char* token = get_value_token(args, size, i, var_name);
		double value = parse_value(token, var_name);

		return value;
	}

	void Options::arg_output(const char** args, unsigned size, unsigned& i) {
		const char* token = get_value_token(args, size, i, "output folder");

		_output_filepath = token;
	}

	void Options::arg_binary(const char**, unsigned, unsigned&) {
		if (_is_binary_output)
			throw std::invalid_argument("binary output may not be specified more than once");

		_is_binary_output = true;
	}

	void Options::arg_stride(const char** args, unsigned size, unsigned& i) {
		if (_is_stride_set)
			throw std::invalid_argument("stride ratio may not be set more than once");

		if (_is_max_size_set > 0)
			throw std::invalid_argument("stride ratio may not be set when max size is set");

		double value = get_value_double(args, size, i, "stride ratio");

		if (value < 1.0)
			throw std::invalid_argument("expected stride ratio >= 1 but got: "
				+ std::to_string(value));

		_stride_ratio = value;
		_is_stride_set = true;
	}

	void Options::arg_max_size(const char** args, unsigned size, unsigned& i) {
		if (_is_max_size_set)
			throw std::invalid_argument("max size may not be specified more than once");

		if (_is_stride_set)
			throw std::invalid_argument("max size may not be set when compression ratio is set");


		double value = get_value_double(args, size, i, "max size");
		if (value < 0.0001)
			throw std::invalid_argument("expected max size > 0.001 but got: "
				+ std::to_string(value)
				+ "MB");

		_max_size = (size_t)(value * 1000000.0);
		_is_max_size_set = true;
	}

	void Options::arg_draco_compression(const char**, unsigned, unsigned&) {
		if (_is_draco_compressed)
			throw std::invalid_argument("draco compression may not be specified more than once");
		_is_draco_compressed = true;
	}


	void Options::arg_draco_quantization(const char** args, unsigned size, unsigned& i) {
		if (!_is_draco_compressed)
			throw std::invalid_argument("Quantization settings can only be specified if Draco compression is enabled");

		std::string arg(args[i]);
		std::string quant_type = arg.substr(0, 3);
		int value;

		if (arg.size() > 3) {
			std::string value_str = arg.substr(3);
			value = std::stoi(value_str);
		}
		else {
			if (i + 1 >= size) {
				throw std::invalid_argument("Quantization value is required after '" + quant_type + "'");
			}
			value = std::stoi(args[++i]);
		}

		if (value <= 0 || value > 30)
			throw std::invalid_argument("Quantization must be between 1 and 30");

		// Apply the quantization setting based on the type
		if (quant_type == "-qp") _draco_quantization[0] = value;
		else if (quant_type == "-qn") _draco_quantization[1] = value;
		else if (quant_type == "-qt") _draco_quantization[2] = value;
		else if (quant_type == "-qc") _draco_quantization[3] = value;
		else throw std::invalid_argument("Invalid quantization type specified: " + quant_type);
	}


	bool Options::draco_quantization_valid() const {
		for (int i = 0; i < 4; i++)
			if (_draco_quantization[i] <= 0 || _draco_quantization[i] > 30)
				return false;
		return true;
	}

	void Options::arg_exaggeration(const char** args, unsigned size, unsigned& i) {
		if (_is_exaggeration_set)
			throw std::invalid_argument("vertical exaggeration may not be specified more than once");

		double value = get_value_double(args, size, i, "vertical exaggeration");

		if (value <= 0.0)
			throw std::invalid_argument("expectd vertical exaggeration > 0 but got:"
				+ std::to_string(value));

		_exaggeration = value;
		_is_exaggeration_set = true;
	}

	PathInfo get_path_info(const char* filepath) {
		const char* start_of_filename = filepath;

		for (const char* iter = filepath; *iter; ++iter) {
#if OS_IS_WINDOW
			bool is_dir_char = *iter == '/' || *iter == '\\';
#else
			bool is_dir_char = *iter == '/';
#endif

			if (is_dir_char)
				start_of_filename = iter + 1;
		}

		const char* start_of_extension = start_of_filename;

		for (const char* iter = start_of_filename; *iter; ++iter) {
			if (*iter == '.')
				break;

			start_of_extension += 1;
		}

		std::string file_basename = std::string(start_of_filename, start_of_extension - start_of_filename);
		std::string folder = std::string(filepath, start_of_filename - filepath);

		if (file_basename.empty())
			file_basename = "output";

		return {folder, file_basename};
	}

	void print_help() {
		std::cout << usage_str << std::endl << help_str << std::endl;
	}

	Options::Options(unsigned argc, const char* argv[]) {
		if (argc < 2) {
			print_help();
			_is_help = true;
			return;
		}
		_input_filepath = argv[1];

		if (_input_filepath == "-h" || _input_filepath == "--help") {
			print_help();
			_is_help = true;
			return;
		}

		auto path_info = get_path_info(argv[1]);
		_output_filepath = path_info.folder;
		unsigned args_size = argc - 2;
		const char** args = argv + 2;

		for (unsigned i = 0; i < args_size; ++i) {
			const char* arg = args[i];
			auto callback_iter = arg_callbacks.find(arg);

			if (callback_iter == arg_callbacks.end())
				throw std::invalid_argument("unexpected token '"
					+ std::string(arg)
					+ "'\n"
					+ std::string(usage_str)
					+ "\n\n"
					+ std::string(try_help_str));

			auto callback = callback_iter->second;

			(this->*callback)(args, args_size, i);
		}

		if (_output_filepath.empty()) {
			_output_filepath = ".";
			_output_filepath += dir_delim;
		}
		else {
			if (_output_filepath.back() != dir_delim)
				_output_filepath += dir_delim;
		}

		_output_filepath += path_info.file_basename;
	}
}
