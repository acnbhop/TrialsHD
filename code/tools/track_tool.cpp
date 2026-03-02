//
// track_tool.cpp
//

// Game headers
#include "game/track.hpp"

// Standard headers
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <cctype>

void PrintUsage(const char* ProgramName)
{
	std::cout << "=== THD Track Tool ===\n";
	std::cout << "Usage:\n";
	std::cout << "  " << ProgramName << " <input.trk>                 Unpacks the track to a .xml file in the same directory\n";
	std::cout << "  " << ProgramName << " <input.xml>                 Packs the XML file back into a .trk file\n";
	std::cout << "  " << ProgramName << " --print <input.trk>         Prints the track summary to a .txt file\n";
	std::cout << "  " << ProgramName << " --print-console <input.trk> Prints the track summary to the console\n";

	std::cout << "\nBatch Processing Commands:\n";
	std::cout << "  " << ProgramName << " --export <folder>           Exports all .trk files in folder to .xml files\n";
	std::cout << "  " << ProgramName << " --print-dump <folder>       Dumps summaries for all .trk files to .txt files\n";
	std::cout << "\nOptions:\n";
	std::cout << "  --output <folder>                    Output to a separate folder (mirrors directory structure)\n";
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		PrintUsage(argv[0]);
		return EXIT_FAILURE;
	}

	bool PrintToFile = false;
	bool PrintToConsole = false;
	bool BatchExport = false;
	bool BatchDump = false;
	std::string TargetPath = "";
	std::string OutputDir = "";

	// Parse arguments
	for (int i = 1; i < argc; ++i)
	{
		std::string Arg = argv[i];
		if (Arg == "--print")
		{
			PrintToFile = true;
		}
		else if (Arg == "--print-console")
		{
			PrintToConsole = true;
		}
		else if (Arg == "--export")
		{
			BatchExport = true;
		}
		else if (Arg == "--print-dump")
		{
			BatchDump = true;
		}
		else if (Arg == "--output")
		{
			if (i + 1 < argc)
			{
				OutputDir = argv[++i];
			}
			else
			{
				std::cerr << "[-] --output requires a folder argument.\n";
				return EXIT_FAILURE;
			}
		}
		else
		{
			TargetPath = Arg;
		}
	}

	if (TargetPath.empty())
	{
		std::cerr << "[-] No target file or folder specified.\n";
		PrintUsage(argv[0]);
		return EXIT_FAILURE;
	}

	//
	// BATCH FOLDER PROCESSING
	//

	if (BatchExport || BatchDump)
	{
		if (!std::filesystem::exists(TargetPath) || !std::filesystem::is_directory(TargetPath))
		{
			std::cerr << "[-] Error: Specified target is not a valid directory: " << TargetPath << "\n";
			return EXIT_FAILURE;
		}

		std::cout << "[i] Starting batch processing in folder: " << TargetPath << "\n";
		if (!OutputDir.empty())
		{
			std::cout << "[i] Output directory: " << OutputDir << "\n";
		}

		int ProcessedCount = 0;

		for (const auto& Entry : std::filesystem::recursive_directory_iterator(TargetPath))
		{
			if (!Entry.is_regular_file()) continue;

			std::filesystem::path FilePath = Entry.path();
			std::string Ext = FilePath.extension().string();
			std::transform(Ext.begin(), Ext.end(), Ext.begin(), [](unsigned char c) { return std::tolower(c); });

			if (Ext == ".trk")
			{
				std::filesystem::path RelPath = std::filesystem::relative(FilePath, TargetPath);
				std::cout << "  -> Loading: " << RelPath.string() << " ... ";

				redlynx::game::Track TrackData;
				if (!TrackData.Load(FilePath.string()))
				{
					std::cout << "[FAILED TO LOAD]\n";
					continue;
				}

				if (BatchExport)
				{
					std::filesystem::path OutPath;
					if (!OutputDir.empty())
					{
						OutPath = std::filesystem::path(OutputDir) / RelPath;
						OutPath.replace_extension(".xml");
						std::filesystem::create_directories(OutPath.parent_path());
					}
					else
					{
						OutPath = FilePath;
						OutPath.replace_extension(".xml");
					}
					if (TrackData.ExportXML(OutPath.string()))
					{
						std::cout << "[XML EXPORTED] ";
					}
					else
					{
						std::cout << "[XML FAILED] ";
					}
				}

				if (BatchDump)
				{
					std::filesystem::path OutPath;
					if (!OutputDir.empty())
					{
						OutPath = std::filesystem::path(OutputDir) / RelPath;
						OutPath.replace_extension(".txt");
						std::filesystem::create_directories(OutPath.parent_path());
					}
					else
					{
						OutPath = FilePath;
						OutPath.replace_extension(".txt");
					}
					std::ofstream OutFile(OutPath);
					if (OutFile)
					{
						std::streambuf* OldCoutBuf = std::cout.rdbuf();
						std::cout.rdbuf(OutFile.rdbuf());
						TrackData.PrintSummary();
						std::cout.rdbuf(OldCoutBuf);

						std::cout << "[TXT DUMPED]";
					}
					else
					{
						std::cout << "[TXT FAILED]";
					}
				}

				std::cout << "\n";
				ProcessedCount++;
			}
		}

		std::cout << "[+] Batch processing complete. Processed " << ProcessedCount << " files.\n";
		return EXIT_SUCCESS;
	}

	//
	// SINGLE FILE PROCESSING
	//

	std::filesystem::path InputPath(TargetPath);
	std::string Ext = InputPath.extension().string();
	std::transform(Ext.begin(), Ext.end(), Ext.begin(), [](unsigned char c){ return std::tolower(c); });

	redlynx::game::Track TrackData;

	if (Ext == ".trk")
	{
		if (PrintToFile || PrintToConsole)
		{
			std::cout << "[i] Loading track for inspection: " << TargetPath << "\n";
			if (!TrackData.Load(TargetPath))
			{
				return EXIT_FAILURE;
			}

			if (PrintToFile)
			{
				std::filesystem::path OutputPath = InputPath;
				OutputPath.replace_extension(".txt");

				std::ofstream OutFile(OutputPath);
				if (OutFile)
				{
					std::streambuf* OldCoutBuf = std::cout.rdbuf();
					std::cout.rdbuf(OutFile.rdbuf());
					TrackData.PrintSummary();
					std::cout.rdbuf(OldCoutBuf);
					std::cout << "[+] Summary written to " << OutputPath << "\n";
				}
				else
				{
					std::cerr << "[-] Failed to open output file: " << OutputPath << "\n";
					return EXIT_FAILURE;
				}
			}

			if (PrintToConsole) TrackData.PrintSummary();

			return EXIT_SUCCESS;
		}
		else
		{
			std::filesystem::path OutputPath = InputPath;
			OutputPath.replace_extension(".xml");

			std::cout << "[i] Unpacking track: " << TargetPath << "\n";
			if (TrackData.Load(TargetPath))
			{
				if (TrackData.ExportXML(OutputPath.string()))
				{
					std::cout << "[+] Successfully unpacked to " << OutputPath << "\n";
					return EXIT_SUCCESS;
				}
			}
			return EXIT_FAILURE;
		}
	}
	else if (Ext == ".xml")
	{
		std::filesystem::path OutputPath = InputPath;
		OutputPath.replace_extension(".trk");

		std::cout << "[i] Packing XML: " << TargetPath << "\n";
		if (TrackData.ImportXML(TargetPath))
		{
			if (TrackData.Save(OutputPath.string()))
			{
				std::cout << "[+] Successfully packed to " << OutputPath << "\n";
				return EXIT_SUCCESS;
			}
		}
		return EXIT_FAILURE;
	}
	else
	{
		std::cerr << "[-] Unsupported file extension: '" << Ext << "'. Must be .trk or .xml\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
