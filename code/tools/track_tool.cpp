//
// track_tool.cpp
//

#include "game/track.hpp"

#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) 
{
    if (argc < 4) 
    {
        std::cout << "Trials HD Track Tool\n";
        std::cout << "Usage:\n";
        std::cout << "  " << argv[0] << " unpack <input.trk> <output.xml>\n";
        std::cout << "  " << argv[0] << " pack <input.xml> <output.trk>\n";
        return EXIT_FAILURE;
    }

    std::string Mode = argv[1];
    std::string Input = argv[2];
    std::string Output = argv[3];

    redlynx::game::Track TrackData;

    if (Mode == "unpack") 
    {
        std::cout << "[i] Loading track: " << Input << "\n";
        if (TrackData.Load(Input)) 
        {
            TrackData.PrintSummary();
            
            std::cout << "[i] Exporting to XML: " << Output << "\n";
            if (TrackData.ExportXML(Output)) 
            {
                std::cout << "[+] Successfully unpacked to " << Output << "\n";
                return EXIT_SUCCESS;
            }
        }
    } 
    else if (Mode == "pack") 
    {
        std::cout << "[i] Importing XML: " << Input << "\n";
        if (TrackData.ImportXML(Input)) 
        {
            std::cout << "[i] Saving to track: " << Output << "\n";
            if (TrackData.Save(Output)) 
            {
                std::cout << "[+] Successfully packed to " << Output << "\n";
                return EXIT_SUCCESS;
            }
        }
    } 
    else 
    {
        std::cerr << "[-] Unknown mode: " << Mode << "\n";
    }

    return EXIT_FAILURE;
}
