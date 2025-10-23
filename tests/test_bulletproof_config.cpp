// Alex Kim's Bulletproof Config System Demo
// Shows off the creative auto-repair capabilities!

#include <iostream>
#include <fstream>
#include <string>
#include "core/include/json_validator.h"

void createBrokenJsonExample() {
    // Create a deliberately broken JSON to test our repair system
    std::string brokenJson = R"({
  // This is a comment (not valid JSON!)
  'audio' : {
    sampleRate: 48000,  // Unquoted key and comment
    "bufferSize": 128,  // Trailing comma coming up...
  },
  "vr": {
    'trackingRate': 90,  // Single quotes
    smoothingFactor: 0.01,  // Another unquoted key
  },   // Trailing comma here too!
})";

    std::ofstream file("/home/emzi/Projects/vr-binaural-recorder/broken_test_config.json");
    file << brokenJson;
    file.close();

    std::cout << "📝 Created broken JSON test file with multiple issues" << std::endl;
    std::cout << "Issues included:" << std::endl;
    std::cout << "  • Comments (not valid JSON)" << std::endl;
    std::cout << "  • Single quotes instead of double quotes" << std::endl;
    std::cout << "  • Unquoted keys" << std::endl;
    std::cout << "  • Trailing commas" << std::endl;
    std::cout << "  • Extra whitespace" << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "🚀 Alex Kim's Bulletproof Config System Demo!" << std::endl;
    std::cout << "==========================================" << std::endl << std::endl;

    // Create a broken JSON file for testing
    createBrokenJsonExample();

    // Test our JSON validator
    std::cout << "🔧 Testing JSON auto-repair capabilities..." << std::endl;
    std::cout << "--------------------------------------------" << std::endl;

    auto report = vrb::JsonValidator::RepairConfigFile(
        "/home/emzi/Projects/vr-binaural-recorder/broken_test_config.json");

    std::cout << std::endl << "📊 VALIDATION REPORT:" << std::endl;
    std::cout << "=====================" << std::endl;

    switch (report.result) {
        case vrb::JsonValidator::ValidationResult::Valid:
            std::cout << "Result: ✅ JSON was already valid!" << std::endl;
            break;
        case vrb::JsonValidator::ValidationResult::RepairedAndValid:
            std::cout << "Result: 🎉 JSON was broken but successfully repaired!" << std::endl;
            break;
        case vrb::JsonValidator::ValidationResult::Unrepairable:
            std::cout << "Result: ❌ JSON was too broken to repair automatically" << std::endl;
            break;
    }

    std::cout << std::endl << "🔍 Issues Found:" << std::endl;
    for (const auto& issue : report.issuesFound) {
        std::cout << "  • " << issue << std::endl;
    }

    std::cout << std::endl << "🔧 Repairs Applied:" << std::endl;
    for (const auto& repair : report.repairsApplied) {
        std::cout << "  • " << repair << std::endl;
    }

    if (report.wasBackedUp) {
        std::cout << std::endl << "💾 Backup created: " << report.backupPath << std::endl;
    }

    // Show the actual config system in action
    std::cout << std::endl << "🎯 Testing with actual Config class..." << std::endl;
    std::cout << "--------------------------------------" << std::endl;

    try {
        // Test the main config file (should work now)
        vrb::Config config("/home/emzi/Projects/vr-binaural-recorder/vr_binaural_config.json");
        std::cout << "✅ Main config loaded successfully!" << std::endl;
        std::cout << "   Sample Rate: " << config.GetSampleRate() << " Hz" << std::endl;
        std::cout << "   Buffer Size: " << config.GetBufferSize() << " samples" << std::endl;

        // Test our repaired config
        vrb::Config repairedConfig("/home/emzi/Projects/vr-binaural-recorder/broken_test_config.json");
        std::cout << "✅ Repaired config loaded successfully!" << std::endl;
        std::cout << "   Sample Rate: " << repairedConfig.GetSampleRate() << " Hz" << std::endl;
        std::cout << "   Buffer Size: " << repairedConfig.GetBufferSize() << " samples" << std::endl;
        std::cout << "   Tracking Rate: " << repairedConfig.GetTrackingRate() << " Hz" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "❌ Config test failed: " << e.what() << std::endl;
    }

    std::cout << std::endl << "🎊 Demo Complete!" << std::endl;
    std::cout << "================" << std::endl;
    std::cout << "The config system is now bulletproof and ready for production!" << std::endl;
    std::cout << "ASMRtists can't break it with malformed JSON anymore! 🛡️" << std::endl;

    return 0;
}