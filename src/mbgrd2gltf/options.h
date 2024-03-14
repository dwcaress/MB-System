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

#ifndef OPTIONS_H
#define OPTIONS_H

  // standard library
#include <string>
#include <unordered_map>

namespace mbgrd2gltf {
	class Options {
	public: // types

		typedef void (Options::* ArgCallback)(const char** args, unsigned size, unsigned& i);

	private: // members

		std::string _input_filepath;
		std::string _output_filepath;
		double _compression_ratio = 1.0;
		size_t _max_size = 0;
		double _exaggeration = 1.0;
		bool _is_binary_output = false;
		bool _is_help = false;
		bool _is_compression_set = false;
		bool _is_max_size_set = false;
		bool _is_exaggeration_set = false;
		bool _is_output_folder_set = false;
		bool _is_draco_compressed = false;
		int _draco_quantization = 11;

		static const std::unordered_map<std::string, ArgCallback> arg_callbacks;

	private: // methods

		void arg_binary(const char** args, unsigned size, unsigned& i);
		void arg_output(const char** args, unsigned size, unsigned& i);
		void arg_compression(const char** args, unsigned size, unsigned& i);
		void arg_max_size(const char** args, unsigned size, unsigned& i);
		void arg_exaggeration(const char** args, unsigned size, unsigned& i);
		void arg_draco_compression(const char** args, unsigned size, unsigned& i);
		void arg_draco_quantization(const char** args, unsigned size, unsigned& i);

	public: // members

		Options(unsigned argc, const char** argv);

		const std::string& input_filepath() const { return _input_filepath; }
		const std::string& output_filepath() const { return _output_filepath; }
		double compression_ratio() const { return _compression_ratio; }
		size_t max_size() const { return _max_size; }
		double exaggeration() const { return _exaggeration; }
		bool is_binary_output() const { return _is_binary_output; }
		bool is_help() const { return _is_help; }
		bool is_compression_set() const { return _is_compression_set; }
		bool is_max_size_set() const { return _is_max_size_set; }
		bool is_exaggeration_set() const { return _is_exaggeration_set; }
		bool is_output_folder_set() const { return _is_output_folder_set; }
		bool is_draco_compressed() const { return _is_draco_compressed; }
		int draco_quantization() const { return _draco_quantization; }
	};
}

#endif
