#include "gpch.h"
#include "AudioRingBuffer.h"

#include <algorithm>
#include <cstring>

namespace Grapheme {

	CAudioRingBuffer::CAudioRingBuffer(uint32_t a_capacity_samples) 
		: m_capacity_samples(a_capacity_samples + 1) { // We add one to distinguish between full vs empty later (and we avoid needing a separate count atomic which would need a lock or CAS loop) 
		m_audio_buffer.resize(m_capacity_samples, 0.0f);
	}

	uint32_t CAudioRingBuffer::Write(const float* a_audio_data, uint32_t a_sample_count) {
		// Called via miniaudio callback thread (aka producer)

		const uint32_t write_position = m_write_head.load(std::memory_order_relaxed);
		const uint32_t read_position = m_read_head.load(std::memory_order_acquire);

		// Available space = capacity - 1 - current stored count
		const uint32_t used_space		= (write_position - read_position + m_capacity_samples) % m_capacity_samples;
		const uint32_t free_space		= (m_capacity_samples - 1) - used_space;
		const uint32_t to_write	= std::min(a_sample_count, free_space);
		
		if (to_write == 0) {
			return 0; // Buffer full, so we drop the samples instead of blocking, because that could cause glitches
		}

		// Write in up to 2 segments -- memcpy handles circular wrap-around w/o branching per sample 
		const uint32_t first_chunk = std::min(to_write, m_capacity_samples - write_position);
		std::memcpy(m_audio_buffer.data() + write_position, a_audio_data, first_chunk * sizeof(float));

		if (to_write > first_chunk) {
			const uint32_t second_chunk = to_write - first_chunk;
			std::memcpy(m_audio_buffer.data(), a_audio_data + first_chunk, second_chunk * sizeof(float));
		}

		// Publish new write position
		// Consumer will see all written data
		m_write_head.store((write_position + to_write) % m_capacity_samples, std::memory_order_release);

		return to_write;
	}

	uint32_t CAudioRingBuffer::Read(float* a_destination, uint32_t a_sample_count) {
		// Called via STT processing thread (aka consumer)

		const uint32_t read_position = m_read_head.load(std::memory_order_relaxed);
		const uint32_t write_position = m_write_head.load(std::memory_order_acquire);

		// Available space = capacity - 1 - current stored count
		const uint32_t available = (write_position - read_position + m_capacity_samples) % m_capacity_samples;
		const uint32_t to_read = std::min(a_sample_count, available);

		if (to_read == 0) {
			return 0; // Buffer full, so we drop the samples
		}

		// Read in up to 2 segments -- this handles wrap-around
		const uint32_t first_chunk = std::min(to_read, m_capacity_samples - read_position);
		std::memcpy(a_destination, m_audio_buffer.data() + read_position, first_chunk * sizeof(float));

		if (to_read > first_chunk) {
			const uint32_t second_chunk = to_read - first_chunk;
			std::memcpy(a_destination, m_audio_buffer.data() + first_chunk, second_chunk * sizeof(float));
		}

		
		m_read_head.store((read_position + to_read) % m_capacity_samples, std::memory_order_release);

		return to_read;
	}

	uint32_t CAudioRingBuffer::Available() const {
		const uint32_t write_position = m_write_head.load(std::memory_order_acquire);
		const uint32_t read_position = m_read_head.load(std::memory_order_acquire);
		return (write_position - read_position + m_capacity_samples) % m_capacity_samples;
	}

	void CAudioRingBuffer::Reset() {
		m_write_head.store(0, std::memory_order_release);
		m_write_head.store(0, std::memory_order_release);
	}
}