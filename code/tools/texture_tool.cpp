//
// texture_tool.cpp
//

// Engine headers
#include "engine/asset/texture.hpp"

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
	std::cout << "=== THD Texture Tool ===\n";
	std::cout << "Usage:\n";
	std::cout << "  " << ProgramName << " <input.tex>                 Exports the TEX to a .dds file in the same directory\n";
	std::cout << "  " << ProgramName << " <input.dds>                 Imports the DDS file into a .tex file\n";
	std::cout << "  " << ProgramName << " --print <input.tex>         Prints the TEX summary to a .txt file\n";
	std::cout << "  " << ProgramName << " --print-console <input.tex> Prints the TEX summary to the console\n";
	std::cout << "\nBatch Processing Commands:\n";
	std::cout << "  " << ProgramName << " --export <folder>           Exports all .tex files in folder to .dds files\n";
	std::cout << "  " << ProgramName << " --import <folder>           Imports all .dds files in folder to .tex files\n";
	std::cout << "  " << ProgramName << " --print-dump <folder>       Dumps summaries for all .tex files to .txt files\n";
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
	bool BatchImport = false;
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
		else if (Arg == "--import")
		{
			BatchImport = true;
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

	if (BatchExport || BatchImport || BatchDump)
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
		int FailedCount = 0;

		for (const auto& Entry : std::filesystem::recursive_directory_iterator(TargetPath))
		{
			if (!Entry.is_regular_file()) continue;

			std::filesystem::path FilePath = Entry.path();
			std::string Ext = FilePath.extension().string();
			std::transform(Ext.begin(), Ext.end(), Ext.begin(), [](unsigned char c) { return std::tolower(c); });

			// For export and dump, look for .tex files
			if ((BatchExport || BatchDump) && Ext == ".tex")
			{
				std::filesystem::path RelPath = std::filesystem::relative(FilePath, TargetPath);
				std::cout << "  -> Loading: " << RelPath.string() << " ... ";

				redlynx::engine::asset::Tex TexData;
				if (!TexData.Load(FilePath.string()))
				{
					std::cout << "[FAILED TO LOAD]\n";
					FailedCount++;
					continue;
				}

				if (BatchExport)
				{
					std::filesystem::path OutPath;
					if (!OutputDir.empty())
					{
						OutPath = std::filesystem::path(OutputDir) / RelPath;
						OutPath.replace_extension(".dds");
						std::filesystem::create_directories(OutPath.parent_path());
					}
					else
					{
						OutPath = FilePath;
						OutPath.replace_extension(".dds");
					}
					if (TexData.ExportDDS(OutPath.string()))
					{
						std::cout << "[DDS EXPORTED] ";
					}
					else
					{
						std::cout << "[DDS FAILED] ";
						FailedCount++;
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
						TexData.PrintSummary();
						std::cout.rdbuf(OldCoutBuf);

						std::cout << "[TXT DUMPED]";
					}
					else
					{
						std::cout << "[TXT FAILED]";
						FailedCount++;
					}
				}

				std::cout << "\n";
				ProcessedCount++;
			}

			// For import, look for .dds files
			if (BatchImport && Ext == ".dds")
			{
				std::filesystem::path RelPath = std::filesystem::relative(FilePath, TargetPath);
				std::cout << "  -> Importing: " << RelPath.string() << " ... ";

				redlynx::engine::asset::Tex TexData;
				if (!TexData.ImportDDS(FilePath.string()))
				{
					std::cout << "[FAILED TO IMPORT]\n";
					FailedCount++;
					continue;
				}

				std::filesystem::path OutPath;
				if (!OutputDir.empty())
				{
					OutPath = std::filesystem::path(OutputDir) / RelPath;
					OutPath.replace_extension(".tex");
					std::filesystem::create_directories(OutPath.parent_path());
				}
				else
				{
					OutPath = FilePath;
					OutPath.replace_extension(".tex");
				}

				if (TexData.Save(OutPath.string()))
				{
					std::cout << "[TEX SAVED]\n";
				}
				else
				{
					std::cout << "[TEX FAILED]\n";
					FailedCount++;
				}

				ProcessedCount++;
			}
		}

		std::cout << "[+] Batch processing complete. Processed " << ProcessedCount << " files";
		if (FailedCount > 0)
		{
			std::cout << " (" << FailedCount << " failed)";
		}
		std::cout << ".\n";
		return (FailedCount > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	//
	// SINGLE FILE PROCESSING
	//

	std::filesystem::path InputPath(TargetPath);
	std::string Ext = InputPath.extension().string();
	std::transform(Ext.begin(), Ext.end(), Ext.begin(), [](unsigned char c){ return std::tolower(c); });

	if (Ext == ".tex")
	{
		redlynx::engine::asset::Tex TexData;

		if (PrintToFile || PrintToConsole)
		{
			std::cout << "[i] Loading TEX for inspection: " << TargetPath << "\n";
			if (!TexData.Load(TargetPath))
			{
				return EXIT_FAILURE;
			}

			if (PrintToFile)
			{
				std::filesystem::path OutputPath;
				if (!OutputDir.empty())
				{
					OutputPath = std::filesystem::path(OutputDir) / InputPath.filename();
					OutputPath.replace_extension(".txt");
					std::filesystem::create_directories(OutputPath.parent_path());
				}
				else
				{
					OutputPath = InputPath;
					OutputPath.replace_extension(".txt");
				}

				std::ofstream OutFile(OutputPath);
				if (OutFile)
				{
					std::streambuf* OldCoutBuf = std::cout.rdbuf();
					std::cout.rdbuf(OutFile.rdbuf());
					TexData.PrintSummary();
					std::cout.rdbuf(OldCoutBuf);
					std::cout << "[+] Summary written to " << OutputPath << "\n";
				}
				else
				{
					std::cerr << "[-] Failed to open output file: " << OutputPath << "\n";
					return EXIT_FAILURE;
				}
			}

			if (PrintToConsole) TexData.PrintSummary();

			return EXIT_SUCCESS;
		}
		else
		{
			// Default action for .tex: export to .dds
			std::filesystem::path OutputPath;
			if (!OutputDir.empty())
			{
				OutputPath = std::filesystem::path(OutputDir) / InputPath.filename();
				OutputPath.replace_extension(".dds");
				std::filesystem::create_directories(OutputPath.parent_path());
			}
			else
			{
				OutputPath = InputPath;
				OutputPath.replace_extension(".dds");
			}

			std::cout << "[i] Exporting TEX to DDS: " << TargetPath << "\n";
			if (TexData.Load(TargetPath))
			{
				if (TexData.ExportDDS(OutputPath.string()))
				{
					std::cout << "[+] Successfully exported to " << OutputPath << "\n";
					return EXIT_SUCCESS;
				}
			}
			return EXIT_FAILURE;
		}
	}
	else if (Ext == ".dds")
	{
		// Default action for .dds: import to .tex
		std::filesystem::path OutputPath;
		if (!OutputDir.empty())
		{
			OutputPath = std::filesystem::path(OutputDir) / InputPath.filename();
			OutputPath.replace_extension(".tex");
			std::filesystem::create_directories(OutputPath.parent_path());
		}
		else
		{
			OutputPath = InputPath;
			OutputPath.replace_extension(".tex");
		}

		std::cout << "[i] Importing DDS to TEX: " << TargetPath << "\n";

		redlynx::engine::asset::Tex TexData;
		if (TexData.ImportDDS(TargetPath))
		{
			if (TexData.Save(OutputPath.string()))
			{
				std::cout << "[+] Successfully imported to " << OutputPath << "\n";
				return EXIT_SUCCESS;
			}
		}
		return EXIT_FAILURE;
	}
	else
	{
		std::cerr << "[-] Unsupported file extension: '" << Ext << "'. Must be .tex or .dds\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}