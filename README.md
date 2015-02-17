Mesh Viewer - PR1 Final Project
===============================
This program is part of the programming class final project. It permits to view 
a triangular mesh 3D model.

Features
========
This viewer permits to open triangular mesh in .ply format.
- automatic model rotation, with customizable direction and rotation axis;
- change rotation speed with '+' and '-' keys, start or stop with space key;
- free rotation of the model dragging with mouse left button or keyboard
  arrow keys;
- automatic model scale on loading;
- change zoom with PageDown and PageUp keys or with mouse scroll wheel
  (note that mouse scroll wheel is not supported on Mac OS X's default
  GLUT installation);
- context menu on mouse right button:
  + chose light: fixed respect to object or to observer;
  + chose visual mode: verices, boundary edges, filled faces;
  + chose rotation axis;
  + enable or disable color (if present);
- may contain traces of nuts or milk.

Build and run
=============
To build the project with gcc or a compatible compiler, launch the following     command in the project root directory
~~~~{.sh}
gcc -o ./bin/main main.c components.c -lglut -lGL -lGLU -lm        
~~~~
or similar command for other compilers. When compiled with the `__DEBUG__` 
macro defined (e.g. through the gcc's -D parameter) the application 
provides extra debug output.

If you have a working GNU make installation, you can build the project using     the makefile. Launch in the root folder:
~~~~{.sh}
make
~~~~

To build the project with extra debug output:
~~~~{.sh}
make debug
~~~~

To build the project documentation:
~~~~{.sh}
make doc
~~~~

Why GLUT?
=========
Because it was asked me to do so. Don't blame me, please.

License
=======
The project is licensed under GPL 3. See [LICENSE](/LICENSE) file for the full   license. 
