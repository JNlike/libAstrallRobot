[English](../README.md) | [中文](README.zh-CN.md) | [日本語](README.ja.md) | [Français](README.fr.md) | [Deutsch](README.de.md)

# Astrall RobotDog Cpp

Ein C++17-Demoprojekt zur Steuerung eines Astrall-Roboterhundes mit dem Astrall SDK.

Dieses Projekt zeigt, wie das SDK initialisiert, Roboterdaten abonniert, Geräte- und Energiezustände gelesen, Heartbeat-Nachrichten gesendet und Bewegungen per Tastatur gesteuert werden.

## Funktionen

- C++-Projektstruktur auf Basis von CMake.
- Astrall-SDK-Integration über `lib/astrall_sdk`.
- Tastatursteuerung für Sportmodi, Bewegung, Autorisierung und Beleuchtung.
- Abonnement von IMU- und Bewegungsdaten.
- Periodische Ausgabe von Heartbeat, Geräteinformationen, Systemstatus, Sportstatus und Batteriestatus.
- Automatisches Kopieren der SDK-Runtime-Bibliotheken nach dem Build.

## Projektstruktur

```text
.
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── keyboard.cpp
│   └── keyboard.h
├── lib/
│   └── astrall_sdk/
│       ├── include/
│       ├── x86-64/
│       └── arm64/
└── docs/
```

## Voraussetzungen

- Linux-Umgebung
- CMake 3.10 oder neuer
- C++17-kompatibler Compiler
- POSIX-Thread-Unterstützung
- Astrall-Roboterhund und kompatible Astrall-SDK-Runtime

## Build

```bash
cmake -S . -B build
cmake --build build
```

Die ausführbare Datei wird als `sdk_demo` erzeugt. Die erforderlichen Astrall-SDK-Runtime-Bibliotheken werden nach dem Build in das Verzeichnis der ausführbaren Datei kopiert.

## Ausführen

```bash
./build/sdk_demo
```

Führen Sie das Programm in einem Terminal aus, das mit der Roboterumgebung verbunden ist. Die Demo liest Tastatureingaben direkt von der Standardeingabe.

## Tastatursteuerung

| Taste | Aktion |
| --- | --- |
| `1` | In den Damping-Modus wechseln |
| `2` | In den Fixed-Stand-Modus wechseln |
| `3` | In den Fixed-Down-Modus wechseln |
| `4` | In den Move-Modus wechseln |
| `5` | Automatisches Laden starten |
| `6` | Laden beenden |
| `9` | SDK-Steuerberechtigung anfordern |
| `0` | Bewegung stoppen |
| `w` / `s` | Vorwärtsgeschwindigkeit erhöhen / verringern |
| `a` / `d` | Seitliche Geschwindigkeit erhöhen / verringern |
| `q` / `e` | Gierrate erhöhen / verringern |
| `n` | Licht einschalten |
| `o` | Licht ausschalten |

Bewegungstasten wirken nur, wenn sich der Roboter im Move-Modus befindet.

## Hinweise

Verwenden Sie diese Demo vorsichtig in einem sicheren, offenen Bereich. Stellen Sie vor dem Aktivieren der Bewegung sicher, dass der Roboter über SDK-Steuerberechtigung verfügt und genügend Platz vorhanden ist.

## Lizenz

Siehe [LICENSE](../LICENSE).
