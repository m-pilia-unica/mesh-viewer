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

/*!
 * \file components.h
 * @author Martino Pilia
 * @date 2015-01-29
 */

#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
    #include <GL/glext.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*! A well known mathematical constant. */
#define PI 3.14159

/*! Maximum string length. */
#define STR_LEN 200

/*!
 * Path of the directory containing models (with final separator).
 */
#if defined(__APPLE__) || defined(__linux__)
    #define MODEL_DIR "Model/"
#else
    #define MODEL_DIR "Model\\"
#endif // defined(__APPLE__) || defined(__linux__)

/*! Time between two consecutive angle updates. */
#define TIME_GAP 15

/*! Unit variation applied while changing the angular speed. */
#define ANGULAR_INCREMENT 7e-2f

/*! Maximum angular speed allowed (in deg/TIME_GAP). */
#define MAX_ANGULAR_SPEED 90.0f

/*! Minimum angular speed allowed (in deg/TIME_GAP). */
#define MIN_ANGULAR_SPEED .5f

/*! Coefficient defining the mouse drag speed when moving model. */
#define DRAG_SPEED 4.0f

/*! Coefficient defining mouse wheel speed when zooming model. */
#define WHEEL_SCROLL_SPEED 1.5f

/*! Unit variation applied while moving camera with arrow keys. */
#define ARROW_ROT_SPEED 2.5e-2f

/*! Unit variation applied while zooming with arrow keys. */
#define ARROW_ZOOM_SPEED .2f

/*! Maximum value for camera distance. */
#define MAX_RHO 100.0f

/*! Maximum value for camera latitude. */
#define MAX_PHI 90.0f

/*!
 * Coefficient defining initial distance from the camera to the center of
 * object's bounding box.
 */
#define INITIAL_DISTANCE_RATIO 5.0f

/*! Minimum bounding box radius value, above which the model is scaled up. */
#define MIN_BB_RADIUS 2.0f

/*!
 * This macro is a simple portable way to suppress unused variable warning
 * on most compilers. Note that this macro just suppress the warning
 * without side effects, but it does not provide any information to the
 * compiler on the effective usage of the variable or parameter: for such
 * purpose, see some C extension like the __unused__ value
 * for __attribute__.
 */
#define UNUSED(x) (void) (x)

// global vars declared as extern
// for actual definition see main.cpp

// light settings
extern const GLfloat light_ambient[];
extern const GLfloat light_diffuse[];
extern const GLfloat light_specular[];
extern const GLfloat light_position[];

// material settings
extern const GLfloat mat_ambient[];
extern const GLfloat mat_diffuse[];
extern const GLfloat mat_specular[];
extern const GLfloat high_shininess[];

/*!
 * Type defining the direction for camera movement.
 */
typedef enum Camera_movement
{
    MOVE_UP = 1,    /*!< Increment camera latitude. */
    MOVE_DOWN = 2,  /*!< Decrement camera latitude. */
    MOVE_LEFT = 3,  /*!< Decrement camera longitude. */
    MOVE_RIGHT = 4, /*!< Increment camera longitude. */
    MOVE_CLOSE = 5, /*!< Decrement camera radial distance. */
    MOVE_AWAY = 6   /*!< Increment camera radial distance. */
} Camera_movement;

/*!
 * Type defining the sign for the angular speed variation for the automatic
 * model rotation around its axis.
 */
typedef enum Speed_variation
{
    SPEED_INCREMENT = 1, /*!< Increment angular speed. */
    SPEED_DECREMENT = 2  /*!< Decrement angular speed. */
    
} Speed_variation;

/*! 
 * Type for the position of a point in tridimensional space expressed in 
 * polar coordinates. 
 */
typedef struct Polar_3D Polar_3D;

/*!
 * Type for the position of a point in tridimensional space expressed in
 * Cartesian coordinates.
 */
typedef struct Vector_3D Vector_3D;

/*!
 * Structure defining a point in tridimensional space, whose coordinates are
 * expressed in polar form.
 */
struct Polar_3D
{
    float theta; /*!< Longitude. */
    float phi;   /*!< Latitude. */
    float rho;   /*!< Radial distance from origin. */
};

/*!
 * Structure defining a point or a vector in tridimensional space, whose
 * coordinates are expressed in cartesian form.
 */
