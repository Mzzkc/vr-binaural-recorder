// Test JSON config parsing to debug the reported issue
// Mike Rodriguez - Config parsing investigation

#include <iostream>
#include <fstream>
#include <json/json.h>

int main() {
    std::cout << "=== JSON Config Parsing Test ===" << std::endl;

    // Test the main config file
    std::string configPath = "vr_binaural_config.json";
    std::ifstream file(configPath);

    if (!file.is_open()) {
        std::cerr << "❌ Cannot open config file: " << configPath << std::endl;
        return 1;
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;

    std::cout << "📄 Testing config file: " << configPath << std::endl;

    if (!Json::parseFromStream(builder, file, &root, &errors)) {
        std::cerr << "❌ Failed to parse config: " << errors << std::endl;
        return 1;
    }

    std::cout << "✅ JSON parsing successful!" << std::endl;

    // Test accessing some values
    if (root.isMember("audio")) {
        Json::Value audio = root["audio"];
        if (audio.isMember("sampleRate")) {
            int sampleRate = audio["sampleRate"].asInt();
            std::cout << "✅ Sample rate: " << sampleRate << " Hz" << std::endl;
        }
        if (audio.isMember("bufferSize")) {
            int bufferSize = audio["bufferSize"].asInt();
            std::cout << "✅ Buffer size: " << bufferSize << " samples" << std::endl;
        }
    } else {
        std::cout << "⚠️  No 'audio' section found" << std::endl;
    }

    // Test if file exists where Config class expects it
    std::string defaultPath = "config/vr_binaural_config.json";
    std::ifstream defaultFile(defaultPath);
    if (defaultFile.is_open()) {
        std::cout << "✅ Default config path also exists: " << defaultPath << std::endl;
        defaultFile.close();
    } else {
        std::cout << "ℹ️  Default config path not found: " << defaultPath << std::endl;
    }

    file.close();

    std::cout << "\n🎯 CONCLUSION: JSON config parsing is working correctly!" << std::endl;
    std::cout << "If errors were reported, they may be from a different config file or context." << std::endl;

    return 0;
}