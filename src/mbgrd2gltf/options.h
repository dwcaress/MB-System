#ifndef STOQS_OPTIONS_H
#define STOQS_OPTIONS_H

// standard library
#include <string>
#include <unordered_map>

namespace stoqs
{	
	class Options
	{
	public: // types

		typedef void (Options::*ArgCallback)(const char **args, unsigned size, unsigned& i);

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

		static const std::unordered_map<std::string, ArgCallback> arg_callbacks;

	private: // methods

		void arg_binary(const char **args, unsigned size, unsigned& i);
		void arg_output(const char **args, unsigned size, unsigned& i);
		void arg_compression(const char **args, unsigned size, unsigned& i);
		void arg_max_size(const char **args, unsigned size, unsigned& i);
		void arg_exaggeration(const char **args, unsigned size, unsigned& i);

	public: // members

		Options(unsigned argc, const char **argv);

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
	};
}

#endif
