#include <Grapheme.h> 

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

int main() {
    Grapheme::SSTTConfig stt_config;
    stt_config.m_model_config.m_model_path = "models/ggml-base.en.bin";
    stt_config.m_model_config.m_language = "en";

    auto stt_engine = Grapheme::CSTTEngine::Create(stt_config);
    if (!stt_engine || !stt_engine->IsLoaded()) {
        std::cerr << "Failed to load STT model\n";
        return 1;
    }

    Grapheme::SAudioCaptureConfig audio_config;
    audio_config.m_sample_rate = 16000;
    audio_config.m_channels = 1;
    audio_config.m_buffer_size_ms = 5000; // 5 seconds of buffer

    auto audio_device = Phoneme::CAudioDevice::Create(audio_config);

    // Check if the hardware is actually available
    Grapheme::EResultCode start_result = audio_device->StartCapture();
    if (start_result != Grapheme::EResultCode::Success) {
        std::cerr << "Capture failed: " << Grapheme::ResultCodeToString(start_result) << "\n";
        return 1;
    }

    Grapheme::SSerializationConfig ser_config;
    ser_config.m_pretty_print = true;
    ser_config.m_include_confidence = true;

    Grapheme::CTranscriptSerializer serializer(ser_config);

    std::vector<Grapheme::STranscriptResult> all_results;
    std::atomic<bool> running{ true };

    // Worker thread: drain audio buffer -> transcribe -> serialize
    std::thread worker([&]() {
        const uint32_t chunk_samples = audio_config.m_sample_rate * 3; // 3-second chunks
        std::vector<float> audio_buffer(chunk_samples);

        while (running.load()) {
            // Wait for enough audio to accumulate
            std::this_thread::sleep_for(std::chrono::seconds(3));

            if (!audio_device->IsCapturing()) {
                break;
            }

            // Transcribe the captured chunk
            Grapheme::STranscriptResult result = stt_engine->Transcribe(audio_buffer.data(),
                static_cast<int>(audio_buffer.size()));

            if (result.Success()) {
                // Serialize to JSON immediately (e.g., for a live feed / websocket)
                std::string json = serializer.Serialize(result);
                std::cout << json << "\n";

                all_results.push_back(std::move(result));
            }
            else {
                // EResultCode tells us *why* it failed
                std::cerr << "Transcription failed: "
                    << Grapheme::ResultCodeToString(result.m_result_code) << "\n";
            }
        }
    });

    // Main thread: wait for user input to stop
    std::cout << "Recording... Press Enter to stop.\n";
    std::cin.get();
    running.store(false);
    worker.join();

    audio_device->StopCapture();

    // SaveToFile also returns EResultCode; the path might be invalid, disk full, etc.
    Grapheme::EResultCode save_result = serializer.SaveToFile("transcript.json", all_results);
    if (save_result != Grapheme::EResultCode::Success) {
        std::cerr << "Save failed: " << Grapheme::ResultCodeToString(save_result) << "\n";
        return 1;
    }

    std::cout << "Saved " << all_results.size() << " segments to transcript.json\n";
    return 0;
}