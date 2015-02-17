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

#include <math.h>
#include <float.h>
#include "components.h"

GLuint *indices = NULL;  //!< Vertexs indexes.
GLfloat *color = NULL;   //!< Vertexs colors.
GLfloat *vertexp = NULL; //!< Vertexs coordinates.
GLfloat *normals = NULL; //!< Vertexs normals.

int n_vertex = 0;  //!< Vertexes number.
int n_faces = 0;   //!< Faces number.
int isColored = 1; //!< Nonzero if model has color.
FILE *f_ply=NULL;  //!< Input file for model.

int rotate = 1; //!< Variable indicating wether the model is rotating.
int light_rotation = 0;  /*!< Variable indicating if the light is rotating
                              (i.e. fixed respect to the model) or fixed
                              (i.e. fixed respect to the observer). */

Vector_3D rotation_axis = {0, 1, 0}; //!< Components of the model rotation axis.
Polar_3D eye = {};        //!< Polar coordinates of the vieweing camera.
int moving_camera_h = 0;  /*!< Indicate wether left or right arrow
                               is pressed at the moment. */
int moving_camera_v = 0;  /*!< Indicate wether up or down arrow
                               is pressed at the moment. */
int moving_camera_r = 0;  /*!< Indicate wether PageUp or PageDown key
                               is pressed at the moment. */
int adjusting_speed = 0;  /*!< Indicate wether a key for the rotation speed
                               variation is pressed at the moment. */
float angle = 0;          //!< Current angle for model rotation.
float angularSpeed = 3;   //!< Current angular speed, in deg/TIME_GAP.
int rotation_sign = 1;    //!< Sign for the model rotation (right hand rule).
int displayColor = 1;     //!< True to display the model with colors.
Vector_3D center;         //!< Center of the model bounding box.
int l_button_pressed = 0; //!< True iff the mouse left button is pressed.
int last_x; //!< Last tracked x coordinate of the mouse position.
int last_y; //!< Last tracked y coordinate of the mouse position.

