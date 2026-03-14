#pragma once
#include "SerializationConfig.h"
#include "../STT/STTResults.h"
#include "../STT/STTConfig.h"
#include "../Core/ResultCodes.h"

#include <string>
#include <vector>

namespace Grapheme {
	// TODO: Add YAML support

	class CTranscriptSerializer {
	private:
		SSerializationConfig m_config;

	public:
		explicit CTranscriptSerializer(const SSerializationConfig& a_config = {});

		// Serializes single results to JSON
		std::string Serialize(const STranscriptResult& a_result) const;
		std::string Serialize(const SLanguageResult& a_result) const;

		// Serializes multiple transcript results to JSON
		std::string Serialize(const std::vector<STranscriptResult>& a_results) const;

		// File I/O
		EResultCode SaveToFile(const std::string& a_path, const STranscriptResult& a_result) const;
		EResultCode SaveToFile(const std::string& a_path, const std::vector<STranscriptResult>& a_results) const;

		// Deserialization
		STranscriptResult DeserializeTranscriptResult(const std::string& a_json_file_path) const;
		SLanguageResult DeserializeLanguageResult(const std::string& a_json_file_path) const;
		STranscriptResult LoadFromFile(const std::string& a_path) const;

	};

}
