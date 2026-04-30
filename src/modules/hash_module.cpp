// =============================================================================
// modules/hash_module.cpp — Implementación FNV-1a/32 sobre cadena o archivo.
// =============================================================================
#include "modules/hash_module.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace edusec::hash_module {

namespace {
constexpr std::uint32_t kFnvOffset = 0x811C9DC5u;
constexpr std::uint32_t kFnvPrime  = 0x01000193u;
}  // namespace

std::uint32_t fnv1a_32(const void* data, std::size_t len) {
    const auto* bytes = static_cast<const std::uint8_t*>(data);
    std::uint32_t hash = kFnvOffset;
    for (std::size_t i = 0; i < len; ++i) {
        hash ^= bytes[i];
        hash *= kFnvPrime;
    }
    return hash;
}

std::string to_hex(std::uint32_t value) {
    std::ostringstream oss;
    oss << std::hex << std::setw(8) << std::setfill('0') << value;
    return oss.str();
}

namespace {

int hash_string(const std::string& text) {
    const std::uint32_t h = fnv1a_32(text.data(), text.size());
    std::cout << "FNV1a-32(string) = 0x" << to_hex(h) << '\n';
    return 0;
}

int hash_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::cerr << "[!] No se pudo abrir: " << path << '\n';
        return 1;
    }

    std::uint32_t hash = kFnvOffset;
    char buffer[4096];
    std::size_t total = 0;
    while (in.read(buffer, sizeof(buffer)) || in.gcount() > 0) {
        const auto got = static_cast<std::size_t>(in.gcount());
        for (std::size_t i = 0; i < got; ++i) {
            hash ^= static_cast<std::uint8_t>(buffer[i]);
            hash *= kFnvPrime;
        }
        total += got;
    }

    std::cout << "FNV1a-32(file) = 0x" << to_hex(hash)
              << "  (" << total << " bytes leídos: " << path << ")\n";
    return 0;
}

}  // namespace

int run(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Uso: hash --string <texto> | --file <ruta>\n";
        return 2;
    }

    const std::string& flag  = args[0];
    const std::string& value = args[1];

    if (flag == "--string") return hash_string(value);
    if (flag == "--file")   return hash_file(value);

    std::cerr << "[!] Flag desconocido para hash: " << flag << '\n';
    return 2;
}

}  // namespace edusec::hash_module
