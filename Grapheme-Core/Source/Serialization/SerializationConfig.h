#pragma once

namespace Grapheme {
	struct GRAPHEME_API SSerializationConfig {
		int m_indent_width = 4;

		// Flags
		bool m_pretty_print			= true;
		bool m_include_timestamps	= true;
		bool m_include_confidence	= true;
		bool m_include_metadata		= true;

		// TODO:
		// bool m_use_json = true;
		// bool m_use_yaml = true;
	};
}