//! Maximum value reached by vertices for each coordinate.
float max_coord[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

//! Minumum value reached by vertices for each coordinate.
float min_coord[3] = {FLT_MAX, FLT_MAX, FLT_MAX};

float bb_radius; //!< Radius of the bounding box, defined as the distance
                 //!< between the center and a vertex of the bounding box.

/*!
 * Handle viewport resize.
 */
void resize(int w, int h)
{
    const float ar = (float) w / (float) h; // viewport aspect ratio
    glViewport(0, 0, w, h);                 // set viewport position and size

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
}

/*!
 * Update the angle used for model rotation around its rotation axis.
 * When called, the function increments the angle, then if the rotation
 * is enabled it schedules a next call of itself with a time gap.
 */
void updateAngle(int value)
{
    UNUSED(value); // suppress warning for unused parameter

    angle += angularSpeed; // update angle

    // reduce angle into [0, 360]
    // NOTE: the angle is always positive, sign is applied in the rotation
    if (angle > 360.0f)
        angle = fmod(angle, 360.0f);

    // schedule call with timing, only when rotation is active
    if(rotate)
        glutTimerFunc(TIME_GAP, updateAngle, 0);

    glutPostRedisplay(); // ask for viewport refresh
}

/*!
 * This function variates the rotation speed of the model, inside the interval
 * (`MIN_ANGULAR_SPEED`, `MAX_ANGULAR_SPEED`), of an amount equal to  
 * `ANGULAR_INCREMENT`, then it schedules a call of itself (with a `TIME_GAP`
 * delay) wether the key controlling rotation speed variation is still
 * pressed.
 */
void adjust_rotation_speed(int dir)
{
    if (dir == SPEED_INCREMENT && angularSpeed < MAX_ANGULAR_SPEED)
        angularSpeed += ANGULAR_INCREMENT;

    if (dir == SPEED_DECREMENT && angularSpeed > MIN_ANGULAR_SPEED)
        angularSpeed -= ANGULAR_INCREMENT;
        
    // schedule call iff key is still pressed
    if (adjusting_speed)
        glutTimerFunc(TIME_GAP, adjust_rotation_speed, dir);

    glutPostRedisplay(); // ask for viewport refresh
}

/*!
 * This function performs a small camera movement in the desired direction, 
 * whose size is specified by the `ARROW_ZOOM_SPEED` or `ARROW_ROT_SPEED`
 * macros, then calls itself (with a `TIME_GAP` delay) if the key for
 * the camera movement is still pressed.
 *
 * The camera position is mantained inside the interval
 * \f[ 
 *      (0,\, 2\pi) 
 *      \times (\frac{-\pi}{2},\, \frac{\pi}{2}) 
 *      \times (0,\, \text{MAX\_RHO}) 
 * \f]
 */
void move_camera(int dir)
{
    int still_moving; // schedule or not another timed call to move_camera

    switch (dir)
    {
        case MOVE_AWAY:
            if (eye.rho + ARROW_ZOOM_SPEED < MAX_RHO)
                eye.rho += ARROW_ZOOM_SPEED;
            still_moving = (moving_camera_r ? 1 : 0);
            break;

        case MOVE_CLOSE:
            if (eye.rho > ARROW_ROT_SPEED)
                eye.rho -= ARROW_ZOOM_SPEED;
            still_moving = (moving_camera_r ? 1 : 0);
            break;

        case MOVE_UP:
            if (eye.phi + ARROW_ROT_SPEED < PI / 2)
                eye.phi += ARROW_ROT_SPEED;
            still_moving = (moving_camera_v ? 1 : 0);
            break;

        case MOVE_DOWN:
            if (eye.phi - ARROW_ROT_SPEED > -PI / 2)
                eye.phi -= ARROW_ROT_SPEED;
            still_moving = (moving_camera_v ? 1 : 0);
            break;

        case MOVE_RIGHT:
            eye.theta += ARROW_ROT_SPEED;
            if (eye.theta > 2 * PI)
                eye.theta = fmod(eye.theta, 2 * PI);
            still_moving = (moving_camera_h ? 1 : 0);
            break;

        case MOVE_LEFT:
            eye.theta -= ARROW_ROT_SPEED;
            if (eye.theta < 0)
                eye.theta = fmod(eye.theta, 2 * PI);
            still_moving = (moving_camera_h ? 1 : 0);
            break;
    }

    // schedule call, iff key is still holded
    if (still_moving)
        glutTimerFunc(TIME_GAP, move_camera, dir);

    glutPostRedisplay(); // ask for viewport refresh
}

/*!
 * This function draws the objects on screen, and it is called everytime a
 * refresh of the viewport is needed.
 *
 * It moves the camera in the desired position (retrieved by mouse input),
 * then draws on screen the model. The model is translated in a way such the
 * center of its bounding box coincides with the origin, and then it's rotated.
 * Rotation angle is continuously updated by the updateAngle(int) procedure
 * while the automatic rotation of the model is active.
 *
 * The model is scaled up if it is too small, in order to permit adequate
 * zooming without cutting it with the front clipping plane.
 */
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();

    // move camera
    gluLookAt(
            /* camera position (from polar coordinate to cartesian) */
            eye.rho * sin(eye.theta) * cos(eye.phi),
            eye.rho * sin(eye.phi),
            eye.rho * cos(eye.theta) * cos(eye.phi),
            /* origin */
            0, 0, 0,
            /* up vector */
            0, 1, 0
            );

    // scale model if too small
    if (bb_radius < MIN_BB_RADIUS)
    {
        float f = MIN_BB_RADIUS / bb_radius;
        glScalef(f, f, f);
    }

    // rotate the model around rotation axis
    glRotatef(
            rotation_sign * angle,
            rotation_axis.x,
            rotation_axis.y,
            rotation_axis.z);

    // translate the bounding box center in the origin of axes
    glTranslatef(-center.x, -center.y, -center.z);

    glEnableClientState(GL_VERTEX_ARRAY); // utilizzare l'array dei vertici
    // color the model only if color is present and active
    if (isColored && displayColor)
        glEnableClientState(GL_COLOR_ARRAY);  // utilizzare l'array dei colori
    glEnableClientState(GL_NORMAL_ARRAY); // utilizzare l'array delle normali
    glVertexPointer(3, GL_FLOAT, 0, vertexp);
    glNormalPointer(GL_FLOAT, 0, normals);
    if(isColored && displayColor)
        glColorPointer(3, GL_FLOAT, 0, color);

    // draw model
    glDrawElements(GL_TRIANGLES, n_faces * 3, GL_UNSIGNED_INT, indices);

    // disable arrays
    glDisableClientState(GL_NORMAL_ARRAY);
    if (isColored)
        glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    // draw rotating light (if light is solidal with model)
    if (light_rotation)
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);

    glPopMatrix();

    // draw fixed ligh (if light is solidal with observer)
    if (!light_rotation)
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glutSwapBuffers();
}

