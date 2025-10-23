// Debug test for distance attenuation issue
// Mike Rodriguez - Investigating the distance attenuation inversion

#include <iostream>
#include <cmath>

// Simulate the distance attenuation calculation
float calculateAttenuation(float distance) {
    if (distance > 0.1f) {
        return std::min(1.0f, 1.0f / (distance * distance * 0.1f + 0.1f));
    }
    return 1.0f;
}

// Simulate the Position calculation from SetListenerPosition
float calculateDistance(float x, float y, float z) {
    return std::sqrt(x * x + y * y + z * z);
}

int main() {
    std::cout << "=== DISTANCE ATTENUATION DEBUG ===" << std::endl;

    // Test positions from the failing test
    float nearX = 0.1f, nearY = 0.0f, nearZ = 0.0f;  // Close distance position from test
    float farX = 5.0f, farY = 0.0f, farZ = 0.0f;     // Far distance position from test

    float nearDistance = calculateDistance(nearX, nearY, nearZ);
    float farDistance = calculateDistance(farX, farY, farZ);

    float nearAttenuation = calculateAttenuation(nearDistance);
    float farAttenuation = calculateAttenuation(farDistance);

    std::cout << "Near position (" << nearX << ", " << nearY << ", " << nearZ << "):" << std::endl;
    std::cout << "  Distance: " << nearDistance << std::endl;
    std::cout << "  Attenuation: " << nearAttenuation << std::endl;

    std::cout << "Far position (" << farX << ", " << farY << ", " << farZ << "):" << std::endl;
    std::cout << "  Distance: " << farDistance << std::endl;
    std::cout << "  Attenuation: " << farAttenuation << std::endl;

    // Test energy calculation
    // Assume base energy of 1.0 for test tone
    float baseEnergy = 100.0f; // Some baseline energy

    float nearEnergy = baseEnergy * nearAttenuation * nearAttenuation; // Attenuation is applied squared in energy
    float farEnergy = baseEnergy * farAttenuation * farAttenuation;

    std::cout << std::endl << "Energy comparison:" << std::endl;
    std::cout << "  Near energy: " << nearEnergy << std::endl;
    std::cout << "  Far energy: " << farEnergy << std::endl;
    std::cout << "  Ratio (near/far): " << (nearEnergy / (farEnergy + 0.0001f)) << std::endl;

    // Expected: nearEnergy > farEnergy
    if (nearEnergy > farEnergy) {
        std::cout << "✅ CORRECT: Near energy is higher than far energy" << std::endl;
    } else {
        std::cout << "❌ PROBLEM: Far energy is higher than near energy!" << std::endl;
    }

    // Check if ratio is greater than 2.0 as required by test
    float ratio = nearEnergy / (farEnergy + 0.0001f);
    if (ratio > 2.0f) {
        std::cout << "✅ CORRECT: Attenuation ratio " << ratio << " is > 2.0" << std::endl;
    } else {
        std::cout << "❌ PROBLEM: Attenuation ratio " << ratio << " is <= 2.0" << std::endl;
    }

    return 0;
}