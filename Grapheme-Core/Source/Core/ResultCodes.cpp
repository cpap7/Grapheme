#include "gpch.h"
#include "ResultCodes.h"

namespace Grapheme {
    std::string ResultCodeToString(EResultCode a_result_code) {
        switch (a_result_code) {
            case EResultCode::Success:          return "SUCCESS";
            case EResultCode::NotInitialized:   return "NOT INITIALIZED";
            case EResultCode::InvalidParameter: return "INVALID PARAMETER(S)";
            case EResultCode::ModelLoadFailed:  return "MODEL LOAD FAILED";
            case EResultCode::ProcessingFailed: return "PROCESSING FAILED";
            case EResultCode::Cancelled:        return "CANCELLED";
            case EResultCode::OutOfMemory:      return "OUT OF MEMORY";
            case EResultCode::InternalError:    return "INTERNAL ERROR";
        }

        return "UNKNOWN RESULT CODE";
    }

    EResultCode StringToResultCode(const std::string& a_string) {
        if (a_string == "SUCCESS") { return EResultCode::Success; }
        if (a_string == "NOT INITIALIZED") { return EResultCode::NotInitialized; }
        if (a_string == "INVALID PARAMETER(S)") { return EResultCode::InvalidParameter; }
        if (a_string == "MODEL LOAD FAILED") { return EResultCode::ModelLoadFailed; }
        if (a_string == "PROCESSING FAILED") { return EResultCode::ProcessingFailed; }
        if (a_string == "CANCELLED") { return EResultCode::Cancelled; }
        if (a_string == "OUT OF MEMORY") { return EResultCode::OutOfMemory; }
        if (a_string == "INTERNAL ERROR") { return EResultCode::InternalError; }

        return EResultCode::Unknown;
    }
}