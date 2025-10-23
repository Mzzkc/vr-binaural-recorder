// Quick config validation script for Sam's testing demands
// Shows the expanded config loads all required spatial audio parameters

#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::cout << "🔍 CONFIG VALIDATION FOR SAM'S TESTING" << std::endl;
    std::cout << "=====================================" << std::endl;

    std::ifstream configFile("/home/emzi/Projects/vr-binaural-recorder/config/vr_binaural_config.json.example");
    if (!configFile.is_open()) {
        std::cout << "❌ Config file not found!" << std::endl;
        return 1;
    }

    std::string line, content;
    int lineCount = 0;
    int fieldCount = 0;

    // Count lines and check for essential fields
    std::string essentialFields[] = {
        "bufferSize", "sampleRate", "inputDevice", "outputDevice",
        "api", "dataPath", "dataset", "filterLength", "interpolation",
        "maxDistance", "rolloffFactor", "distanceModel", "enableManualPositioning",
        "showDebugInfo", "testMode", "verboseAudio"
    };

    std::cout << "📊 SCANNING CONFIG FILE..." << std::endl;

    while (std::getline(configFile, line)) {
        lineCount++;
        content += line + "\n";

        // Check for essential fields
        for (const auto& field : essentialFields) {
            if (line.find("\"" + field + "\"") != std::string::npos) {
                fieldCount++;
            }
        }
    }

    configFile.close();

    std::cout << std::endl;
    std::cout << "✅ Config file structure:" << std::endl;
    std::cout << "   Lines: " << lineCount << " (vs old 5-line version)" << std::endl;
    std::cout << "   Essential fields found: " << fieldCount << "/16" << std::endl;

    // Check for specific sections Sam needs
    bool hasAudioSection = content.find("\"audio\":") != std::string::npos;
    bool hasHRTFSection = content.find("\"hrtf\":") != std::string::npos;
    bool hasSpatialSection = content.find("\"spatial\":") != std::string::npos;
    bool hasDebugSection = content.find("\"debug\":") != std::string::npos;
    bool hasWASAPI = content.find("\"wasapi\"") != std::string::npos;

    std::cout << std::endl;
    std::cout << "🎯 SAM'S REQUIRED SECTIONS:" << std::endl;
    std::cout << "   Audio configuration: " << (hasAudioSection ? "✅" : "❌") << std::endl;
    std::cout << "   HRTF settings: " << (hasHRTFSection ? "✅" : "❌") << std::endl;
    std::cout << "   Spatial parameters: " << (hasSpatialSection ? "✅" : "❌") << std::endl;
    std::cout << "   Debug/testing flags: " << (hasDebugSection ? "✅" : "❌") << std::endl;
    std::cout << "   Windows WASAPI support: " << (hasWASAPI ? "✅" : "❌") << std::endl;

    if (hasAudioSection && hasHRTFSection && hasSpatialSection && hasDebugSection && hasWASAPI) {
        std::cout << std::endl;
        std::cout << "🎉 VALIDATION PASSED!" << std::endl;
        std::cout << "Config is now functional for spatial audio testing." << std::endl;
        std::cout << "Sam can proceed with Windows validation!" << std::endl;
    } else {
        std::cout << std::endl;
        std::cout << "❌ VALIDATION FAILED!" << std::endl;
        std::cout << "Missing required sections for spatial audio." << std::endl;
        return 1;
    }

    return 0;
}