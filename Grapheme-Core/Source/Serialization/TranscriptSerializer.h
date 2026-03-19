#pragma once
#include "SerializationConfig.h"
#include "../STT/STTConfig.h"

#include <string>
#include <vector>

namespace Grapheme {
	// TODO: Add YAML support

	// Fwd declarations
	struct SLanguageResult;
	struct STranscriptResult;


	class CTranscriptSerializerImpl {
	private:
		SSerializationConfig m_config;

	public:
		explicit CTranscriptSerializerImpl(const SSerializationConfig& a_config = {});

		// File I/O
		EResultCode SaveToFile(const std::string& a_path, const STranscriptResult& a_result) const;
		EResultCode SaveToFile(const std::string& a_path, const std::vector<STranscriptResult>& a_results) const;

		// TODO: SLanguageResult
		STranscriptResult LoadFromFile(const std::string& a_path) const;

	private:
		// Serializes single results to JSON
		std::string Serialize(const STranscriptResult& a_result) const;
		std::string Serialize(const SLanguageResult& a_result) const;

		// Serializes multiple transcript results to JSON
		std::string Serialize(const std::vector<STranscriptResult>& a_results) const;

		// Deserialization
		STranscriptResult DeserializeTranscriptResult(const std::string& a_json_file_path) const;
		SLanguageResult DeserializeLanguageResult(const std::string& a_json_file_path) const;

	};

}
