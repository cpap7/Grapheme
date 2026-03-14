#pragma once
#include <cstdint>
#include <atomic>
#include <vector>


namespace Grapheme {

	// Lock-free single-producer single-consumer ring buffer
	// Producer: miniaudio callback thread
	// Consumer: STT processing thread

	class CAudioRingBuffer {
	private:
		std::vector<float> m_audio_buffer;
		std::atomic<uint32_t> m_write_head{ 0 };
		std::atomic<uint32_t> m_read_head{ 0 };
		uint32_t m_capacity_samples = 0;

	public:
		explicit CAudioRingBuffer(uint32_t a_capacity_samples);

		uint32_t Write(const float* a_audio_data, uint32_t a_sample_count);
		uint32_t Read(float* a_destination, uint32_t a_sample_count);
		uint32_t Available() const;
		void Reset();
	
	};

}
