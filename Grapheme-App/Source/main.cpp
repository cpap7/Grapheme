#include <Grapheme.h> 

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <mutex>

int main() {
    Grapheme::SSTTConfig stt_config;
    stt_config.m_model_config.m_model_path = "Assets/Models/ggml-small.bin";
    stt_config.m_model_config.m_language = "en";

    auto stt_engine = Grapheme::CSTTEngine::Create(stt_config);
    if (!stt_engine || !stt_engine->IsLoaded()) {
        std::cerr << "[GRAPHEME] Failed to load STT model\n";
        return 1;
    }

    Grapheme::SAudioCaptureConfig audio_config;
    audio_config.m_sample_rate = 16000;
    audio_config.m_channels = 1;
    audio_config.m_buffer_size_ms = 4000; // buffer seconds
    audio_config.m_device_type = Grapheme::EDeviceType::Loopback; // Receive audio from speaker

    auto audio_device = Grapheme::CAudioDevice::Create(audio_config);

    // Shared buffer (populated via audio callback)
    std::mutex audio_buffer_mutex;
    std::vector<float> audio_buffer;
    audio_device->SetAudioDataCallback([&](const float* a_samples, uint32_t a_count) {
        std::lock_guard<std::mutex> lock(audio_buffer_mutex);
        audio_buffer.insert(audio_buffer.end(), a_samples, a_samples + a_count);
    });

    // Check if the hardware is actually available
    Grapheme::EResultCode start_result = audio_device->StartCapture();
    if (start_result != Grapheme::EResultCode::Success) {
        std::cerr << "[GRAPHEME] Capture failed: " << Grapheme::ResultCodeToString(start_result) << "\n";
        return 1;
    }
    
    // Setup serializer
    Grapheme::SSerializationConfig ser_config;
    ser_config.m_pretty_print = true;
    ser_config.m_include_confidence = false;
    Grapheme::CTranscriptSerializer serializer(ser_config);

    std::vector<Grapheme::STranscriptResult> all_results;
    std::atomic<bool> running{ true };

    // Worker thread: drain audio callback buffer -> transcribe -> serialize
    std::thread worker([&]() {
        while (running.load()) {
            // Wait for enough audio to accumulate
            std::this_thread::sleep_for(std::chrono::seconds(4));

            // Drain the accumulated audio
            std::vector<float> chunk;
            {
                std::lock_guard<std::mutex> lock(audio_buffer_mutex);
                chunk.swap(audio_buffer);
            }

            if (chunk.empty() || !audio_device->IsCapturing()) {
                continue;
            }

            Grapheme::STranscriptResult result = stt_engine->Transcribe(chunk.data(), static_cast<int>(chunk.size()));

            if (result.Success()) {
                //serializer.SaveToFile("chunk.json", result);
                std::cout << "[GRAPHEME] Audio Transcription: " << result.m_text << "\n";
                all_results.push_back(std::move(result));
            }
            else {
                std::cerr << "Transcription failed: " << Grapheme::ResultCodeToString(result.m_result_code) << "\n";
            }
        }
    });

    // Main thread: wait for user input to stop
    std::cout << "[GRAPHEME] Capturing speaker audio... Press Enter to stop.\n";
    std::cin.get();
    running.store(false);
    worker.join();

    audio_device->StopCapture();

    // SaveToFile also returns EResultCode; the path might be invalid, disk full, etc.
    Grapheme::EResultCode save_result = serializer.SaveToFile("transcript.json", all_results);
    if (save_result != Grapheme::EResultCode::Success) {
        std::cerr << "[GRAPHEME] Save failed: " << Grapheme::ResultCodeToString(save_result) << "\n";
        return 1;
    }

    std::cout << "[GRAPHEME] Saved " << all_results.size() << " segments to transcript.json\n";
    return 0;
}