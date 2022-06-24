// FileSplicer is one file program that is designed to splice a file into smaller files of a specified size.
#include "FileSplicer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <ios>

#define RED "\e[0;31m"

void display_argument_error()
{
	std::cout << RED << "Incorrect usage of FileSplicer.\n"
				 " Please make sure that splicer is used in the following form: \"splicer [\"read\" OR \"write\"] [source_file_location] [destintation_directory]\""
			  << std::endl;
}
std::vector<char> read_all_bytes(char const* file_path)
{
	std::ifstream ifs(file_path, std::ifstream::binary | std::ifstream::ate);
	std::ifstream::pos_type pos = ifs.tellg();
	if (pos <= 0)
	{
		return std::vector<char>();
	}

	std::vector<char> result(pos);

	ifs.seekg(0, std::ifstream::beg);
	ifs.read(&result[0], pos);

	return result;
}

unsigned int parse_num_bytes_from_string(char const* byte_string)
{
	if (strcmp(byte_string, "") == 0)
	{
		return 0;
	}

	std::string number_string = "";
	std::string suffix_string = "";
	unsigned int current_index = 0;
	unsigned int result;
	while ((int)byte_string[current_index] >= 48 && (int)byte_string[current_index] <= 57)
	{
		number_string += byte_string[current_index];
		++current_index;
	}
	
	result = std::stoi(number_string);

	switch (byte_string[current_index])
	{
		case 'K': 
			result *= std::pow(2, 10);
			break;
		case 'M':
			result *= std::pow(2, 20);
			break;
		case 'G':
			result *= std::pow(2, 30);
			break;
		case 'T':
			result *= std::pow(2, 40);
			break;
	}
	
	return result;
}

//Usage:
//splicer ["combine" OR "split"] [source_file_location] [destintation_directory] [block_size]
int main(int argc, char* argv[], char* envp[])
{
	std::cout << "Splicer" << std::endl;

	if (argc != 4 && argc != 5) //There must three arguments... 
	{
		display_argument_error();
		return -1;
	}

	unsigned int chunk_size = 8 * std::pow(2, 20);

	if (argc == 5)
	{
		chunk_size = parse_num_bytes_from_string(argv[4]);
	}

	if (strcmp(argv[1], "split") == 0 && strcmp(argv[1], "combine") == 0) //The first argument must be 'read' or 'write.
	{
		display_argument_error();
		return -1;
	}

	if (strcmp(argv[1], "combine") == 0) //Do the work for reading already partitioned files: 
	{
		std::vector<char> byte_buffer = std::vector<char>();
		std::vector<char> part_buffer = std::vector<char>();
		std::string source_file_name = (std::string)argv[2] + ".part";
		unsigned int file_count = 0;
		do
		{
			file_count++;
			part_buffer = read_all_bytes((source_file_name + std::to_string(file_count)).c_str());
			if (part_buffer.size() > 0)
			{
				byte_buffer.insert(byte_buffer.end(), part_buffer.begin(), part_buffer.end());
				std::cout << "Processed file \"" << (source_file_name + std::to_string(file_count)).c_str() << "\"." << std::endl;
			}
		} while (part_buffer.size() > 0);


		std::ofstream result_file(argv[3], std::ios::binary);
		result_file.write(&byte_buffer[0], byte_buffer.size());
		result_file.close();
	}

	if (strcmp(argv[1], "split") == 0) //Do the work for partitioning and reforming the original file:
	{
		std::cout << "Start split path." << std::endl;
		char const* source_file = argv[2];
		std::string dest_file_name_template = argv[3];
		std::vector<char> source_bytes = read_all_bytes(source_file);
		unsigned int file_count = 1;
		std::cout << "Number of bytes in the file '" << source_file << "':" << source_bytes.size() << std::endl;

		if (source_bytes.size() <= 0)
		{
			std::cout << "The source file does not exist or does not contain any bytes." << std::endl;
			return -1;
		}
		for (unsigned int starting_byte = 0; starting_byte < source_bytes.size(); starting_byte += chunk_size)
		{
			std::cout << "Creating file: " << (dest_file_name_template + ".part" + std::to_string(file_count)).c_str() << std::endl;
			unsigned int current_chunk_size = source_bytes.size() - starting_byte < chunk_size ? source_bytes.size() - starting_byte : chunk_size;
			std::ofstream chunk_file((dest_file_name_template + ".part" + std::to_string(file_count)).c_str(), std::ios::binary);
			chunk_file.write(&source_bytes[0], current_chunk_size);
			chunk_file.close();
			file_count++;
		}	
	}

	std::cout << "Finished." << std::endl;
	return 0;
}