/*!
 * Perform an action at the release of the specified ASCII key.
 * 
 * Note that rotation speed control is performed registering the pressure and 
 * release of the respective keys (with the `adjusting_speed` variable), 
 * and launching a function (adjust_rotation_speed(int)) which constantly 
 * increments or decrements angular speed only while the key is pressed.
 * This fancy way is more complex than the mere increment or decrement 
 * of a variable with a fixed value on keypress, but permits to have a smooth 
 * keyboard handling which is independent from typematic settings.
 * 
 * Repeated key pressure while holding the key down is indeed disabled, because
 * relying on that instead on the current approach provides a jerky movement
 * of the model and depends on system typematic settings, whose control is out
 * of the scope of this program.
 */
void key(unsigned char key, int x, int y)
{
    // suppress warnings for unused variables
    UNUSED(x);
    UNUSED(y);

    switch (key){
        // Tasto 'esc'
        case 27 :
        case 'q':
            exit(EXIT_SUCCESS);
            break;

        // increment rotation speed
        case '+':
            // ignore callback if related key is still holded
            if (adjusting_speed)
                return;
            // set flag and call specific function otherwise
            adjusting_speed = 1;
            adjust_rotation_speed(SPEED_INCREMENT);
            break;

        // decrement rotation speed
        case '-':
            // ignore callback if related key is still holded
            if (adjusting_speed)
                return;
            // set flag and call specific function otherwise
            adjusting_speed = 1;
            adjust_rotation_speed(SPEED_DECREMENT);
            break;

        // start-stop rotation
        case ' ':
            if (!rotate) // launch angle updating function when needed
                glutTimerFunc(TIME_GAP, updateAngle, 0);
            rotate = !rotate; // commute rotation status
            break;
    }

    glutPostRedisplay(); // ask for viewport refresh
}

/*!
 * Perform an action at the release of the specified ASCII key.
 */
void key_release(unsigned char key, int x, int y)
{
    // suppress warnings for unused parameters
    UNUSED(x);
    UNUSED(y);
   
    if (key == '+' || key == '-')
        adjusting_speed = 0; // register key release
}

/*!
 * Perform an action at the pressure of the specified special key.
 * 
 * The movement is referred to the model, so a movement in a direction 
 * generates a camera move in the opposite direction to obtain a model movement.
 * E.g. left arrow rotates the model left (counterclockwise respect to y axis)
 * through a camera movement on right (clockwise).
 * 
 * Note that camera movement is performed registering the pressure and 
 * release of the respective keys (with the `moving_camera_x` variables,
 * each one tracking one direction: horizontal, vertical, radial), 
 * and launching a function (move_camera(int)) which constantly 
 * moves the camera of a small value `ARROW_ROT_SPEED`, only while the 
 * key is pressed.
 * 
 * This fancy way is more complex than the mere increment or decrement 
 * of a variable with a fixed value on keypress, but permits to have a smooth 
 * keyboard handling which is independent from typematic settings.
 * 
 * Repeated key pressure while holding the key down is indeed disabled, because
 * relying on that instead on the current approach provides a jerky movement
 * of the model and depends on system typematic settings, whose control is out
 * of the scope of this program.
 */
void special_key_press(int key, int x, int y)
{
    // no action, just suppress warnings for unused parameters
    UNUSED(x);
    UNUSED(y);

    switch (key)
    {
        // decrement camera latitude, i.e. rotate object up
        case GLUT_KEY_UP:
            moving_camera_v = 1;
            move_camera(MOVE_DOWN); // camera movement opposite to model's one
            break;

        // increment camera latitude, i.e. rotate object down
        case GLUT_KEY_DOWN:
            moving_camera_v = 1;
            move_camera(MOVE_UP);
            break;

        // decrement camera longitude, i.e. rotate object counterclockwise
        case GLUT_KEY_LEFT:
            moving_camera_h = 1;
            move_camera(MOVE_RIGHT);
            break;

        // increment camera longitude, i.e. rotate object clockwise
        case GLUT_KEY_RIGHT:
            moving_camera_h = 1;
            move_camera(MOVE_LEFT);
            break;

        // increment camera radial position, i.e. move away
        case GLUT_KEY_PAGE_UP:
            moving_camera_r = 1;
            move_camera(MOVE_AWAY);
            break;

        // decrement camera radial position, i.e. move closer
        case GLUT_KEY_PAGE_DOWN:
            moving_camera_r = 1;
            move_camera(MOVE_CLOSE);
            break;
    }

    glutPostRedisplay(); // ask for viewport refresh
}

