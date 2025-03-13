_______________________________________________________________________________
                                 README
_______________________________________________________________________________

FEATURES:
1) Model (DroneModel)
   - Stores the drone’s position, yaw, pitch, roll, and propeller angle.

2) View (DroneView)
   - Handles all rendering (cube for the drone body, sphere for the nose).

3) Controller (DroneController)
   - Responds to user input to update the drone’s state.

4) Multiple Cameras
   - Angled vantage, top-down, orbit, and first-person (prints position data to terminal so users can see it functioning).

5) Animations
   - Spinning propellers and 360° drone roll.

6) Keyboard Controls
   - Move, turn, reset, switch cameras, and roll.

7) Cross-Platform Makefile
   - Automatically detects macOS, Linux, or Windows (MinGW) and compiles 
     accordingly.

_______________________________________________________________________________

USAGE:

1) BUILD:
   - Run "make" (macOS/Linux) or "mingw32-make" (Windows MinGW).
   - Produces the executable "drone" (or "drone.exe" on Windows).

2) RUN:
   - On macOS/Linux: "./drone"
   - On Windows: "drone.exe"

3) CONTROLS:
   - UP/DOWN:    Pitch up/down
   - LEFT/RIGHT: Yaw left/right
   - "=" / "-":  Move forward/back
   - "F" / "S":  Increase/decrease propeller speed
   - "J":        Perform a 360-degree roll
   - "R":        Reset drone position/orientation
   - "0"/"1"/"2"/"3": Switch camera views
   - "ESC":      Quit

4) CLEAN:
   - "make clean" removes object files and the executable.

_______________________________________________________________________________

Distribution:

We worked colaboratively throughout this project with Aadi taking the lead on parts 2 and 4 and Ben taking lead on parts 1 and 3.
