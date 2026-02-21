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
    std::string InputFile = "";

    // parse command line arguments
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
        else 
        {
            InputFile = Arg;
        }
    }

    if (InputFile.empty()) 
    {
        std::cerr << "[-] No input file specified.\n";
        PrintUsage(argv[0]);
        return EXIT_FAILURE;
    }

    std::filesystem::path InputPath(InputFile);
    std::string Ext = InputPath.extension().string();

    // for safety.
    std::transform(Ext.begin(), Ext.end(), Ext.begin(), [](unsigned char c){ return std::tolower(c); });

    redlynx::game::Track TrackData;

    if (Ext == ".trk") 
    {
        if (PrintToFile || PrintToConsole) 
        {
            std::cout << "[i] Loading track for inspection: " << InputFile << "\n";
            if (!TrackData.Load(InputFile)) 
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
                    // hijack std::cout to redirect it to our text file
                    std::streambuf* OldCoutBuf = std::cout.rdbuf();
                    std::cout.rdbuf(OutFile.rdbuf());
                    
                    TrackData.PrintSummary();
                    
                    // restore the original std::cout buffer
                    std::cout.rdbuf(OldCoutBuf);
                    std::cout << "[+] Summary written to " << OutputPath << "\n";
                } 
                else 
                {
                    std::cerr << "[-] Failed to open output file: " << OutputPath << "\n";
                    return EXIT_FAILURE;
                }
            }
            
            if (PrintToConsole) 
            {
                TrackData.PrintSummary();
            }
            
            return EXIT_SUCCESS;
        } 
        else 
        {
            // unpacking mode
            std::filesystem::path OutputPath = InputPath;
            OutputPath.replace_extension(".xml");
            
            std::cout << "[i] Unpacking track: " << InputFile << "\n";
            if (TrackData.Load(InputFile)) 
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
        // packing mode
        std::filesystem::path OutputPath = InputPath;
        OutputPath.replace_extension(".trk");

        std::cout << "[i] Packing XML: " << InputFile << "\n";
        if (TrackData.ImportXML(InputFile)) 
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