/*!
 * Perform an action at the release of the specified key.
 */
void special_key_release(int key, int x, int y)
{
    // no action, just suppress warnings for unused parameters
    UNUSED(x);
    UNUSED(y);

    // cease camera movement
    switch (key)
    {
        case GLUT_KEY_DOWN:
        case GLUT_KEY_UP:
            moving_camera_v = 0;
            break;

        case GLUT_KEY_RIGHT:
        case GLUT_KEY_LEFT:
            moving_camera_h = 0;
            break;

        case GLUT_KEY_PAGE_UP:
        case GLUT_KEY_PAGE_DOWN:
            moving_camera_r = 0;
            break;
    }

    glutPostRedisplay(); // ask for viewport refresh
}

/*!
 * This procedure handles the callbacks from mouse button events.
 */
void mouse(int button, int state, int x, int y)
{
    // suppress warnings for unused variables
    UNUSED(x);
    UNUSED(y);

    switch (button)
    {
        case GLUT_LEFT_BUTTON: // left button
            /* track button pressure */
            if (state == GLUT_DOWN)
                l_button_pressed = 1;
            else
                l_button_pressed = 0;
            break;

        case GLUT_RIGHT_BUTTON: // right button
            /* mapped to context menu yet */
            break;

        case 3: // wheel scroll up
            if (state == GLUT_UP)
                /* ignore, because each wheel scroll generates two
                 * callbacks, one with value GLUT_DOWN followed by one with
                 * value GLUT_UP */
                return;
            if (eye.rho < MAX_RHO)
                eye.rho += WHEEL_SCROLL_SPEED; // move camera away from model
            break;

        case 4: // wheel scroll down
            if (state == GLUT_UP)
                /* ignore, because each wheel scroll generates two
                 * callbacks, one with value GLUT_DOWN followed by one with
                 * value GLUT_UP */
                return;
            if (eye.rho > WHEEL_SCROLL_SPEED)
                eye.rho -= WHEEL_SCROLL_SPEED; // move camera closer to model
            break;
    }

    glutPostRedisplay(); // ask for viewport refresh
}

/*!
 * This procedure tracks mouse movements while no button is pressed. It is
 * useful to mantain the mouse position always updated and ready for drag
 * events.
 */
void passive_mouse_motion(int x, int y)
{
    last_x = x;
    last_y = y;
}

/*!
 * This procedure handles callbacks from mouse events while a button is
 * pressed. It is used to drag the model while left button is pressed.
 * 
 * Like arrow movements, the dragging is referred to a model movement, so it
 * generates an opposite camera movement. E.g. a bottom to top dragging 
 * is converted into a decrement of camera latitute; a left to right dragging 
 * is converted into a decrement of camera longitude.
 */
void mouse_motion(int x, int y)
{
    float delta_theta, delta_phi; // variables for angles variation

    if (l_button_pressed) // act only while left button is pressed
    {
        // set angles variation proportionally to the mouse movement extent,
        // according to viewport size and speed coefficient;
        // there is a factor 2 on theta variation because the total angle
        // to be covered is double respect to phi's one
        delta_theta =
            -(float) (x - last_x) / glutGet(GLUT_WINDOW_WIDTH) * 2 * DRAG_SPEED;
        delta_phi
            = (float) (y - last_y) / glutGet(GLUT_WINDOW_HEIGHT) * DRAG_SPEED;

        // mantain theta inside [-PI, PI] interval
        eye.theta += delta_theta;
        if (fabs(eye.theta) > PI)
            angle = fmod(angle, PI);

        // mantain phi inside [-PI/2, PI/2] interval
        if (eye.phi + delta_phi > -PI / 2 && eye.phi + delta_phi < PI / 2)
            eye.phi += delta_phi;

        // save last mouse position for next call
        last_x = x;
        last_y = y;

        glutPostRedisplay(); // ask for viewport refresh
    }
}

/*!
 * Menu handler.
 */
