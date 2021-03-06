/* ksgl.h - kscript bindings for OpenGL
 *
 * 
 * SEE: https://www.opengl.org/
 * 
 * @author: Cade Brown <cade@kscript.org>
 */
#ifndef KSGL_H__
#define KSGL_H__


/* Default config */
#ifndef KSGL_GLFW
#define KSGL_GLFW
#endif


/* Module name */
#define M_NAME "gl"

/* kscript API */
#include <ks/ks.h>
#include <ks/nx.h>


/* OpenGL API (3.3), generated via gl3w */
#include <gl3w.h>


/* assimp, the asset importer */
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>



/** Types **/

/* gl.Shader(src_vert) - OpenGL shader program
 *
 */
typedef struct ksgl_shader_s {
    KSO_BASE
    
    /* OpenGL handle for the program 
     * Created via 'glCreateProgram()'
     */
    int val;

}* ksgl_shader;


/* gl.VBO(data='') - OpenGL vertex buffer object
 *
 */
typedef struct ksgl_vbo_s {
    KSO_BASE

    /* OpenGL handle for the buffer
     */
    int val;

}* ksgl_vbo;

/* gl.EBO(data='') - OpenGL element buffer object
 *
 */
typedef struct ksgl_ebo_s {
    KSO_BASE

    /* OpenGL handle for the element buffer
     */
    int val;

}* ksgl_ebo;

/* gl.VAO() - OpenGL vertex array object
 *
 */
typedef struct ksgl_vao_s {
    KSO_BASE

    /* OpenGL handle for the array buffer
     */
    int val;

}* ksgl_vao;


/* gl.texture2d() - OpenGL 2D texture
 *
 */
typedef struct ksgl_texture2d_s {
    KSO_BASE

    /* OpenGL handle for the array buffer
     */
    int val;

}* ksgl_texture2d;




/** Functions **/

/* Checks the last error, and if there has been an error, throws an exception and returns false
 */
bool ksgl_check();


/* Convert arguments to a color (RGBA)
 * 'out' should store '4' values
 */
bool ksgl_getcolor(int nargs, kso* args, ks_cfloat* out);



#ifdef KSGL_GLFW

/** gl.glfw submodule **/

/* GLFW (Window Library) */
#include <GLFW/glfw3.h>


/* gl.glfw.Monitor - GLFW monitor wrapper
 *
 */
typedef struct ksgl_glfw_monitor_s {
    KSO_BASE

    /* Wrapped value */
    GLFWmonitor* val;

}* ksgl_glfw_monitor;

/* gl.glfw.Window - GLFW window wrapper
 *
 */
typedef struct ksgl_glfw_window_s {
    KSO_BASE

    /* Wrapped value */
    GLFWwindow* val;

}* ksgl_glfw_window;


#endif

/** gl.ai (assimp) submodule **/


/* gl.ai.Node - Node in scene
 *
 */
typedef struct ksgl_ai_node_s {
    KSO_BASE

    /* Value being wrapped */
    C_STRUCT aiNode* val;

    /* Sub-nodes, children of this current node */
    ks_list sub;

}* ksgl_ai_node;

/* gl.ai.Mesh - Mesh data
 *
 */
typedef struct ksgl_ai_mesh_s {
    KSO_BASE

    /* Value being wrapped */
    C_STRUCT aiMesh* val;

}* ksgl_ai_mesh;


/* gl.ai.Scene - Total scene from imported data
 *
 */
typedef struct ksgl_ai_scene_s {
    KSO_BASE

    /* Value being wrapped */
    C_STRUCT aiScene* val;

    /* Source of the scene */
    ks_str src;

    /* Root node of the scene */
    ksgl_ai_node root;

    /* Meshes used in the scene */
    ks_list meshes;

}* ksgl_ai_scene;


ks_type
    ksglt_vbo,
    ksglt_ebo,
    ksglt_vao,
    ksglt_shader,
    ksglt_texture1d,
    ksglt_texture2d,
    ksglt_texture3d,

    ksgl_glfwt_monitor,
    ksgl_glfwt_window,

    ksgl_ait_scene,
    ksgl_ait_node,
    ksgl_ait_mesh
;


/** Internals **/

/* gl.glfw module */
ks_module _ksgl_glfw();
ks_module _ksgl_util();
ks_module _ksgl_ai();

void _ksgl_shader();
void _ksgl_texture2d();
void _ksgl_vbo();
void _ksgl_vao();
void _ksgl_ebo();

void _ksgl_glfw_monitor();
void _ksgl_glfw_window();

void _ksgl_ai_scene();
void _ksgl_ai_node();
void _ksgl_ai_mesh();


#endif /* KSGL_H__ */


