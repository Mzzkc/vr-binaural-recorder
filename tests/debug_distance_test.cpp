#include <iostream>
#include <cmath>

// Test the distance attenuation calculation
float calculateAttenuation(float distance) {
    float attenuation = 1.0f;
    if (distance > 0.1f) {
        attenuation = std::min(1.0f, 1.0f / (distance * distance * 0.1f + 0.1f));
    }
    return attenuation;
}

float calculateDistance(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

int main() {
    std::cout << "Distance Attenuation Test Debug" << std::endl;
    std::cout << "================================" << std::endl;

    // Test scenario from failing test
    // Near case: Head at (0, 1.2, -0.5), Mic at (0, 1.2, -1.0)
    float distNear = calculateDistance(0.0f, 1.2f, -0.5f, 0.0f, 1.2f, -1.0f);
    float attNear = calculateAttenuation(distNear);

    // Far case: Head at (0, 1.2, -5.0), Mic at (0, 1.2, -1.0)
    float distFar = calculateDistance(0.0f, 1.2f, -5.0f, 0.0f, 1.2f, -1.0f);
    float attFar = calculateAttenuation(distFar);

    std::cout << "Near distance: " << distNear << "m" << std::endl;
    std::cout << "Near attenuation: " << attNear << std::endl;
    std::cout << "Far distance: " << distFar << "m" << std::endl;
    std::cout << "Far attenuation: " << attFar << std::endl;
    std::cout << std::endl;

    std::cout << "Expected: Near attenuation > Far attenuation" << std::endl;
    std::cout << "Actual: " << attNear << " > " << attFar << " = " << (attNear > attFar ? "TRUE" : "FALSE") << std::endl;

    // Test result with same input signal (1.0)
    float signalNear = 1.0f * attNear;
    float signalFar = 1.0f * attFar;

    std::cout << std::endl;
    std::cout << "Signal levels:" << std::endl;
    std::cout << "Near RMS (approx): " << signalNear << std::endl;
    std::cout << "Far RMS (approx): " << signalFar << std::endl;
    std::cout << "Near > Far: " << (signalNear > signalFar ? "TRUE (correct)" : "FALSE (BUG)") << std::endl;

    return 0;
}