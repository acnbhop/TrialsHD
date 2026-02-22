//
// tex_tool.cpp
//

// Game headers
#include "game/texture.hpp"

// Standard headers
#include <iostream>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <cstdlib>

void PrintUsage(const char* ProgramName)
{
    std::cout << "=== THD Texture Tool ===\n";
    std::cout << "Usage:\n";
    std::cout << "  " << ProgramName << " <input.tex>                 Extracts the .tex to a .dds file in the same directory\n";
    std::cout << "Batch Processing Commands:\n";
    std::cout << "  " << ProgramName << " --export <folder>           Exports all .tex files in folder to .dds files\n";
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        PrintUsage(argv[0]);
        return EXIT_FAILURE;
    }

    bool BatchExport = false;
    std::string TargetPath = "";

    // Parse arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string Arg = argv[i];
        if (Arg == "--export")
        {
            BatchExport = true;
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

    if (BatchExport)
    {
        if (!std::filesystem::exists(TargetPath) || !std::filesystem::is_directory(TargetPath))
        {
            std::cerr << "[-] Error: Specified target is not a valid directory: " << TargetPath << "\n";
            return EXIT_FAILURE;
        }

        std::cout << "[i] Starting batch processing in folder: " << TargetPath << "\n";

        int ProcessedCount = 0;

        for (const auto& Entry : std::filesystem::directory_iterator(TargetPath))
        {
            if (!Entry.is_regular_file()) continue;

            std::filesystem::path FilePath = Entry.path();
            std::string Ext = FilePath.extension().string();
            std::transform(Ext.begin(), Ext.end(), Ext.begin(), [](unsigned char c) { return std::tolower(c); });

            if (Ext == ".tex")
            {
                std::cout << "  -> Extracting: " << FilePath.filename().string() << " ... ";

                redlynx::game::Texture TexData;
                if (!TexData.LoadTex(FilePath.string()))
                {
                    std::cout << "[FAILED]\n";
                    continue;
                }

                std::filesystem::path OutPath = FilePath;
                OutPath.replace_extension(".dds");
                if (TexData.SaveDDS(OutPath.string()))
                {
                    std::cout << "[DDS EXPORTED]\n";
                    ProcessedCount++;
                }
                else
                {
                    std::cout << "[DDS FAILED]\n";
                }
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

    if (Ext != ".tex")
    {
        std::cerr << "[-] Unsupported file extension: '" << Ext << "'. Must be .tex\n";
        return EXIT_FAILURE;
    }

    redlynx::game::Texture TexData;
    std::cout << "[i] Loading texture: " << TargetPath << "\n";
    if (!TexData.LoadTex(TargetPath))
    {
        return EXIT_FAILURE;
    }

    std::filesystem::path OutputPath = InputPath;
    OutputPath.replace_extension(".dds");

    std::cout << "[i] Exporting to: " << OutputPath << "\n";
    if (TexData.SaveDDS(OutputPath.string()))
    {
        std::cout << "[+] Successfully exported to DDS.\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
