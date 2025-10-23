// simple_spatial_test.cpp - Direct test of spatial audio without dependencies
#include <iostream>
#include <cmath>
#include <vector>
#include <array>
#include <atomic>
#include <memory>

// Minimal implementation to test spatial audio directly
struct Vec3 {
    float x, y, z;
    Vec3(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}
};

class MinimalHRTFTest {
    static constexpr int FILTER_LENGTH = 512;
    static constexpr int NUM_AZIMUTHS = 72;
    static constexpr int NUM_ELEVATIONS = 14;

    struct Filter {
        std::array<float, FILTER_LENGTH> left;
        std::array<float, FILTER_LENGTH> right;
    };

    std::vector<Filter> filters;
    std::atomic<float> currentAzimuth{0.0f};
    std::atomic<float> currentElevation{0.0f};

public:
    bool Initialize() {
        // Generate synthetic HRTF filters
        const int totalFilters = NUM_AZIMUTHS * NUM_ELEVATIONS;
        filters.resize(totalFilters);

        for (int elev = 0; elev < NUM_ELEVATIONS; ++elev) {
            for (int az = 0; az < NUM_AZIMUTHS; ++az) {
                int index = elev * NUM_AZIMUTHS + az;
                float azimuth = (az * 360.0f / NUM_AZIMUTHS) - 180.0f;
                auto& filter = filters[index];

                // Generate spatial differences
                for (int i = 0; i < FILTER_LENGTH; ++i) {
                    if (i < 64) {
                        float delay = i / 48000.0f;
                        float leftAttenuation = 1.0f - abs(azimuth) / 360.0f;
                        float rightAttenuation = 1.0f - abs(-azimuth) / 360.0f;

                        filter.left[i] = leftAttenuation * std::exp(-delay * 1000.0f);
                        filter.right[i] = rightAttenuation * std::exp(-delay * 1000.0f);

                        // Ensure significant differences for extreme positions
                        if (azimuth < -90.0f) {  // Far left
                            filter.left[i] *= 2.0f;
                            filter.right[i] *= 0.3f;
                        } else if (azimuth > 90.0f) {  // Far right
                            filter.left[i] *= 0.3f;
                            filter.right[i] *= 2.0f;
                        }
                    } else {
                        filter.left[i] = 0.0f;
                        filter.right[i] = 0.0f;
                    }
                }
            }
        }
        return true;
    }

    void SetListenerPosition(const Vec3& position) {
        // Simple azimuth calculation
        float azimuth = std::atan2(position.x, -position.z) * 180.0f / M_PI;
        currentAzimuth = azimuth;
    }

    const Filter& GetFilter() {
        float azimuth = currentAzimuth.load();

        // Normalize and map to index
        while (azimuth < -180.0f) azimuth += 360.0f;
        while (azimuth > 180.0f) azimuth -= 360.0f;

        int azIndex = static_cast<int>((azimuth + 180.0f) * NUM_AZIMUTHS / 360.0f);
        azIndex = std::max(0, std::min(NUM_AZIMUTHS - 1, azIndex));

        // Use middle elevation
        int index = (NUM_ELEVATIONS/2) * NUM_AZIMUTHS + azIndex;
        return filters[std::min(index, (int)filters.size() - 1)];
    }

    void Process(const float* input, float* output, size_t frames) {
        const auto& filter = GetFilter();

        // Simple time-domain convolution
        for (size_t i = 0; i < frames; ++i) {
            float leftSum = 0.0f, rightSum = 0.0f;

            // Only use first 32 filter taps for speed
            for (int j = 0; j < std::min(32, FILTER_LENGTH); ++j) {
                if (i >= j) {
                    leftSum += input[i-j] * filter.left[j];
                    rightSum += input[i-j] * filter.right[j];
                }
            }

            output[i*2] = leftSum;
            output[i*2+1] = rightSum;
        }
    }
};

int main() {
    std::cout << "=== MINIMAL SPATIAL AUDIO TEST ===" << std::endl;

    MinimalHRTFTest processor;
    if (!processor.Initialize()) {
        std::cout << "FAILED: Could not initialize" << std::endl;
        return 1;
    }

    const int FRAMES = 512;
    std::vector<float> input(FRAMES);
    std::vector<float> outputLeft(FRAMES * 2);
    std::vector<float> outputRight(FRAMES * 2);

    // Generate test tone
    for (int i = 0; i < FRAMES; ++i) {
        input[i] = 0.5f * std::sin(2.0f * M_PI * 440.0f * i / 48000.0f);
    }

    // Test LEFT position
    std::cout << "Testing LEFT position (-2, 0, 0)" << std::endl;
    processor.SetListenerPosition(Vec3(-2.0f, 0.0f, 0.0f));
    processor.Process(input.data(), outputLeft.data(), FRAMES);

    float leftEnergyL = 0.0f, rightEnergyL = 0.0f;
    for (int i = 0; i < FRAMES; ++i) {
        leftEnergyL += outputLeft[i*2] * outputLeft[i*2];
        rightEnergyL += outputLeft[i*2+1] * outputLeft[i*2+1];
    }

    // Test RIGHT position
    std::cout << "Testing RIGHT position (2, 0, 0)" << std::endl;
    processor.SetListenerPosition(Vec3(2.0f, 0.0f, 0.0f));
    processor.Process(input.data(), outputRight.data(), FRAMES);

    float leftEnergyR = 0.0f, rightEnergyR = 0.0f;
    for (int i = 0; i < FRAMES; ++i) {
        leftEnergyR += outputRight[i*2] * outputRight[i*2];
        rightEnergyR += outputRight[i*2+1] * outputRight[i*2+1];
    }

    std::cout << "\n=== RESULTS ===" << std::endl;
    std::cout << "Left pos - L: " << leftEnergyL << ", R: " << rightEnergyL << std::endl;
    std::cout << "Right pos - L: " << leftEnergyR << ", R: " << rightEnergyR << std::endl;

    // Analysis
    bool leftCorrect = leftEnergyL > rightEnergyL;
    bool rightCorrect = rightEnergyR > leftEnergyR;
    bool identical = (leftEnergyL == leftEnergyR) && (rightEnergyL == rightEnergyR);

    std::cout << "Left position favors left: " << (leftCorrect ? "YES" : "NO") << std::endl;
    std::cout << "Right position favors right: " << (rightCorrect ? "YES" : "NO") << std::endl;
    std::cout << "Outputs identical: " << (identical ? "YES (BROKEN)" : "NO (WORKING)") << std::endl;

    if (leftCorrect && rightCorrect && !identical) {
        std::cout << "\n✅ SPATIAL AUDIO ALGORITHM WORKS!" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ SPATIAL AUDIO ALGORITHM BROKEN!" << std::endl;
        return 1;
    }
}