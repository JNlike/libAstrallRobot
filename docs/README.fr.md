[English](../README.md) | [中文](README.zh-CN.md) | [日本語](README.ja.md) | [Français](README.fr.md) | [Deutsch](README.de.md)

# Astrall RobotDog Cpp

Un projet de démonstration C++17 pour contrôler un robot chien Astrall avec le SDK Astrall.

Ce projet montre comment initialiser le SDK, s'abonner aux données de télémétrie du robot, lire les informations de l'appareil et l'état de l'alimentation, envoyer des messages heartbeat et contrôler les mouvements du robot au clavier.

## Fonctionnalités

- Structure de projet C++ basée sur CMake.
- Intégration du SDK Astrall via `lib/astrall_sdk`.
- Gestion du clavier pour les modes sportifs, le déplacement, l'autorisation et les lumières.
- Abonnement aux données IMU et aux données de mouvement.
- Affichage périodique du heartbeat, des informations de l'appareil, de l'état système, de l'état sportif et de l'état de la batterie.
- Copie automatique des bibliothèques runtime du SDK après la compilation.

## Structure du projet

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

## Prérequis

- Environnement Linux
- CMake 3.10 ou plus récent
- Compilateur compatible C++17
- Prise en charge des threads POSIX
- Robot chien Astrall et runtime SDK Astrall compatible

## Compilation

```bash
cmake -S . -B build
cmake --build build
```

L'exécutable est généré sous le nom `sdk_demo`. Les bibliothèques runtime Astrall SDK requises sont copiées dans le répertoire de l'exécutable après la compilation.

## Exécution

```bash
./build/sdk_demo
```

Exécutez le programme dans un terminal connecté à l'environnement du robot. La démonstration lit directement les entrées clavier depuis l'entrée standard.

## Commandes clavier

| Touche | Action |
| --- | --- |
| `1` | Passer en mode damping |
| `2` | Passer en mode fixed stand |
| `3` | Passer en mode fixed down |
| `4` | Passer en mode move |
| `5` | Démarrer la charge automatique |
| `6` | Quitter la charge |
| `9` | Demander l'autorité de contrôle SDK |
| `0` | Arrêter le mouvement |
| `w` / `s` | Augmenter / diminuer la vitesse avant |
| `a` / `d` | Augmenter / diminuer la vitesse latérale |
| `q` / `e` | Augmenter / diminuer la vitesse de lacet |
| `n` | Allumer la lumière |
| `o` | Éteindre la lumière |

Les touches de déplacement ne prennent effet que lorsque le robot est en mode move.

## Remarques

Utilisez cette démonstration avec prudence dans une zone sûre et dégagée. Vérifiez que le robot dispose de l'autorité de contrôle SDK et d'un espace suffisant avant d'activer le mouvement.

## Licence

Voir [LICENSE](../LICENSE).
