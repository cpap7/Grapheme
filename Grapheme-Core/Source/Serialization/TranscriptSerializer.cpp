#include "gpch.h"
#include "TranscriptSerializer.h"
#include "../Core/ResultCodes.h"

#include "../STT/STTResults.h"

#include <json.hpp>

namespace Grapheme {
	CTranscriptSerializer::CTranscriptSerializer(const SSerializationConfig& a_config) 
		: m_config(a_config) {

	}

	std::string CTranscriptSerializer::Serialize(const STranscriptResult& a_result) const {
		nlohmann::json json;
		json["Text"]		= a_result.m_text;
		json["ResultCode"]	= ResultCodeToString(a_result.m_result_code);
		json["Success"]		= a_result.Success();

		if (m_config.m_include_confidence && !a_result.m_word_probabilities.empty()) {
			json["WordProbabilities"] = a_result.m_word_probabilities;
		}
		if (!a_result.m_part_indices.empty()) {
			json["PartIndices"] = a_result.m_part_indices;
		}

		return m_config.m_pretty_print ? json.dump(m_config.m_indent_width) : json.dump();
	}

	std::string CTranscriptSerializer::Serialize(const SLanguageResult& a_result) const {
		nlohmann::json json;
		json["LanguageCode"]	= a_result.m_language_code;
		json["Confidence"]		= a_result.m_confidence;
		json["ResultCode"]		= ResultCodeToString(a_result.m_result_code);
		json["Success"]			= a_result.Success();

		return m_config.m_pretty_print ? json.dump(m_config.m_indent_width) : json.dump();
	}
	
	std::string CTranscriptSerializer::Serialize(const std::vector<STranscriptResult>& a_results) const {
		nlohmann::json json_array = nlohmann::json::array();
		for (const auto& result : a_results) {
			json_array.push_back(nlohmann::json::parse(Serialize(result)));
		}

		return m_config.m_pretty_print ? json_array.dump(m_config.m_indent_width) : json_array.dump();
	}

	EResultCode CTranscriptSerializer::SaveToFile(const std::string& a_path, const STranscriptResult& a_result) const {
		std::ofstream file(a_path);
		if (!file.is_open()) {
			return EResultCode::InternalError;
		}
		file << Serialize(a_result);
		return file.good() ? EResultCode::Success : EResultCode::InternalError;
	}

	EResultCode CTranscriptSerializer::SaveToFile(const std::string& a_path, const std::vector<STranscriptResult>& a_results) const {
		std::ofstream file(a_path);
		if (!file.is_open()) {
			return EResultCode::InternalError;
		}
		file << Serialize(a_results);
		return file.good() ? EResultCode::Success : EResultCode::InternalError;
	}

	STranscriptResult CTranscriptSerializer::DeserializeTranscriptResult(const std::string& a_json_string) const {
		STranscriptResult result;
		try {
			nlohmann::json json		= nlohmann::json::parse(a_json_string);
			
			result.m_text			= json.value("Text", "");
			result.m_result_code	= StringToResultCode(json.value("ResultCode", "UNKNOWN"));
			if (json.contains("WordProbabilities")) {
				result.m_word_probabilities = json["WordProbabilities"].get<std::vector<float>>();
			}
			if (json.contains("PartIndices")) {
				result.m_part_indices = json["PartIndices"].get<std::vector<int>>();
			}
		}
		catch (const nlohmann::json::exception&) {
			result.m_result_code = EResultCode::ProcessingFailed;
		}

		return result;
	}

	SLanguageResult CTranscriptSerializer::DeserializeLanguageResult(const std::string& a_json_string) const {
		SLanguageResult result;
		try {
			nlohmann::json json = nlohmann::json::parse(a_json_string);

			result.m_language_code		= json.value("LanguageCode", "");
			result.m_confidence			= json.value("Confidence", 0.0f);
			result.m_result_code		= StringToResultCode(json.value("ResultCode", "UNKNOWN"));
		}
		catch (const nlohmann::json::exception&) {
			result.m_result_code = EResultCode::ProcessingFailed;
		}

		return result;
	}

	STranscriptResult CTranscriptSerializer::LoadFromFile(const std::string& a_path) const {
		std::ifstream file(a_path);
		if (!file.is_open()) {
			STranscriptResult result;
			result.m_result_code = EResultCode::InternalError;
			return result;
		}

		std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		return DeserializeTranscriptResult(contents);
	}
}