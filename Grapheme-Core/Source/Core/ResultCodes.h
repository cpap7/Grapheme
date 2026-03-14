#pragma once
#include <cstdint>
#include <functional>
#include <string>

namespace Grapheme {
    // Common result codes
    enum class EResultCode : uint8_t {
        Unknown = 0,

        Success,
        NotInitialized,
        InvalidParameter,
        ModelLoadFailed,
        ProcessingFailed,
        Cancelled,
        OutOfMemory,
        InternalError

    };

    std::string ResultCodeToString(EResultCode a_result_code);

    EResultCode StringToResultCode(const std::string& a_string);

}