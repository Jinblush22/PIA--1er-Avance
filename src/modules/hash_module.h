// =============================================================================
// modules/hash_module.h — Módulo de hashing educativo (FNV-1a/32)
// -----------------------------------------------------------------------------
// FNV-1a es un hash no criptográfico — se usa aquí como punto de partida
// portable y sin dependencias para demostrar la lectura de archivos y el
// procesamiento byte a byte. En Fase II se ampliará a SHA-256 (OpenSSL) y
// fuerza bruta sobre un diccionario.
// =============================================================================
#ifndef EDUSEC_MODULES_HASH_MODULE_H
#define EDUSEC_MODULES_HASH_MODULE_H

#include <cstdint>
#include <string>
#include <vector>

namespace edusec::hash_module {

// Punto de entrada del subcomando `hash`.
// Devuelve 0 si tuvo éxito, distinto de 0 en error.
int run(const std::vector<std::string>& args);

// Helpers expuestos para pruebas / reutilización.
std::uint32_t fnv1a_32(const void* data, std::size_t len);
std::string   to_hex(std::uint32_t value);

}  // namespace edusec::hash_module

#endif  // EDUSEC_MODULES_HASH_MODULE_H
