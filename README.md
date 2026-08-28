# Welcome to ThatCreeper (a.k.a. Creeper Host, Caden)'s Trijam Repository

If you're on the `jam-base` branch, this is the game engine that powers my games.

This is the repository that includes the source code for the following games:
| Game | Branch | Website | Description |
| ---- | ------ | ------- | ----------- |
| **Blocked** | `master` | https://creeper-host.itch.io/blocked | A sokobon with a mirror mechanic. |
| **Recovery** | `trijam300-postjam` | https://creeper-host.itch.io/recovery | A Minesweeper clone. |
| **Maze Game** | `trijam302` | https://creeper-host.itch.io/maze-game | A Flow Free clone. |
| **Webberton** | `trijam307-postjam` | https://creeper-host.itch.io/webberton | An awesome dungeon crawler. |
| **Shot** | `trijam309` | https://creeper-host.itch.io/shot | A strange, nigh painful Taiko clone. |
| **Line Crosser** | `trijam311` | https://creeper-host.itch.io/line-crosser | A physics sandbox game where you have to do like a chicken and get to the other side. |
| **Weaken Spot: a game about killing a baby but not a dog** | `trijam361` | https://creeper-host.itch.io/weaken-spot | An answer to the question, *"How bad does a Trijam game become when you get a bunch of people without gamedev experience to help you."* |

In addition, there are some failed experiments around the cracks:
| Branch | Description |
| ------ | ----------- |
| `dnreimp` | A failed object-oriented approach to the Trijam engine. Vaguely inspired by Deepnight (Sébastien Bénard)'s [GameBase engine](https://github.com/deepnight/gameBase). |
| `easing` | An expanded version of *Blocked*. |
| `lisp` | The beginning of an attempt to add Lisp-based scripting to the engine for... some reason. |
| `trijam314` | An abandoned platformer attempt. |

The engine is essentially just a pile of helpers over Raylib. This includes the LittleBigPlanet savestate system, some utilities for loading textures and sounds, and a simple and optional Entity/World system.
