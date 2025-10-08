# A3: Shooter Born in Heaven (Run 'n Gun Game)

**Course:** Programming 2  
**Term:** 2025/1  
**Author:** Pietro Comin (GRR20241955)

> [!WARNING]
> **This was the most challenging and rewarding project of the semester, with over 50 hours of dedicated development.**  
> The result is a fully functional 2D *run 'n gun* action game inspired by classics like *Contra* and *Metal Slug*.

## 🕹️ Game Description

**Shooter Born in Heaven** is a 2D action game developed in C using the Allegro 5 library. The player takes on the role of an elite soldier who must advance through a hostile urban environment, facing waves of gangsters and culminating in an intense battle against a powerful final boss.

## 🚀 How to Compile and Run

The project uses a `makefile` to simplify the entire process.

**Requirements:**
* GCC (or any compatible C compiler)
* Make
* Allegro 5 library and its addons (image, font, ttf, primitives)

**Steps:**
1. Open a terminal in the project root.
2. Compile the game:
    ```bash
    make
    ```
3. Run the game:
    ```bash
    make run
    ```
4. Clean generated files:
    ```bash
    make clean
    ```

## ⌨️ Game Controls

| Action                     | Main Key                 | Alternative Key             |
| --------------------------- | ------------------------ | --------------------------- |
| **Move**                    | `A` / `D`                | Left Arrow / Right Arrow    |
| **Jump**                    | `W`                      | Up Arrow                    |
| **Crouch**                  | `S`                      | Down Arrow                  |
| **Run**                     | `Left Shift`             | -                           |
| **Shoot**                   | `Spacebar`               | `Right Shift`               |
| **Reload**                  | `R`                      | -                           |
| **Pause**                   | `P`                      | -                           |
| **Debug Mode (Hitbox)**     | `H`                      | -                           |
| **Exit / Back**             | `ESC`                    | `ESC`                       |

## ✨ Implemented Features

The game not only met all the minimum requirements but also included several extra features that enhanced the overall experience.

### Minimum Requirements (80/80 points)
* **Menus and Screens:** Main menu (Start, Options, Exit) and victory/defeat screens.  
* **Character and Animations:** Sprites and animations for all states (idle, walking, running, jumping, crouching, shooting).  
* **Full Mobility:** The player can walk, jump, crouch, and shoot in all directions and states.  
* **Combat:** Health system and projectile-based attacks for both player and enemies.  
* **Environment:** Background with side-scrolling (parallax) effect.  
* **Enemies:** A standard enemy (shooter) and a final boss with its own AI.  
* **Stage Structure:** Waves of enemies leading up to the boss fight.  

### Extra Features (from the Assignment)
* **[15 pts] Additional Common Enemy:** Implemented a melee-type enemy (`ENEMY_MELEE`).  
* **[10 pts] Crouch and Shoot While Crouched:** The player can defend and attack while crouched.  
* **[10 pts] Stamina System:** Running consumes stamina, which regenerates over time.  
* **[5 pts] Pause Button:** The game can be paused at any time.  

### Additional Features (Beyond the Assignment)
* **Ammo Reload System:** The player has limited ammo and must reload (`R`), adding a tactical element to combat.  
* **Enemy Wave System:** Enemies spawn in waves with increasing difficulty.  
* **Advanced Boss AI:** The boss features multiple attack patterns, a “Rage Mode” triggered at 50% health, and a “Poise” system making it resistant to stun.  
* **Post-Victory “Freeplay” Mode:** After winning, the player can continue playing the stage, and the boss can respawn for new challenges.  
* **Debug Mode:** The `H` key toggles hitbox visualization — an essential tool for development and balancing.

## 📂 Project Structure

The code was organized modularly to ensure proper separation of responsibilities.  
* `src/`: Contains all source files (.c).  
* `include/`: Contains all header files (.h).  
* `assets/`: Contains all graphical resources (sprites, backgrounds) and fonts.  
