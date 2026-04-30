# Diseño técnico — EduSec Toolkit (PIA PAC)

## 1. Arquitectura general

`EduSec Toolkit` es una aplicación CLI monolítica escrita en C++17, organizada
como un dispatcher (`main.cpp`) y un conjunto de módulos independientes
(`modules/`) seleccionables por subcomando. Cada módulo es responsable de una
técnica de ciberseguridad y expone una única función `run(args)` para mantener
una interfaz uniforme y un acoplamiento mínimo con `main.cpp`.

```
+------------------+        argv[1] = subcomando
|   main.cpp       | ---------------------------+
|  (dispatcher)    |                            |
+------------------+                            v
                              ┌────────────┬────────────┬────────────┐
                              │ hash_mod   │ proc_mod   │ netscan_mod│
                              └────────────┴────────────┴────────────┘
```

## 2. Módulos planeados y su función

| Módulo            | Responsabilidad                                                      | Avance 1 |
|-------------------|----------------------------------------------------------------------|----------|
| `hash_module`     | Hashing FNV-1a/32 sobre cadenas y archivos (lectura por bloques)     | ✓        |
| `proc_module`     | Enumeración de procesos en Linux leyendo `/proc/<pid>/comm`          | ✓        |
| `netscan_module`  | TCP connect-scan con sockets POSIX no bloqueantes y `select()`       | ✓ básico |
| `bruteforce` (F2) | Diccionario sobre hashes generados por `hash_module`                 | —        |
| `sniffer` (F2)    | Captura pasiva con `libpcap` para análisis didáctico                 | —        |
| `analyzer` (F2)   | Hooks para análisis estático (Ghidra/Radare2) y dinámico (x64dbg)    | —        |

## 3. Flujo general del programa

1. `main.cpp` valida `argc`/`argv`. Sin argumentos o con `--help`, imprime el
   banner y la ayuda.
2. Empaqueta `argv[2..]` en `std::vector<std::string>` y despacha al módulo
   según el subcomando (`hash` | `procs` | `scan`).
3. El módulo parsea sus flags y ejecuta la técnica:
   - `hash_module` itera bytes y aplica FNV-1a/32.
   - `proc_module` lista `/proc/<pid>` y lee `comm`.
   - `netscan_module` resuelve el host con `getaddrinfo()`, abre socket no
     bloqueante por puerto, llama `connect()` y espera con `select()` hasta
     un timeout de 1 s; consulta `SO_ERROR` para decidir ABIERTO/CERRADO.
4. `main` propaga el `int rc` devuelto como código de salida del proceso.

```
[start] -> parse argv -> [help?] -- yes --> print_usage -> [end]
                              |
                              no
                              v
                    dispatch(subcomando)
                              |
       +----------+-----------+-----------+
       v          v                       v
    hash_mod   proc_mod               netscan_mod
       |          |                       |
       +----------+-----------+-----------+
                              v
                        rc -> [end]
```

## 4. Dependencias técnicas

**Avance 1 (actual):** únicamente la librería estándar de C++17 y POSIX
(`<dirent.h>` para `proc_module`; `<sys/socket.h>`, `<netdb.h>`, `<fcntl.h>`,
`<sys/select.h>` para `netscan_module`). No requiere paquetes externos: el
proyecto compila con `g++ -std=c++17` en cualquier distro Linux moderna.

**Fase II (planeadas):**

- `libpcap-dev` para captura de paquetes (`sniffer`).
- `libssl-dev` (OpenSSL) para SHA-256/MD5 reales en `hash_module`.
- `libcap-ng` opcional para gestión de capabilities en lugar de root pleno.
- Herramientas de análisis externas: Ghidra, Radare2, x64dbg, `strace`, `ltrace`.

**Entorno de pruebas:** VMs aisladas (Kali Linux + Ubuntu Server o Windows
target controlado) sin interfaz puente hacia la red local.
