// =============================================================================
// modules/netscan_module.cpp — TCP connect-scan con sockets POSIX.
// -----------------------------------------------------------------------------
// Estrategia: socket() -> non-blocking -> connect() -> select() con timeout.
// Si select() reporta el socket escribible y SO_ERROR == 0  => puerto ABIERTO.
// Si connect() falla inmediatamente o el timeout expira    => puerto CERRADO.
// =============================================================================
#include "modules/netscan_module.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define EDUSEC_HAS_SOCKETS 1
#else
#define EDUSEC_HAS_SOCKETS 0
#endif

namespace edusec::netscan_module {

namespace {

constexpr int kTimeoutSeconds = 1;  // 1 segundo por puerto — suficiente en LAN/VM

std::vector<int> parse_ports(const std::string& csv) {
    std::vector<int> ports;
    std::stringstream ss(csv);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (item.empty()) continue;
        try {
            const int p = std::stoi(item);
            if (p > 0 && p <= 65535) ports.push_back(p);
        } catch (...) { /* ignora tokens inválidos */ }
    }
    return ports;
}

#if EDUSEC_HAS_SOCKETS

// Resuelve "host" (IP o nombre) a in_addr. Devuelve true si tuvo éxito.
bool resolve_host(const std::string& host, in_addr& out) {
    // Primero intento como dirección IP literal.
    if (inet_pton(AF_INET, host.c_str(), &out) == 1) return true;

    // Si no, resolución DNS.
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* res = nullptr;
    if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0 || res == nullptr) {
        return false;
    }
    out = reinterpret_cast<sockaddr_in*>(res->ai_addr)->sin_addr;
    freeaddrinfo(res);
    return true;
}

// Devuelve true si el puerto está abierto.
bool probe_port(const in_addr& addr, int port) {
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;

    // Modo no bloqueante para poder aplicar timeout con select().
    int flags = ::fcntl(sock, F_GETFL, 0);
    ::fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_port   = htons(static_cast<uint16_t>(port));
    target.sin_addr   = addr;

    bool open = false;
    const int rc = ::connect(sock, reinterpret_cast<sockaddr*>(&target), sizeof(target));

    if (rc == 0) {
        open = true;  // conexión inmediata
    } else if (errno == EINPROGRESS) {
        fd_set wset;
        FD_ZERO(&wset);
        FD_SET(sock, &wset);
        timeval tv{};
        tv.tv_sec  = kTimeoutSeconds;
        tv.tv_usec = 0;

        if (::select(sock + 1, nullptr, &wset, nullptr, &tv) > 0) {
            int so_err = 0;
            socklen_t len = sizeof(so_err);
            ::getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_err, &len);
            open = (so_err == 0);
        }
        // Si select() devuelve 0 -> timeout -> puerto cerrado/filtrado.
    }
    // Cualquier otro errno -> cerrado.

    ::close(sock);
    return open;
}

#endif  // EDUSEC_HAS_SOCKETS

}  // namespace

int run(const std::vector<std::string>& args) {
    std::string host;
    std::vector<int> ports;

    for (std::size_t i = 0; i + 1 < args.size(); i += 2) {
        const auto& flag  = args[i];
        const auto& value = args[i + 1];
        if (flag == "--host")  host  = value;
        if (flag == "--ports") ports = parse_ports(value);
    }

    if (host.empty() || ports.empty()) {
        std::cerr << "Uso: scan --host <ip|host> --ports <p1,p2,...>\n";
        return 2;
    }

#if !EDUSEC_HAS_SOCKETS
    std::cerr << "[!] netscan no soportado en esta plataforma.\n";
    return 1;
#else
    in_addr addr{};
    if (!resolve_host(host, addr)) {
        std::cerr << "[!] No se pudo resolver el host: " << host << '\n';
        return 1;
    }

    char ip_str[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &addr, ip_str, sizeof(ip_str));

    std::cout << "[scan] objetivo: " << host << " (" << ip_str << ")\n"
              << "----------------------------\n"
              << " PUERTO   ESTADO\n"
              << "----------------------------\n";

    int open_count = 0;
    for (int port : ports) {
        const bool open = probe_port(addr, port);
        if (open) ++open_count;
        std::cout << "  " << port
                  << std::string(port < 10 ? 6 : port < 100 ? 5 : port < 1000 ? 4 : port < 10000 ? 3 : 2, ' ')
                  << (open ? "ABIERTO" : "CERRADO") << '\n';
    }

    std::cout << "----------------------------\n"
              << "Resumen: " << open_count << " abiertos / "
              << ports.size() << " probados\n";
    return 0;
#endif
}

}  // namespace edusec::netscan_module
