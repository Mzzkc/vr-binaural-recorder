# VR BINAURAL RECORDER - AI GUIDE
**C++17 VR spatial audio | CMake 3.16+ | OpenVR+PortAudio+ImGui+spdlog+jsoncpp**

## BUILD
```bash
mkdir -p build && cd build
cmake .. && cmake --build . --config Release
ctest --verbose  # tests
cmake --build . --target package  # installer
```

**Platform deps:**
- Win: ASIO SDK, NSIS
- Linux: X11, OpenGL dev
- macOS: System OpenGL

## ARCH
**Core:** audio_engine(PortAudio ASIO/WASAPI), hrtf_processor(binaural synth+convolution), vr_tracker(OpenVR pose+thread), overlay_ui(ImGui VR), config(JSON hot-reload)

**Deps:** PortAudio(full), OpenVR(v1.23.7 real), ImGui+GLFW(VR), spdlog(perf log), jsoncpp(config), HRTF(MIT KEMAR 368 WAV)

**Config:** vr_binaural_config.json(root) - audio(rate,buf,ASIO), hrtf(dataset,conv), vr(track,overlay), perf(thread,SIMD,mem), log(level,async)

**Tests:** tests/test_suite.cpp(Google Test auto-built)

**Valid:** ./check_status.sh | python3 tools/validator.py

**Debug:** config→"logging.level":"debug", "debug.profiling":true

## PERF
PROD-READY: AVX2 SIMD, real-time thread, lock-free rings, cache-align, <10ms latency target, adaptive buf, mem pools

## STATUS
SUBSTANTIAL: functional VR track, binaural proc, prod infra | PENDING: hardware valid

## FILES
hrtf_data/(368 WAV), third_party/(CMake), logs/(auto-rot), build/(ops), include/(7400+L impl), configs(root)
