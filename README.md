# Ball Physics Simulation

An animated terminal simulation of a ball affected by gravity. It bounces from the floor with energy loss on impact, settles naturally, and has a shadow that shows its horizontal position. The scene has no walls or ceiling.

Build and run from this folder:

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic simulation.c -o ballsim.exe -lm
.\ballsim.exe
```

Press `Q` or `Esc` to stop the simulation.
