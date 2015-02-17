/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) Martino Pilia, 2015
 */

#include "components.h"

// NOTE: extern variables definition; for declaration see components.hpp
// light settings
const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { -10.0f, 10.0f, 10.0f, 1.0f };

// material settings
const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

// viewer subroutine
int main(int argc, char *argv[])
{
    char filename[STR_LEN];
    int flag;

    // ask for filename, until a valid file is opened
    do
    {
        get_filename(filename);
        flag = parse_file(filename, argv[0]);
    } while (flag);

    // calculate bounding box center and radius, scale model if needed,
    // convert color into [0, 1] range
    init_model();


    glutInit(&argc, argv);
    glutInitWindowSize(1920, 1080);
    glutInitWindowPosition(10, 10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

    glutCreateWindow("3D view");

    // callback handlers association
    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutKeyboardUpFunc(key_release);
    glutMouseFunc(mouse);
    glutMotionFunc(mouse_motion);
    glutPassiveMotionFunc(passive_mouse_motion);
    glutSpecialFunc(special_key_press);
    glutSpecialUpFunc(special_key_release);
    glutIgnoreKeyRepeat(1); // ignore auto-repeated keystrokes

    createGLUTMenu();

    glClearColor(0, 0, 0, 1); // viewport background color (rgba)

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    // LIGHT0 properties
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    // launch function which updates angle for rotation
    // i.e. model rotation starts automatically on program launch
    glutTimerFunc(0, updateAngle, 0);

    glutMainLoop();

    return EXIT_SUCCESS;
}
