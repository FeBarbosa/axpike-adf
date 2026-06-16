#include "adele/adf/LowPrecisionSimulation/LowPrecisionTrace.h"

#include "sim.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <limits>
#include <mutex>
#include <sstream>
#include <string>

namespace {

struct TraceConfig {
    bool initialized = false;
    bool enabled = false;
    bool header_written = false;
    bool close_file = false;
    bool format_fp16 = true;
    bool format_e5m2 = true;
    bool width_fp32 = true;
    bool width_fp64 = true;
    uint64_t limit = 1000;
    uint64_t count = 0;
    FILE* file = stderr;
    std::mutex mutex;
};

TraceConfig trace_config;

bool envEnabled(const char* value) {
    return value != nullptr && value[0] != '\0' && std::strcmp(value, "0") != 0;
}

uint64_t parseLimit(const char* value, uint64_t fallback) {
    if (value == nullptr || value[0] == '\0') {
        return fallback;
    }

    char* end = nullptr;
    unsigned long long parsed = std::strtoull(value, &end, 10);
    if (end == value || *end != '\0') {
        return fallback;
    }

    return parsed;
}

bool tokenMatches(const std::string& list, const char* token) {
    size_t begin = 0;
    while (begin <= list.size()) {
        size_t end = list.find(',', begin);
        std::string item = list.substr(begin, end == std::string::npos ? std::string::npos : end - begin);
        if (item == token || item == "all") {
            return true;
        }
        if (end == std::string::npos) {
            break;
        }
        begin = end + 1;
    }

    return false;
}

std::string lowerString(const char* value) {
    std::string out = value == nullptr ? "" : value;
    for (char& c : out) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return out;
}

void initializeTrace() {
    if (trace_config.initialized) {
        return;
    }

    trace_config.enabled = envEnabled(std::getenv("AXPIKE_TRACE_LP_CONVERSIONS"));
    trace_config.limit = parseLimit(std::getenv("AXPIKE_TRACE_LP_LIMIT"), 1000);

    const char* trace_file = std::getenv("AXPIKE_TRACE_LP_FILE");
    if (trace_file != nullptr && trace_file[0] != '\0') {
        FILE* opened = std::fopen(trace_file, "w");
        if (opened != nullptr) {
            trace_config.file = opened;
            trace_config.close_file = true;
        }
    }

    const std::string formats = lowerString(std::getenv("AXPIKE_TRACE_LP_FORMAT"));
    if (!formats.empty()) {
        trace_config.format_fp16 = tokenMatches(formats, "fp16");
        trace_config.format_e5m2 = tokenMatches(formats, "e5m2");
    }

    const std::string widths = lowerString(std::getenv("AXPIKE_TRACE_LP_WIDTH"));
    if (!widths.empty()) {
        trace_config.width_fp32 = tokenMatches(widths, "fp32");
        trace_config.width_fp64 = tokenMatches(widths, "fp64");
    }

    trace_config.initialized = true;
}

bool shouldTrace(const char* format, const char* width) {
    if (!trace_config.enabled || trace_config.count >= trace_config.limit) {
        return false;
    }

    const std::string fmt = lowerString(format);
    const std::string w = lowerString(width);

    bool format_enabled = (fmt == "fp16" && trace_config.format_fp16) ||
                          (fmt == "e5m2" && trace_config.format_e5m2);
    bool width_enabled = (w == "fp32" && trace_config.width_fp32) ||
                         (w == "fp64" && trace_config.width_fp64);

    return format_enabled && width_enabled;
}

void writeHeader() {
    if (trace_config.header_written) {
        return;
    }

    std::fprintf(trace_config.file,
                 "seq\tformat\twidth\top\tpc\tinsn_id\traw_insn\tbefore_bits\tafter_bits\tbefore_value\tafter_value\tbefore_class\tafter_class\tabs_error\trel_error\n");
    trace_config.header_written = true;
}

const char* opName(source_t* source) {
    if (source == nullptr) {
        return "unknown";
    }
    if (source->op == source_t::READ) {
        return "regbank_read";
    }
    if (source->op == source_t::WRITE) {
        return "regbank_write";
    }
    return "unknown";
}

const char* valueClass(double value) {
    if (std::isnan(value)) {
        return "nan";
    }
    if (std::isinf(value)) {
        return "inf";
    }
    return "finite";
}

std::string hexBits(uint64_t bits, int width) {
    std::ostringstream out;
    out << "0x" << std::hex << std::setfill('0') << std::setw(width) << bits;
    return out.str();
}

float bitsToFloat(uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

double bitsToDouble(uint64_t bits) {
    double value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

void finiteErrors(double before, double after, double* abs_error, double* rel_error) {
    if (!std::isfinite(before) || !std::isfinite(after)) {
        *abs_error = std::numeric_limits<double>::quiet_NaN();
        *rel_error = std::numeric_limits<double>::quiet_NaN();
        return;
    }

    *abs_error = std::fabs(after - before);
    if (before == 0.0) {
        *rel_error = *abs_error == 0.0 ? 0.0 : std::numeric_limits<double>::infinity();
    } else {
        *rel_error = *abs_error / std::fabs(before);
    }
}

void traceLine(const char* format, const char* width, processor_t* p, source_t* source,
               uint64_t before_bits, uint64_t after_bits, int bit_width,
               double before_value, double after_value) {
    std::lock_guard<std::mutex> lock(trace_config.mutex);
    initializeTrace();
    if (!shouldTrace(format, width)) {
        return;
    }

    writeHeader();

    double abs_error;
    double rel_error;
    finiteErrors(before_value, after_value, &abs_error, &rel_error);

    uint64_t pc = 0;
    uint64_t raw_insn = 0;
    uint32_t insn_id = 0;
    if (p != nullptr) {
        pc = p->ax_control.pc;
        raw_insn = p->ax_control.insn.bits();
        insn_id = p->ax_control.cur_insn_id;
    }

    const int hex_width = bit_width / 4;
    std::fprintf(trace_config.file,
                 "%llu\t%s\t%s\t%s\t%s\t%u\t%s\t%s\t%s\t%.17g\t%.17g\t%s\t%s\t%.17g\t%.17g\n",
                 static_cast<unsigned long long>(trace_config.count),
                 format,
                 width,
                 opName(source),
                 hexBits(pc, 16).c_str(),
                 insn_id,
                 hexBits(raw_insn, 16).c_str(),
                 hexBits(before_bits, hex_width).c_str(),
                 hexBits(after_bits, hex_width).c_str(),
                 before_value,
                 after_value,
                 valueClass(before_value),
                 valueClass(after_value),
                 abs_error,
                 rel_error);

    trace_config.count++;
    std::fflush(trace_config.file);
}

} // namespace

void traceLowPrecisionConversionFP32(
        const char* format,
        processor_t* p,
        source_t* source,
        uint32_t before,
        uint32_t after
) {
    traceLine(format, "FP32", p, source, before, after, 32,
              static_cast<double>(bitsToFloat(before)),
              static_cast<double>(bitsToFloat(after)));
}

void traceLowPrecisionConversionFP64(
        const char* format,
        processor_t* p,
        source_t* source,
        uint64_t before,
        uint64_t after
) {
    traceLine(format, "FP64", p, source, before, after, 64,
              bitsToDouble(before),
              bitsToDouble(after));
}