struct Vector_3D
{
    float x; /*!< X component. */
    float y; /*!< Y component. */
    float z; /*!< Z component. */
};

/*!
 * \brief Manage window resize.
 * @param w Width.
 * @param h heigth.
 */
void resize(int w, int h);

/*!
 * \brief Update the rotation angle of the model.
 * @param value Unused parameter.
 */
void updateAngle(int value);

/*!
 * \brief Adjust the rotation speed while a related key is pressed.
 * @param sign Sign of rotation variation.
 */
void adjust_rotation_speed(int dir);

/*!
 * \brief Move camera while a related key is pressed.
 * @param dir Direction for movement.
 */
void move_camera(int dir);

/*!
 * \brief Manage viewport content drawing.
 */
void display(void);

/*!
 * \brief Handle ASCII keypresses.
 * @param key Pressed key value.
 * @param x Mouse x position on keypress.
 * @param y Mouse y position on keypress.
 */
void key(unsigned char key, int x, int y);

/*!
 * \brief Handle ASCII key release.
 * @param key Released key value.
 * @param x Mouse x position on keypress.
 * @param y Mouse y position on keypress.
 */
void key_release(unsigned char key, int x, int y);

/*!
 * \brief Handle non-ASCII keypress.
 * @param key Pressed key value.
 * @param x Mouse x position on keypress.
 * @param y Mouse y position on keypress.
 */
void special_key_press(int key, int x, int y);

/*!
 * \brief Handle non-ASCII key release.
 * @param key Released key value.
 * @param x Mouse x position on keypress.
 * @param y Mouse y position on keypress.
 */
void special_key_release(int key, int x, int y);

/*!
 * \brief Handle mouse button presses.
 * @param button Identifier of the button currently pressed.
 * @param state State of the button. Glut indicates press with GLUT_DOWN,
 * release with GLUT_UP).
 * @param x Mouse x position on keypress.
 * @param y Mouse x position on keypress.
 */
void mouse(int button, int state, int x, int y);

/*!
 * \brief Handle mouse movement while no key is pressed.
 * @param x Mouse x position.
 * @param y Mouse y position.
 */
void passive_mouse_motion(int x, int y);

/*!
 * \brief Handle mouse movement while a key is pressed.
 * @param x Mouse x position.
 * @param y Mouse y position.
 */
void mouse_motion(int x, int y);

/*!
 * \brief Handle menu entry actions.
 * @param value Selected menu entry.
 */
void menuCallback(int value);

/*!
 * \brief Handle submenu entry actions.
 * @param value Selected submenu entry.
 */
void axisSubmenuCallback(int value);

/*!
 * \brief Create the GLUT menu.
 */
void createGLUTMenu(void);

/*!
 * \brief Parse the content from the desired file.
 * @param filename Name of the file to be parsed.
 * @param path Executable name with full path, i.e. argv[0] from the caller.
 * @return Zero if file was parsed successfully, nonzero otherwise.
 */
int parse_file(char *filename, char *path);

/*!
 * \brief Do some stuff needed for model initialization.
 */
void init_model(void);

/*!
 * \brief Get from user the desired filename to be imported.
 * @param filename String to be filled with the filename.
 * @note The filename string may have a maximum length of STR_LEN.
 */
void get_filename(char *filename);

/*!
 * \brief Handles fatal errors, writing a descriptive error message and
 * closing the program.
 * @param fun_name String representing the name of the function which failed.
 * @param caller Name of the caller of the failed function.
 * @param file Name of the file in which the failure happened.
 * @param line Line number of the failed function invocation.
 * @note This procedure causes the termination of the program.
 */
void error_handler(char *fun_name, const char *caller, char *file, int line);

/*!
 * \brief Drain the stdin buffer.
 * @note This function hangs, waiting for input, if stdin is already empty.
 */
void clear_stdin(void);

/*!
 * \brief Open file for OS X.
 * @param fn Name of the file.
 * @param dim_file String length of the filename parameter.
 * @param exe_path Value of argv[0].
 * @param mode Mode in which open the file.
 * @return The return value of fopen(FILE*, const char *) on the filename.
 */
FILE* osx_open_file(char *fn, int dim_file, char *exe_path, char *mode);
