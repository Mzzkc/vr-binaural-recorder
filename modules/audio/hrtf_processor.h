// hrtf_processor.h - Head-Related Transfer Function Processing
// Implements spatial audio using HRTF convolution

#pragma once

#include <memory>
#include <vector>
#include <array>
#include <atomic>
#include <mutex>
#include <complex>
#include <filesystem>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <immintrin.h>  // For SIMD
#include "vr_types.h"

namespace vrb {


/**
 * @brief HRTF processor for spatial audio rendering
 */
class HRTFProcessor {
public:
    HRTFProcessor();
    ~HRTFProcessor();

    bool Initialize(const std::string& hrtfDataPath);
    void UpdateSpatialPosition(const VRPose& hmdPose, const std::vector<VRPose>& controllerPoses);
    void Process(const float* input, float* output, size_t frames, int inputChannels);
    void Reset();

    struct ProcessingStats {
        float azimuth;
        float elevation;
        float distance;
        int hrtfIndex;
    };
    ProcessingStats GetStats() const;

private:
    struct HRTFData {
        static constexpr int FILTER_LENGTH = 512;
        static constexpr int NUM_AZIMUTHS = 72;
        static constexpr int NUM_ELEVATIONS = 14;

        struct Filter {
            std::array<float, FILTER_LENGTH> left;
            std::array<float, FILTER_LENGTH> right;
        };

        std::vector<Filter> filters;
        const Filter& GetFilter(float azimuth, float elevation) const;
        int GetFilterIndex(float azimuth, float elevation) const;
    };

    class ConvolutionEngine {
    public:
        ConvolutionEngine(int filterLength);
        ~ConvolutionEngine();
        void Process(const float* input, float* outputLeft, float* outputRight,
                    size_t frames, const HRTFData::Filter& filter);
        void Reset();

    private:
        int m_filterLength;
        std::vector<float> m_historyBuffer;
        int m_historyIndex;
        int m_fftSize;
        std::vector<std::complex<float>> m_filterFFTLeft;
        std::vector<std::complex<float>> m_filterFFTRight;
        std::vector<std::complex<float>> m_signalFFT;
        std::vector<std::complex<float>> m_outputFFT;
        std::vector<std::complex<float>> m_twiddleFactors;
        std::vector<float> m_overlapBuffer;
        const HRTFData::Filter* m_cachedFilter{nullptr};
        bool m_filterCached{false};

        void InitializeFFT();
        void CleanupFFT();
        void ProcessFFT(const float* input, float* outputLeft, float* outputRight,
                       size_t frames, const HRTFData::Filter& filter);
        void ProcessTimeDomain(const float* input, float* outputLeft, float* outputRight,
                              size_t frames, const HRTFData::Filter& filter);
        void ComputeFilterFFT(const HRTFData::Filter& filter);
        void PerformRealFFT(const float* input, std::complex<float>* output, int size);
        void PerformRealIFFT(const std::complex<float>* input, float* output, int size);
        void FFTRadix2(const std::complex<float>* input, std::complex<float>* output, int size);
        void IFFTRadix2(const std::complex<float>* input, std::complex<float>* output, int size);
        void ComplexMultiply(const std::complex<float>* a, const std::complex<float>* b,
                            std::complex<float>* result, int size);
    };

    class InterpolationEngine {
    public:
        InterpolationEngine();
        void UpdateTarget(float azimuth, float elevation, float distance);
        void GetSmoothedValues(float& azimuth, float& elevation, float& distance);

    private:
        std::atomic<float> m_targetAzimuth{0.0f};
        std::atomic<float> m_targetElevation{0.0f};
        std::atomic<float> m_targetDistance{1.0f};
        float m_currentAzimuth{0.0f};
        float m_currentElevation{0.0f};
        float m_currentDistance{1.0f};
        static constexpr float SMOOTHING_FACTOR = 0.95f;
    };

    bool LoadHRTFDataset(const std::string& path);
    bool LoadSOFAFile(const std::string& filename);
    bool LoadMITKEMARCompact(const std::string& filename);
    bool LoadMITKEMARFiles(const std::string& path);
    bool LoadImpulseResponse(const std::string& leftFile, const std::string& rightFile,
                             float azimuth, float elevation);
    bool GenerateHighQualitySyntheticHRTF();

    void CalculateAngles(const VRPose& headPose, const VRPose& micPose,
                        float& azimuth, float& elevation, float& distance);
    void ApplyDistanceAttenuation(float* buffer, size_t frames, float distance);

    std::unique_ptr<HRTFData> m_hrtfData;
    std::unique_ptr<ConvolutionEngine> m_convolution;
    std::unique_ptr<InterpolationEngine> m_interpolation;

    mutable std::mutex m_processingMutex;
    std::atomic<bool> m_initialized{false};

    std::atomic<float> m_currentAzimuth{0.0f};
    std::atomic<float> m_currentElevation{0.0f};
    std::atomic<float> m_currentDistance{1.0f};
    std::atomic<int> m_currentFilterIndex{0};

};

} // namespace vrb