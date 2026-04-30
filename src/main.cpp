// =============================================================================
// EduSec Toolkit — punto de entrada
// PIA PAC (Ene-Jun 2026) — Componente educativo de ciberseguridad en C++
// -----------------------------------------------------------------------------
// Despacha el subcomando recibido (hash | procs | scan) al módulo correspondiente.
// Uso exclusivo en VM aisladas; ver README.md para alcance y límites.
// =============================================================================

#include <iostream>
#include <string>
#include <vector>

#include "modules/hash_module.h"
#include "modules/proc_module.h"
#include "modules/netscan_module.h"

namespace {

void print_banner() {
    std::cout
        << "===============================================\n"
        << "  PIA PAC - GRUPO 01\n"
        << "  "
        << "===============================================\n";
}

void print_usage(const char* prog) {
    std::cout
        << "Uso: " << prog << " <subcomando> [opciones]\n\n"
        << "Subcomandos disponibles:\n"
        << "  hash    Genera hash FNV-1a de cadena o archivo\n"
        << "          --string <texto>   Hashea la cadena dada\n"
        << "          --file   <ruta>    Hashea el contenido del archivo\n\n"
        << "  procs   Enumera procesos del sistema (Linux, /proc)\n\n"
        << "  scan    TCP connect-scan a un host\n"
        << "          --host  <ip|host>  Host objetivo\n"
        << "          --ports <lista>    Puertos separados por coma (ej: 22,80,443)\n\n"
        << "  --help, -h    Muestra esta ayuda\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_banner();
        print_usage(argv[0]);
        return 0;
    }

    const std::string sub = argv[1];

    if (sub == "--help" || sub == "-h") {
        print_banner();
        print_usage(argv[0]);
        return 0;
    }

    // Empaqueta el resto de argumentos para el módulo
    std::vector<std::string> args;
    for (int i = 2; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    if (sub == "hash")  return edusec::hash_module::run(args);
    if (sub == "procs") return edusec::proc_module::run(args);
    if (sub == "scan")  return edusec::netscan_module::run(args);

    std::cerr << "[!] Subcomando desconocido: " << sub << "\n\n";
    print_usage(argv[0]);
    return 2;
}