void menuCallback(int value)
{
    switch(value)
    {
        case 1:
                // cambia la podalità di visualizzazione settandola a wireframe
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                break;
        case 2:
                // show only vertices
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                break;
        case 3:
                // show filled surfaces
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                break;
        case 4:
                // commute color displaying setting
                displayColor = !displayColor;
                break;
        case 5:
                // commute light rotation
                light_rotation = !light_rotation;
                break;
        case 6:
                // commute rotation way
                rotation_sign *= -1;
                break;
        case 7:
                exit(EXIT_SUCCESS);
                break;
    }
}

/*!
 * Callback function for the axis choice submenu handling.
 */
void axisSubmenuCallback(int value)
{
    switch(value)
    {
        case 1:
                // x axis
                rotation_axis.x = 1;
                rotation_axis.y = 0;
                rotation_axis.z = 0;
                break;
        case 2:
                // y axis
                rotation_axis.x = 0;
                rotation_axis.y = 1;
                rotation_axis.z = 0;
                break;
        case 3:
                // z axis
                rotation_axis.x = 0;
                rotation_axis.y = 0;
                rotation_axis.z = 1;
                break;
        case 4:
                // commute x component
                rotation_axis.x = !rotation_axis.x;
                break;
        case 5:
                // commute x component
                rotation_axis.y = !rotation_axis.y;
                break;
        case 6:
                // commute x component
                rotation_axis.z = !rotation_axis.z;
                break;
    }
}

/*!
 * Create context menu.
 */
