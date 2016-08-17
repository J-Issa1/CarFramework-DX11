# DirectX 11 Racing Game Framework

A Engine Framework displaying the basics behind a car racing game.

Included in the project are:

* Player controlled car.
* AI controlled car.
* Simulated real-life physics (gravity, sliding, acceleration).
* Bounding Sphere Collision.

# The Player

The player is able to control the main car and they are able to move around the track freely. In order to represent a similar notion to real-life car, I have included a basic gearing system. 

# AI

In order to differentiate the AI car from the player, I decided to make the AI a blue car compared to the player's orange. The Ai car moves around the track via waypoints, which can be edited in a seperate txt file. This file is titled 'waypoints.txt'.

Inside the txt file, a new waypoint is signified on a new line starting with a 'w'. This is for the file parser to understand what information is being read in. Waypoints can be edited by changing the numbers. Waypoints are laid out in an 'x, y, z' coordinate system, and are seperated by a space.

I.e. 51.1540947 0.8 -36.5976219

# Cubes and Collision

Cubes are included to show off the collision and physics system included in the project. When dropped from any distance on the y axis, the cubes bounce and lose height, mirroring real-life bounce. This is due to gravity acting on the cubes. In this project, gravity is presumed to be -9.81.

Also included on the cubes are sliding force. 