void createGLUTMenu()
{
    int menu, axis_submenu;

    // Create submenu for rotation axis choice and add its entries to it
    axis_submenu = glutCreateMenu(axisSubmenuCallback);
    glutAddMenuEntry("X axis", 1);
    glutAddMenuEntry("Y axis", 2);
    glutAddMenuEntry("Z axis", 3);
    glutAddMenuEntry("Commute X component", 4);
    glutAddMenuEntry("Commute Y component", 5);
    glutAddMenuEntry("Commute Z component", 6);

    // Questa istruzione mi permette di creare realmente il menu
    // associando una funzione di callback
    menu = glutCreateMenu(menuCallback);

    // Add menu entries
    glutAddMenuEntry("Show boundary edges", 1);
    glutAddMenuEntry("Show vertices", 2);
    glutAddMenuEntry("Show polygons surface", 3);
    if (isColored) // only if model file has color informations
        glutAddMenuEntry("Enable/disable color", 4);
    glutAddMenuEntry("Fixed/rotating light", 5);
    glutAddMenuEntry("Rotate clockwise/counterclockwise", 6);
    glutAddSubMenu("Rotation axis", axis_submenu);
    glutAddMenuEntry("Exit", 7);

    // menu associato al tasto destro
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

/*!
 * Read data from the model input file. Informations on vertices, normals,
 * color and faces are stored in three different dynamical arrays.
 */
int parse_file(char *filename, char *path)
{
    int i, j, k;
    int line;
    char tmp[STR_LEN + 1]; // buffer containing each token read from the file

    UNUSED(path); // no effect, only suppresses warnings for unused parameter

    // open file
    #if defined(__APPLE__)
    f_ply = openFile(filename, strlen(filename), path, strlen(path), "r");
    #else // __linux__, _WIN32
    f_ply = fopen(filename, "r");
    #endif // defined(__APPLE__)

    // error check for file opening
    if (f_ply == NULL)
    {
        printf("Unable to open the file %s.\n", filename);
        return -1;
    }

    // read first line from file and check that is a valid ply file
    fscanf(f_ply, "%[^\n]s", tmp);

    if (strcmp(tmp, "ply"))
    {
        printf("Not a valid .ply file.\n");
        return -1;
    }

    // scan header file for informations on model
    isColored = 0;
    do // while (strcmp(tmp, "end_header"))
    {
        // get next token
        fscanf(f_ply, "%s", tmp);

        if (!strcmp(tmp, "vertex"))
        {
            // get next token, containing face number
            fscanf(f_ply, "%d", &n_vertex);

            // allocate dynamical array for normals components
            line = __LINE__ + 1;
            normals = (GLfloat*) malloc(sizeof (GLfloat) * n_vertex * 3);
            if (normals == NULL)
                error_handler("malloc", __func__, __FILE__, line);

            // allocate dynamical array for vertices components
            line = __LINE__ + 1;
            vertexp = (GLfloat*) malloc(sizeof (GLfloat) * n_vertex * 3);
            if (vertexp == NULL)
                error_handler("malloc", __func__, __FILE__, line);
        }

        // check if model is colored
        if (!isColored && !strcmp(tmp, "red")) // do this only once
        {
            isColored = 1;

            // allocate dynamical array for color
            line = __LINE__ + 1;
            color = (GLfloat*) malloc(sizeof (GLfloat) * n_vertex * 3);
            if (color == NULL)
                error_handler("malloc", __func__, __FILE__, line);
        }

        if (!strcmp(tmp, "face"))
        {
            // get next token, containing face number
            fscanf(f_ply, "%d", &n_faces);

            // allocate dynamical array for faces indices
            line = __LINE__ + 1;
            indices = (GLuint*) malloc(sizeof (GLuint) * n_faces * 3);
            if (indices == NULL)
                error_handler("malloc", __func__, __FILE__, line);
        }
    } while (strcmp(tmp, "end_header"));
    
    // check if header contains useful declarations
    if (n_vertex == 0 || n_faces == 0)
    {
        printf("Invalid file header, nothing to draw is declared.\n");
        return -1;
    }        

    // read vertex, normals and color data:
    // i index iterates on vertices;
    // j index tracks the position of first of the three components for the
    //     current tuple in each array;
    // k index iterates inside each tuple.
    j = 0;
    for (i = 0; i < n_vertex; i++)
    {
        // get vertex coordinates
        for (k = 0; k < 3; ++k)
        {
            fscanf(f_ply, "%f", &vertexp[j + k]);

            // search greatest and smallest coords
            if (vertexp[j + k] > max_coord[k])
                max_coord[k] = vertexp[j + k];
            if (vertexp[j + k] < min_coord[k])
                min_coord[k] = vertexp[j + k];

            #ifdef __DEBUG__
            printf("%.2f ", vertexp[j + k]);
            #endif
        }

        #ifdef __DEBUG__
        printf("  ");
        #endif

        // get normal components
        for (k = 0; k < 3; ++k)
        {
            fscanf(f_ply, "%f", &normals[j + k]);
            #ifdef __DEBUG__
            printf("% .2f ", normals[j + k]);
            #endif
        }

        #ifdef __DEBUG__
        printf("  ");
        #endif

        // get color components
        if (isColored)
        {
            for (k = 0; k < 3; ++k)
            {
                fscanf(f_ply, "%f", &color[j + k]);
                #ifdef __DEBUG__
                printf("%.2f ", color[j + k]);
                #endif
            }
        }

        #ifdef __DEBUG__
        printf("\n");
        #endif

        j += 3;
    }

    // get faces data
    k = 0;
    for (i = 0; i < n_faces; ++i)
    {
        char buf[10];

        fscanf(f_ply, "%s", buf); // number of vertices per face (useless)

        // get indexes of vertices for each face
        for (j = 0; j < 3; ++j)
        {
            fscanf(f_ply, "%u", &indices[k++]);
            #ifdef __DEBUG__
            printf("%u ", indices[k - 1]);
            #endif
        }

        #ifdef __DEBUG__
        printf("\n");
        #endif
    }

    // release file
    fclose(f_ply);

    #ifdef __DEBUG__
    printf( "\n      min       max\n"
            "x: %f %f\n"
            "y: %f %f\n"
            "z: %f %f\n",
            min_coord[0],
            max_coord[0],
            min_coord[1],
            max_coord[1],
            min_coord[2],
            max_coord[2]
            );
    #endif // __DEBUG__

    return 0;
}

/*!
 * This procedure determines the bounding box center and radius. Radius
 * is also used to to setup initial camera position. If the model is colored,
 * this procedure converts the color from [0, 255] to [0, 1].
 */
void init_model(void)
{
    int i;

    // find the center of the model bounding box
    center.x = (max_coord[0] + min_coord[0]) / 2;
    center.y = (max_coord[1] + min_coord[1]) / 2;
    center.z = (max_coord[2] + min_coord[2]) / 2;

    // set bounding box radius
    bb_radius = 1.0f / 4.0f * sqrt(
              (max_coord[0] - min_coord[0]) * (max_coord[0] - min_coord[0])
            + (max_coord[1] - min_coord[1]) * (max_coord[1] - min_coord[1])
            + (max_coord[2] - min_coord[2]) * (max_coord[2] - min_coord[2]));

    // set initial eye position out of the bounding box
    eye.rho = INITIAL_DISTANCE_RATIO * bb_radius;

    // scale initial position for very small models
    if (bb_radius < MIN_BB_RADIUS)
        eye.rho *= MIN_BB_RADIUS / bb_radius;

    // convert color into [0,1]
    if (isColored)
        for(i = 0; i < n_vertex * 3; i++)
            color[i] /= 255;
}

/*!
 * This subroutine asks the user for the name of a file to open.
 */
void get_filename(char *fileName)
{
    int flag, choice, fname_len;
    char input[STR_LEN];
    char specifier[STR_LEN + 1];

    /* show menu with avaible files
     * note: files in menu have a one-based numeration */
    printf( "\nThe following model files are avaible:\n"
            "  1: buddha_n.ply\n"
            "  2: bunny_n.ply\n"
            "  3: cube_n.ply\n"
            "  4: heli_n.ply\n"
            "  5: swirl_n.ply\n"
            "  6: insert another filename manually\n");

    /* get input filename from user */
    do /* while (flag) */
    {
        flag = 0; /* set condition for exit: will be immutated if input
                     is valid */

        printf("Chose a file to edit, then press Return [1-6]: ");

        /* get desired file number to import */
        scanf("%d", &choice);
        clear_stdin();

        /* input validation */
        if (choice < 1 || choice > 6)
        {
            printf("\nInvalid file number. Please retry.\n");
            flag = 1;
        }
    } while (flag);

    /* copy actual filename */
    switch (choice)
    {
        case 1:
            strncpy(fileName, MODEL_DIR "buddha_n.ply", STR_LEN);
            break;

        case 2:
            strncpy(fileName, MODEL_DIR "bunny_n.ply", STR_LEN);
            break;

        case 3:
            strncpy(fileName, MODEL_DIR "cube_n.ply", STR_LEN);
            break;

        case 4:
            strncpy(fileName, MODEL_DIR "heli_n.ply", STR_LEN);
            break;

        case 5:
            strncpy(fileName, MODEL_DIR "swirl_n.ply", STR_LEN);
            break;

        case 6:
            /* custom filename */
            fname_len = STR_LEN - strlen(MODEL_DIR); // filename length
            printf("Insert filename (max. %d chars): ", fname_len);
            sprintf(specifier, "%%%ds", fname_len);
            scanf(specifier, &input);
            sprintf(fileName, "%s%s", MODEL_DIR, input);
            break;

        default:
            /* should be unreachable */
            perror("Something went seriously wrong");
            exit(EXIT_FAILURE);
            break;
    }
}

/*!
 * This procedure writes an adequately descriptive error message on
 * <code>stdout</code>, descibing the error encountered, then it
 * quits the program with <code>EXIT_FAILURE</code> status.
 * This procedure is intended to be used to write a standardized message
 * with few lines of code, after the check of the output from a function
 * call shows a failure, e.g.
 * \code
 *      line_no = __LINE__ + 1;
 *      m = malloc(n * sizeof(int));
 *
 *      if (m == NULL)
 *          error_handler("malloc", __func__, __FILE__, line_no);
 * \endcode
 */
void error_handler(char *fun_name, const char *caller, char *file, int line)
{
    char message[STR_LEN + 1];
    sprintf(message, "%s:%d: %s: %s() error",
            file,
            line,
            caller,
            fun_name);
    perror(message);
    exit(EXIT_FAILURE);
}

/*!
 * This procedure asks the <code>stdin</code> one character a time, until it
 * reaches <code>EOF</code> or it gets an <code>endline</code> character.
 * Note that this subroutine sticks, hanging for input, if it is called while
 * <code>stdin</code> is already empty.
 */
void clear_stdin(void)
{
    char c;
    while ((c = getchar()) != EOF && c != '\n') {}
}

/*!
 * Open file in OSX, using a path relative to the executable location.
 */
FILE* osx_open_file(char *fn, int dim_file, char *exe_path, char *mode)
{
    int path_len = strlen(exe_path);
    char new_path[path_len + dim_file + 1];
    int i = path_len;

    /* delete executable name, leaving only its containing directory */
    while (exe_path[i] != '/' && i > 0)
        i--;
    exe_path[i] = '\0';
    
    strcpy(new_path, exe_path);
    strcat(new_path, fn);
    
    return fopen(new_path, mode);
}
