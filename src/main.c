/* main.c - module implementation for OpenGL bindings
 *
 * @author: Cade Brown <cade@kscript.org>
 */
#include <ksgl.h>
#include <ks/cext.h>


/* Internals */

/* Cross product */
static void my_cross(ks_cfloat Ax, ks_cfloat Ay, ks_cfloat Az, ks_cfloat Bx, ks_cfloat By, ks_cfloat Bz, ks_cfloat *Cx, ks_cfloat *Cy, ks_cfloat *Cz) {
    *Cx = Ay * Bz - Az * By;
    *Cy = Az * Bx - Ax * Bz;
    *Cz = Ax * By - Ay * Bx;
}


/* C-API */


/* Module functions */

static KS_TFUNC(M, translate) {
    ks_cfloat x, y, z;
    KS_ARGS("x:cfloat y:cfloat z:cfloat", &x, &y, &z);

    /* Result 4x4 matrix */
    nx_F res[4][4];

    int i, j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            res[i][j] = 0.0;
        }
    }

    res[0][0] = 1.0;
    res[1][1] = 1.0;
    res[2][2] = 1.0;
    res[3][3] = 1.0;

    res[0][3] = x;
    res[1][3] = y;
    res[2][3] = z;
    return (kso)nx_array_newc(nxt_array, &res[0][0], nxd_F, 2, (ks_size_t[]){ 4, 4 }, NULL);
}

static KS_TFUNC(M, scale) {
    ks_cfloat x, y, z;
    KS_ARGS("x:cfloat y:cfloat z:cfloat", &x, &y, &z);

    /* Result 4x4 matrix */
    nx_F res[4][4];

    int i, j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            res[i][j] = 0.0;
        }
    }

    res[0][0] = x;
    res[1][1] = y;
    res[2][2] = z;
    res[3][3] = 1.0;

    return (kso)nx_array_newc(nxt_array, &res[0][0], nxd_F, 2, (ks_size_t[]){ 4, 4 }, NULL);
}

static KS_TFUNC(M, lookat) {
    kso pos, target;
    kso up = KSO_NONE;
    KS_ARGS("pos target ?up", &pos, &target, &up);

    ks_cfloat px, py, pz, tx, ty, tz;
    ks_cfloat ux = 0, uy = 1, uz = 0;

    if (up != KSO_NONE) {
        ks_list uv = ks_list_newi(up);
        if (!uv) {
            return NULL;
        }
        if (uv->len != 3) {
            KS_THROW(kst_SizeError, "Expected 'up' to be of length 3, but got length %i", (int)uv->len);
            KS_DECREF(uv);
            return NULL;
        }

        if (!kso_get_cf(uv->elems[0], &ux) || !kso_get_cf(uv->elems[1], &uy) || !kso_get_cf(uv->elems[2], &uz)) {
            KS_DECREF(uv);
            return NULL;
        }

        KS_DECREF(uv);
    } 
    ks_list pv = ks_list_newi(pos);
    if (!pos) {
        return NULL;
    }
    if (pv->len != 3) {
        KS_THROW(kst_SizeError, "Expected 'pos' to be of length 3, but got length %i", (int)pv->len);
        KS_DECREF(pv);
        return NULL;
    }

    if (!kso_get_cf(pv->elems[0], &px) || !kso_get_cf(pv->elems[1], &py) || !kso_get_cf(pv->elems[2], &pz)) {
        KS_DECREF(pos);
        return NULL;
    }

    KS_DECREF(pv);

    ks_list tv = ks_list_newi(target);
    if (!tv) {
        return NULL;
    }
    if (tv->len != 3) {
        KS_THROW(kst_SizeError, "Expected 'target' to be of length 3, but got length %i", (int)tv->len);
        KS_DECREF(tv);
        return NULL;
    }

    if (!kso_get_cf(tv->elems[0], &tx) || !kso_get_cf(tv->elems[1], &ty) || !kso_get_cf(tv->elems[2], &tz)) {
        KS_DECREF(tv);
        return NULL;
    }

    KS_DECREF(tv);


    /* Result 4x4 matrix */
    nx_F res[4][4];
    int i, j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            res[i][j] = 0.0;
        }
    }

    /* Z-direction */
    ks_cfloat dx = tx - px, dy = ty - py, dz = tz - pz;
    ks_cfloat nrm = sqrt(dx * dx + dy * dy + dz * dz);

    dx = -dx / nrm;
    dy = -dy / nrm;
    dz = -dz / nrm;

    res[2][0] = dx;
    res[2][1] = dy;
    res[2][2] = dz;

    /* New X direction */
    ks_cfloat Xx, Xy, Xz;
    my_cross(ux, uy, uz, dx, dy, dz, &Xx, &Xy, &Xz);

    nrm = sqrt(Xx*Xx + Xy*Xy + Xz*Xz);
    Xx /= nrm;
    Xy /= nrm;
    Xz /= nrm;

    /* New Y diretion */
    ks_cfloat Yx, Yy, Yz;
    my_cross(dx, dy, dz, Xx, Xy, Xz, &Yx, &Yy, &Yz);

    nrm = sqrt(Yx*Yx + Yy*Yy + Yz*Yz);
    Yx /= nrm;
    Yy /= nrm;
    Yz /= nrm;

    res[0][0] = Xx;
    res[0][1] = Xy;
    res[0][2] = Xz;

    res[1][0] = Yx;
    res[1][1] = Yy;
    res[1][2] = Yz;
    
    res[3][3] = 1.0;
    

    /* Add offsets */
    res[0][3] = -(Xx*px + Xy*py + Xz*pz);
    res[1][3] = -(Yx*px + Yy*py + Yz*pz);
    res[2][3] = -(dx*px + dy*py + dz*pz);

    return (kso)nx_array_newc(nxt_array, &res[0][0], nxd_F, 2, (ks_size_t[]){ 4, 4 }, NULL);

}

static KS_TFUNC(M, perspective) {
    ks_cfloat fov, aspect;
    ks_cfloat Znear = 0.15, Zfar = 150.0;
    KS_ARGS("fov:cfloat aspect:cfloat ?Znear:cfloat ?Zfar:cfloat", &fov, &aspect, &Znear, &Zfar);

    /* Result 4x4 matrix */
    nx_F res[4][4];
    ks_cfloat tan_fov_2 = tan(fov / 2.0);

    int i, j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            res[i][j] = 0.0;
        }
    }

    res[0][0] = 1.0 / (aspect * tan_fov_2);
    res[1][1] = 1.0 / (tan_fov_2);
    res[2][2] = -(Zfar + Znear) / (Zfar - Znear);
    res[2][3] = -(2 * Zfar * Znear) / (Zfar - Znear);

    res[3][2] = -1.0;
    
    /* I don't think this is needed */
    //res[3][3] = +1.0;

    return (kso)nx_array_newc(nxt_array, &res[0][0], nxd_F, 2, (ks_size_t[]){ 4, 4 }, NULL);
}


/** OpenGL wrapper functions **/

static KS_TFUNC(M, enable) {
    ks_cint cap;
    KS_ARGS("cap:cint", &cap);

    glEnable(cap);

    return KSO_NONE;
}

static KS_TFUNC(M, disable) {
    ks_cint cap;
    KS_ARGS("cap:cint", &cap);

    glDisable(cap);

    return KSO_NONE;
}

static KS_TFUNC(M, clear) {
    ks_cint flags;
    KS_ARGS("flags:cint", &flags);

    glClear(flags);

    return KSO_NONE;
}

static KS_TFUNC(M, clearColor) {
    int nargs;
    kso* args;
    KS_ARGS("*args", &nargs, &args);

    ks_cfloat val[4];
    if (!ksgl_getcolor(nargs, args, val)) {
        return NULL;
    }

    glClearColor(val[0], val[1], val[2], val[3]);

    return KSO_NONE;
}

static KS_TFUNC(M, viewport) {
    ks_cint x, y, w, h;
    KS_ARGS("x:cint y:cint w:cint h:cint", &x, &y, &w, &h);

    glViewport(x, y, w, h);    

    return KSO_NONE;
}


static KS_TFUNC(M, polygon_mode) {
    ks_cint face, mode = GL_FILL;
    KS_ARGS("face:cint ?mode:cint", &face, &mode);

    glPolygonMode(face, mode);

    return KSO_NONE;
}


/*** Drawing Commands ***/

static KS_TFUNC(M, draw_arrays) {
    ks_cint mode, num, offset = 0;
    KS_ARGS("mode:cint num:cint ?offset:cint", &mode, &num, &offset);

    glDrawArrays(mode, offset, num); 

    return KSO_NONE;
}

static KS_TFUNC(M, draw_elements) {
    ks_cint mode, num, type, byteoffset = 0;
    KS_ARGS("mode:cint num:cint type:cint ?byteoffset:cint", &mode, &num, &type, &byteoffset);

    glDrawElements(mode, num, type, (void*)byteoffset);

    return KSO_NONE;
}




/* Export */

static ks_module get() {
    /* OpenGL initialization */
    

    /* Initialize through gl3w */
    gl3wInit();
    if (gl3wIsSupported(3, 3) != 0) {
        KS_THROW(kst_Error, "Failed to initialize OpenGL v3.3");
        return NULL;
    }

#ifdef KSGL_GLFW
    ks_module res_glfw = _ksgl_glfw();
    if (!res_glfw) {
        return NULL;
    }
#endif

    ks_module res_util = _ksgl_util();
    if (!res_util) {
        KS_DECREF(res_glfw);
        return NULL;
    }
    ks_module res_ai = _ksgl_ai();
    if (!res_util) {
        KS_DECREF(res_glfw);
        KS_DECREF(res_util);
        return NULL;
    }

    ks_str k = ks_str_new(-1, "nx");
    ks_module m = ks_import(k);
    KS_DECREF(k);
    if (!m) {
        return NULL;
    }

    _ksgl_shader();

    _ksgl_texture2d();

    _ksgl_vbo();
    _ksgl_ebo();
    _ksgl_vao();

    ks_module res = ks_module_new(M_NAME, "", "OpenGL bindings for kscript", KS_IKV(

        /* Submodules */
#ifdef KSGL_GLFW
        {"glfw",  (kso)res_glfw},
#endif

        {"ai",  (kso)res_ai},
        {"util",  (kso)res_util},


        /* Constants */
        
        /* Types */
        {"Shader",  (kso)ksglt_shader},

        {"Texture2D",  (kso)ksglt_texture2d},

        {"EBO",  (kso)ksglt_ebo},
        {"VBO",  (kso)ksglt_vbo},
        {"VAO",  (kso)ksglt_vao},

        /* Functions */


        {"translate",              ksf_wrap(M_translate_, M_NAME ".translate(x, y, z)", "Creates a translation matrix shifting off 'x', 'y', and 'z' respectively")},
        {"scale",                  ksf_wrap(M_scale_, M_NAME ".scale(x, y, z)", "Creates a scaling matrix scaling with 'x', 'y', and 'z' respectively")},

        {"lookat",                 ksf_wrap(M_lookat_, M_NAME ".lookat(pos, target, up=(0, 1, 0))", "Creates a view matrix for the camera which is located at 'pos', and looking at 'target', with the up direction being 'up'")},
        {"perspective",            ksf_wrap(M_perspective_, M_NAME ".perspective(fov, aspect, Znear=0.15, Zfar=150)", "Creates a perspective matrix with the given field of view (in radians), aspect ratio (w / h), and clipping planes")},


        {"enable",                 ksf_wrap(M_enable_, M_NAME ".enable(cap)", "Enables a feature in OpenGL")},
        {"disable",                ksf_wrap(M_disable_, M_NAME ".disable(cap)", "Disables a feature in OpenGL")},
        {"clear",                  ksf_wrap(M_clear_, M_NAME ".clear(flags)", "Clears 'flags' (which should be a bitmask of OpenGL flags)")},
        {"clear_color",            ksf_wrap(M_clearColor_, M_NAME ".clearColor(*args)", "Sets the clear color to '*args', which should be the RGBA components (default: black)")},

        {"viewport",               ksf_wrap(M_viewport_, M_NAME ".viewport(x, y, w, h)", "Set the viewport rendering range")},

        {"polygon_mode",           ksf_wrap(M_polygon_mode_, M_NAME "polygon_mode(face, mode=gl.FILL)", "Set the polygon rendering mode")},

        {"draw_arrays",            ksf_wrap(M_draw_arrays_, M_NAME ".draw_arrays(mode, num, offset=0)", "Draws primitives from the currently bound vao")},
        {"draw_elements",           ksf_wrap(M_draw_elements_, M_NAME ".draw_elements(mode, num, type, byteoffset=0)", "Draws primitives from the currently bound VAO's EBO")},

    ));

    /* Generated enumeration */
    ks_type E_gl = ks_enum_make("gl", KS_EIKV(
#ifdef GL_DEPTH_BUFFER_BIT
  {"DEPTH_BUFFER_BIT", GL_DEPTH_BUFFER_BIT},
#endif
#ifdef GL_STENCIL_BUFFER_BIT
  {"STENCIL_BUFFER_BIT", GL_STENCIL_BUFFER_BIT},
#endif
#ifdef GL_COLOR_BUFFER_BIT
  {"COLOR_BUFFER_BIT", GL_COLOR_BUFFER_BIT},
#endif
#ifdef GL_FALSE
  {"FALSE", GL_FALSE},
#endif
#ifdef GL_TRUE
  {"TRUE", GL_TRUE},
#endif
#ifdef GL_POINTS
  {"POINTS", GL_POINTS},
#endif
#ifdef GL_LINES
  {"LINES", GL_LINES},
#endif
#ifdef GL_LINE_LOOP
  {"LINE_LOOP", GL_LINE_LOOP},
#endif
#ifdef GL_LINE_STRIP
  {"LINE_STRIP", GL_LINE_STRIP},
#endif
#ifdef GL_TRIANGLES
  {"TRIANGLES", GL_TRIANGLES},
#endif
#ifdef GL_TRIANGLE_STRIP
  {"TRIANGLE_STRIP", GL_TRIANGLE_STRIP},
#endif
#ifdef GL_TRIANGLE_FAN
  {"TRIANGLE_FAN", GL_TRIANGLE_FAN},
#endif
#ifdef GL_QUADS
  {"QUADS", GL_QUADS},
#endif
#ifdef GL_NEVER
  {"NEVER", GL_NEVER},
#endif
#ifdef GL_LESS
  {"LESS", GL_LESS},
#endif
#ifdef GL_EQUAL
  {"EQUAL", GL_EQUAL},
#endif
#ifdef GL_LEQUAL
  {"LEQUAL", GL_LEQUAL},
#endif
#ifdef GL_GREATER
  {"GREATER", GL_GREATER},
#endif
#ifdef GL_NOTEQUAL
  {"NOTEQUAL", GL_NOTEQUAL},
#endif
#ifdef GL_GEQUAL
  {"GEQUAL", GL_GEQUAL},
#endif
#ifdef GL_ALWAYS
  {"ALWAYS", GL_ALWAYS},
#endif
#ifdef GL_ZERO
  {"ZERO", GL_ZERO},
#endif
#ifdef GL_ONE
  {"ONE", GL_ONE},
#endif
#ifdef GL_SRC_COLOR
  {"SRC_COLOR", GL_SRC_COLOR},
#endif
#ifdef GL_ONE_MINUS_SRC_COLOR
  {"ONE_MINUS_SRC_COLOR", GL_ONE_MINUS_SRC_COLOR},
#endif
#ifdef GL_SRC_ALPHA
  {"SRC_ALPHA", GL_SRC_ALPHA},
#endif
#ifdef GL_ONE_MINUS_SRC_ALPHA
  {"ONE_MINUS_SRC_ALPHA", GL_ONE_MINUS_SRC_ALPHA},
#endif
#ifdef GL_DST_ALPHA
  {"DST_ALPHA", GL_DST_ALPHA},
#endif
#ifdef GL_ONE_MINUS_DST_ALPHA
  {"ONE_MINUS_DST_ALPHA", GL_ONE_MINUS_DST_ALPHA},
#endif
#ifdef GL_DST_COLOR
  {"DST_COLOR", GL_DST_COLOR},
#endif
#ifdef GL_ONE_MINUS_DST_COLOR
  {"ONE_MINUS_DST_COLOR", GL_ONE_MINUS_DST_COLOR},
#endif
#ifdef GL_SRC_ALPHA_SATURATE
  {"SRC_ALPHA_SATURATE", GL_SRC_ALPHA_SATURATE},
#endif
#ifdef GL_NONE
  {"NONE", GL_NONE},
#endif
#ifdef GL_FRONT_LEFT
  {"FRONT_LEFT", GL_FRONT_LEFT},
#endif
#ifdef GL_FRONT_RIGHT
  {"FRONT_RIGHT", GL_FRONT_RIGHT},
#endif
#ifdef GL_BACK_LEFT
  {"BACK_LEFT", GL_BACK_LEFT},
#endif
#ifdef GL_BACK_RIGHT
  {"BACK_RIGHT", GL_BACK_RIGHT},
#endif
#ifdef GL_FRONT
  {"FRONT", GL_FRONT},
#endif
#ifdef GL_BACK
  {"BACK", GL_BACK},
#endif
#ifdef GL_LEFT
  {"LEFT", GL_LEFT},
#endif
#ifdef GL_RIGHT
  {"RIGHT", GL_RIGHT},
#endif
#ifdef GL_FRONT_AND_BACK
  {"FRONT_AND_BACK", GL_FRONT_AND_BACK},
#endif
#ifdef GL_NO_ERROR
  {"NO_ERROR", GL_NO_ERROR},
#endif
#ifdef GL_INVALID_ENUM
  {"INVALID_ENUM", GL_INVALID_ENUM},
#endif
#ifdef GL_INVALID_VALUE
  {"INVALID_VALUE", GL_INVALID_VALUE},
#endif
#ifdef GL_INVALID_OPERATION
  {"INVALID_OPERATION", GL_INVALID_OPERATION},
#endif
#ifdef GL_OUT_OF_MEMORY
  {"OUT_OF_MEMORY", GL_OUT_OF_MEMORY},
#endif
#ifdef GL_CW
  {"CW", GL_CW},
#endif
#ifdef GL_CCW
  {"CCW", GL_CCW},
#endif
#ifdef GL_POINT_SIZE
  {"POINT_SIZE", GL_POINT_SIZE},
#endif
#ifdef GL_POINT_SIZE_RANGE
  {"POINT_SIZE_RANGE", GL_POINT_SIZE_RANGE},
#endif
#ifdef GL_POINT_SIZE_GRANULARITY
  {"POINT_SIZE_GRANULARITY", GL_POINT_SIZE_GRANULARITY},
#endif
#ifdef GL_LINE_SMOOTH
  {"LINE_SMOOTH", GL_LINE_SMOOTH},
#endif
#ifdef GL_LINE_WIDTH
  {"LINE_WIDTH", GL_LINE_WIDTH},
#endif
#ifdef GL_LINE_WIDTH_RANGE
  {"LINE_WIDTH_RANGE", GL_LINE_WIDTH_RANGE},
#endif
#ifdef GL_LINE_WIDTH_GRANULARITY
  {"LINE_WIDTH_GRANULARITY", GL_LINE_WIDTH_GRANULARITY},
#endif
#ifdef GL_POLYGON_MODE
  {"POLYGON_MODE", GL_POLYGON_MODE},
#endif
#ifdef GL_POLYGON_SMOOTH
  {"POLYGON_SMOOTH", GL_POLYGON_SMOOTH},
#endif
#ifdef GL_CULL_FACE
  {"CULL_FACE", GL_CULL_FACE},
#endif
#ifdef GL_CULL_FACE_MODE
  {"CULL_FACE_MODE", GL_CULL_FACE_MODE},
#endif
#ifdef GL_FRONT_FACE
  {"FRONT_FACE", GL_FRONT_FACE},
#endif
#ifdef GL_DEPTH_RANGE
  {"DEPTH_RANGE", GL_DEPTH_RANGE},
#endif
#ifdef GL_DEPTH_TEST
  {"DEPTH_TEST", GL_DEPTH_TEST},
#endif
#ifdef GL_DEPTH_WRITEMASK
  {"DEPTH_WRITEMASK", GL_DEPTH_WRITEMASK},
#endif
#ifdef GL_DEPTH_CLEAR_VALUE
  {"DEPTH_CLEAR_VALUE", GL_DEPTH_CLEAR_VALUE},
#endif
#ifdef GL_DEPTH_FUNC
  {"DEPTH_FUNC", GL_DEPTH_FUNC},
#endif
#ifdef GL_STENCIL_TEST
  {"STENCIL_TEST", GL_STENCIL_TEST},
#endif
#ifdef GL_STENCIL_CLEAR_VALUE
  {"STENCIL_CLEAR_VALUE", GL_STENCIL_CLEAR_VALUE},
#endif
#ifdef GL_STENCIL_FUNC
  {"STENCIL_FUNC", GL_STENCIL_FUNC},
#endif
#ifdef GL_STENCIL_VALUE_MASK
  {"STENCIL_VALUE_MASK", GL_STENCIL_VALUE_MASK},
#endif
#ifdef GL_STENCIL_FAIL
  {"STENCIL_FAIL", GL_STENCIL_FAIL},
#endif
#ifdef GL_STENCIL_PASS_DEPTH_FAIL
  {"STENCIL_PASS_DEPTH_FAIL", GL_STENCIL_PASS_DEPTH_FAIL},
#endif
#ifdef GL_STENCIL_PASS_DEPTH_PASS
  {"STENCIL_PASS_DEPTH_PASS", GL_STENCIL_PASS_DEPTH_PASS},
#endif
#ifdef GL_STENCIL_REF
  {"STENCIL_REF", GL_STENCIL_REF},
#endif
#ifdef GL_STENCIL_WRITEMASK
  {"STENCIL_WRITEMASK", GL_STENCIL_WRITEMASK},
#endif
#ifdef GL_VIEWPORT
  {"VIEWPORT", GL_VIEWPORT},
#endif
#ifdef GL_DITHER
  {"DITHER", GL_DITHER},
#endif
#ifdef GL_BLEND_DST
  {"BLEND_DST", GL_BLEND_DST},
#endif
#ifdef GL_BLEND_SRC
  {"BLEND_SRC", GL_BLEND_SRC},
#endif
#ifdef GL_BLEND
  {"BLEND", GL_BLEND},
#endif
#ifdef GL_LOGIC_OP_MODE
  {"LOGIC_OP_MODE", GL_LOGIC_OP_MODE},
#endif
#ifdef GL_DRAW_BUFFER
  {"DRAW_BUFFER", GL_DRAW_BUFFER},
#endif
#ifdef GL_READ_BUFFER
  {"READ_BUFFER", GL_READ_BUFFER},
#endif
#ifdef GL_SCISSOR_BOX
  {"SCISSOR_BOX", GL_SCISSOR_BOX},
#endif
#ifdef GL_SCISSOR_TEST
  {"SCISSOR_TEST", GL_SCISSOR_TEST},
#endif
#ifdef GL_COLOR_CLEAR_VALUE
  {"COLOR_CLEAR_VALUE", GL_COLOR_CLEAR_VALUE},
#endif
#ifdef GL_COLOR_WRITEMASK
  {"COLOR_WRITEMASK", GL_COLOR_WRITEMASK},
#endif
#ifdef GL_DOUBLEBUFFER
  {"DOUBLEBUFFER", GL_DOUBLEBUFFER},
#endif
#ifdef GL_STEREO
  {"STEREO", GL_STEREO},
#endif
#ifdef GL_LINE_SMOOTH_HINT
  {"LINE_SMOOTH_HINT", GL_LINE_SMOOTH_HINT},
#endif
#ifdef GL_POLYGON_SMOOTH_HINT
  {"POLYGON_SMOOTH_HINT", GL_POLYGON_SMOOTH_HINT},
#endif
#ifdef GL_UNPACK_SWAP_BYTES
  {"UNPACK_SWAP_BYTES", GL_UNPACK_SWAP_BYTES},
#endif
#ifdef GL_UNPACK_LSB_FIRST
  {"UNPACK_LSB_FIRST", GL_UNPACK_LSB_FIRST},
#endif
#ifdef GL_UNPACK_ROW_LENGTH
  {"UNPACK_ROW_LENGTH", GL_UNPACK_ROW_LENGTH},
#endif
#ifdef GL_UNPACK_SKIP_ROWS
  {"UNPACK_SKIP_ROWS", GL_UNPACK_SKIP_ROWS},
#endif
#ifdef GL_UNPACK_SKIP_PIXELS
  {"UNPACK_SKIP_PIXELS", GL_UNPACK_SKIP_PIXELS},
#endif
#ifdef GL_UNPACK_ALIGNMENT
  {"UNPACK_ALIGNMENT", GL_UNPACK_ALIGNMENT},
#endif
#ifdef GL_PACK_SWAP_BYTES
  {"PACK_SWAP_BYTES", GL_PACK_SWAP_BYTES},
#endif
#ifdef GL_PACK_LSB_FIRST
  {"PACK_LSB_FIRST", GL_PACK_LSB_FIRST},
#endif
#ifdef GL_PACK_ROW_LENGTH
  {"PACK_ROW_LENGTH", GL_PACK_ROW_LENGTH},
#endif
#ifdef GL_PACK_SKIP_ROWS
  {"PACK_SKIP_ROWS", GL_PACK_SKIP_ROWS},
#endif
#ifdef GL_PACK_SKIP_PIXELS
  {"PACK_SKIP_PIXELS", GL_PACK_SKIP_PIXELS},
#endif
#ifdef GL_PACK_ALIGNMENT
  {"PACK_ALIGNMENT", GL_PACK_ALIGNMENT},
#endif
#ifdef GL_MAX_TEXTURE_SIZE
  {"MAX_TEXTURE_SIZE", GL_MAX_TEXTURE_SIZE},
#endif
#ifdef GL_MAX_VIEWPORT_DIMS
  {"MAX_VIEWPORT_DIMS", GL_MAX_VIEWPORT_DIMS},
#endif
#ifdef GL_SUBPIXEL_BITS
  {"SUBPIXEL_BITS", GL_SUBPIXEL_BITS},
#endif
#ifdef GL_TEXTURE_1D
  {"TEXTURE_1D", GL_TEXTURE_1D},
#endif
#ifdef GL_TEXTURE_2D
  {"TEXTURE_2D", GL_TEXTURE_2D},
#endif
#ifdef GL_TEXTURE_WIDTH
  {"TEXTURE_WIDTH", GL_TEXTURE_WIDTH},
#endif
#ifdef GL_TEXTURE_HEIGHT
  {"TEXTURE_HEIGHT", GL_TEXTURE_HEIGHT},
#endif
#ifdef GL_TEXTURE_BORDER_COLOR
  {"TEXTURE_BORDER_COLOR", GL_TEXTURE_BORDER_COLOR},
#endif
#ifdef GL_DONT_CARE
  {"DONT_CARE", GL_DONT_CARE},
#endif
#ifdef GL_FASTEST
  {"FASTEST", GL_FASTEST},
#endif
#ifdef GL_NICEST
  {"NICEST", GL_NICEST},
#endif
#ifdef GL_BYTE
  {"BYTE", GL_BYTE},
#endif
#ifdef GL_UNSIGNED_BYTE
  {"UNSIGNED_BYTE", GL_UNSIGNED_BYTE},
#endif
#ifdef GL_SHORT
  {"SHORT", GL_SHORT},
#endif
#ifdef GL_UNSIGNED_SHORT
  {"UNSIGNED_SHORT", GL_UNSIGNED_SHORT},
#endif
#ifdef GL_INT
  {"INT", GL_INT},
#endif
#ifdef GL_UNSIGNED_INT
  {"UNSIGNED_INT", GL_UNSIGNED_INT},
#endif
#ifdef GL_FLOAT
  {"FLOAT", GL_FLOAT},
#endif
#ifdef GL_STACK_OVERFLOW
  {"STACK_OVERFLOW", GL_STACK_OVERFLOW},
#endif
#ifdef GL_STACK_UNDERFLOW
  {"STACK_UNDERFLOW", GL_STACK_UNDERFLOW},
#endif
#ifdef GL_CLEAR
  {"CLEAR", GL_CLEAR},
#endif
#ifdef GL_AND
  {"AND", GL_AND},
#endif
#ifdef GL_AND_REVERSE
  {"AND_REVERSE", GL_AND_REVERSE},
#endif
#ifdef GL_COPY
  {"COPY", GL_COPY},
#endif
#ifdef GL_AND_INVERTED
  {"AND_INVERTED", GL_AND_INVERTED},
#endif
#ifdef GL_NOOP
  {"NOOP", GL_NOOP},
#endif
#ifdef GL_XOR
  {"XOR", GL_XOR},
#endif
#ifdef GL_OR
  {"OR", GL_OR},
#endif
#ifdef GL_NOR
  {"NOR", GL_NOR},
#endif
#ifdef GL_EQUIV
  {"EQUIV", GL_EQUIV},
#endif
#ifdef GL_INVERT
  {"INVERT", GL_INVERT},
#endif
#ifdef GL_OR_REVERSE
  {"OR_REVERSE", GL_OR_REVERSE},
#endif
#ifdef GL_COPY_INVERTED
  {"COPY_INVERTED", GL_COPY_INVERTED},
#endif
#ifdef GL_OR_INVERTED
  {"OR_INVERTED", GL_OR_INVERTED},
#endif
#ifdef GL_NAND
  {"NAND", GL_NAND},
#endif
#ifdef GL_SET
  {"SET", GL_SET},
#endif
#ifdef GL_TEXTURE
  {"TEXTURE", GL_TEXTURE},
#endif
#ifdef GL_COLOR
  {"COLOR", GL_COLOR},
#endif
#ifdef GL_DEPTH
  {"DEPTH", GL_DEPTH},
#endif
#ifdef GL_STENCIL
  {"STENCIL", GL_STENCIL},
#endif
#ifdef GL_STENCIL_INDEX
  {"STENCIL_INDEX", GL_STENCIL_INDEX},
#endif
#ifdef GL_DEPTH_COMPONENT
  {"DEPTH_COMPONENT", GL_DEPTH_COMPONENT},
#endif
#ifdef GL_RED
  {"RED", GL_RED},
#endif
#ifdef GL_GREEN
  {"GREEN", GL_GREEN},
#endif
#ifdef GL_BLUE
  {"BLUE", GL_BLUE},
#endif
#ifdef GL_ALPHA
  {"ALPHA", GL_ALPHA},
#endif
#ifdef GL_RGB
  {"RGB", GL_RGB},
#endif
#ifdef GL_RGBA
  {"RGBA", GL_RGBA},
#endif
#ifdef GL_POINT
  {"POINT", GL_POINT},
#endif
#ifdef GL_LINE
  {"LINE", GL_LINE},
#endif
#ifdef GL_FILL
  {"FILL", GL_FILL},
#endif
#ifdef GL_KEEP
  {"KEEP", GL_KEEP},
#endif
#ifdef GL_REPLACE
  {"REPLACE", GL_REPLACE},
#endif
#ifdef GL_INCR
  {"INCR", GL_INCR},
#endif
#ifdef GL_DECR
  {"DECR", GL_DECR},
#endif
#ifdef GL_VENDOR
  {"VENDOR", GL_VENDOR},
#endif
#ifdef GL_RENDERER
  {"RENDERER", GL_RENDERER},
#endif
#ifdef GL_VERSION
  {"VERSION", GL_VERSION},
#endif
#ifdef GL_EXTENSIONS
  {"EXTENSIONS", GL_EXTENSIONS},
#endif
#ifdef GL_NEAREST
  {"NEAREST", GL_NEAREST},
#endif
#ifdef GL_LINEAR
  {"LINEAR", GL_LINEAR},
#endif
#ifdef GL_NEAREST_MIPMAP_NEAREST
  {"NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST},
#endif
#ifdef GL_LINEAR_MIPMAP_NEAREST
  {"LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST},
#endif
#ifdef GL_NEAREST_MIPMAP_LINEAR
  {"NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR},
#endif
#ifdef GL_LINEAR_MIPMAP_LINEAR
  {"LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR},
#endif
#ifdef GL_TEXTURE_MAG_FILTER
  {"TEXTURE_MAG_FILTER", GL_TEXTURE_MAG_FILTER},
#endif
#ifdef GL_TEXTURE_MIN_FILTER
  {"TEXTURE_MIN_FILTER", GL_TEXTURE_MIN_FILTER},
#endif
#ifdef GL_TEXTURE_WRAP_S
  {"TEXTURE_WRAP_S", GL_TEXTURE_WRAP_S},
#endif
#ifdef GL_TEXTURE_WRAP_T
  {"TEXTURE_WRAP_T", GL_TEXTURE_WRAP_T},
#endif
#ifdef GL_REPEAT
  {"REPEAT", GL_REPEAT},
#endif
#ifdef GL_COLOR_LOGIC_O
  {"COLOR_LOGIC_O", GL_COLOR_LOGIC_O},
#endif
#ifdef GL_POLYGON_OFFSET_UNIT
  {"POLYGON_OFFSET_UNIT", GL_POLYGON_OFFSET_UNIT},
#endif
#ifdef GL_POLYGON_OFFSET_POIN
  {"POLYGON_OFFSET_POIN", GL_POLYGON_OFFSET_POIN},
#endif
#ifdef GL_POLYGON_OFFSET_LIN
  {"POLYGON_OFFSET_LIN", GL_POLYGON_OFFSET_LIN},
#endif
#ifdef GL_POLYGON_OFFSET_FIL
  {"POLYGON_OFFSET_FIL", GL_POLYGON_OFFSET_FIL},
#endif
#ifdef GL_POLYGON_OFFSET_FACTO
  {"POLYGON_OFFSET_FACTO", GL_POLYGON_OFFSET_FACTO},
#endif
#ifdef GL_TEXTURE_BINDING_1
  {"TEXTURE_BINDING_1", GL_TEXTURE_BINDING_1},
#endif
#ifdef GL_TEXTURE_BINDING_2
  {"TEXTURE_BINDING_2", GL_TEXTURE_BINDING_2},
#endif
#ifdef GL_TEXTURE_INTERNAL_FORMA
  {"TEXTURE_INTERNAL_FORMA", GL_TEXTURE_INTERNAL_FORMA},
#endif
#ifdef GL_TEXTURE_RED_SIZ
  {"TEXTURE_RED_SIZ", GL_TEXTURE_RED_SIZ},
#endif
#ifdef GL_TEXTURE_GREEN_SIZ
  {"TEXTURE_GREEN_SIZ", GL_TEXTURE_GREEN_SIZ},
#endif
#ifdef GL_TEXTURE_BLUE_SIZ
  {"TEXTURE_BLUE_SIZ", GL_TEXTURE_BLUE_SIZ},
#endif
#ifdef GL_TEXTURE_ALPHA_SIZ
  {"TEXTURE_ALPHA_SIZ", GL_TEXTURE_ALPHA_SIZ},
#endif
#ifdef GL_DOUBL
  {"DOUBL", GL_DOUBL},
#endif
#ifdef GL_PROXY_TEXTURE_1
  {"PROXY_TEXTURE_1", GL_PROXY_TEXTURE_1},
#endif
#ifdef GL_PROXY_TEXTURE_2
  {"PROXY_TEXTURE_2", GL_PROXY_TEXTURE_2},
#endif
#ifdef GL_R3_G3_B
  {"R3_G3_B", GL_R3_G3_B},
#endif
#ifdef GL_RGB
  {"RGB", GL_RGB},
#endif
#ifdef GL_RGB
  {"RGB", GL_RGB},
#endif
#ifdef GL_RGB
  {"RGB", GL_RGB},
#endif
#ifdef GL_RGB1
  {"RGB1", GL_RGB1},
#endif
#ifdef GL_RGB1
  {"RGB1", GL_RGB1},
#endif
#ifdef GL_RGB1
  {"RGB1", GL_RGB1},
#endif
#ifdef GL_RGBA
  {"RGBA", GL_RGBA},
#endif
#ifdef GL_RGBA
  {"RGBA", GL_RGBA},
#endif
#ifdef GL_RGB5_A
  {"RGB5_A", GL_RGB5_A},
#endif
#ifdef GL_RGBA
  {"RGBA", GL_RGBA},
#endif
#ifdef GL_RGB10_A
  {"RGB10_A", GL_RGB10_A},
#endif
#ifdef GL_RGBA1
  {"RGBA1", GL_RGBA1},
#endif
#ifdef GL_RGBA1
  {"RGBA1", GL_RGBA1},
#endif
#ifdef GL_VERTEX_ARRA
  {"VERTEX_ARRA", GL_VERTEX_ARRA},
#endif
#ifdef GL_UNSIGNED_BYTE_3_3_
  {"UNSIGNED_BYTE_3_3_", GL_UNSIGNED_BYTE_3_3_},
#endif
#ifdef GL_UNSIGNED_SHORT_4_4_4_
  {"UNSIGNED_SHORT_4_4_4_", GL_UNSIGNED_SHORT_4_4_4_},
#endif
#ifdef GL_UNSIGNED_SHORT_5_5_5_
  {"UNSIGNED_SHORT_5_5_5_", GL_UNSIGNED_SHORT_5_5_5_},
#endif
#ifdef GL_UNSIGNED_INT_8_8_8_
  {"UNSIGNED_INT_8_8_8_", GL_UNSIGNED_INT_8_8_8_},
#endif
#ifdef GL_UNSIGNED_INT_10_10_10_
  {"UNSIGNED_INT_10_10_10_", GL_UNSIGNED_INT_10_10_10_},
#endif
#ifdef GL_TEXTURE_BINDING_3
  {"TEXTURE_BINDING_3", GL_TEXTURE_BINDING_3},
#endif
#ifdef GL_PACK_SKIP_IMAGE
  {"PACK_SKIP_IMAGE", GL_PACK_SKIP_IMAGE},
#endif
#ifdef GL_PACK_IMAGE_HEIGH
  {"PACK_IMAGE_HEIGH", GL_PACK_IMAGE_HEIGH},
#endif
#ifdef GL_UNPACK_SKIP_IMAGE
  {"UNPACK_SKIP_IMAGE", GL_UNPACK_SKIP_IMAGE},
#endif
#ifdef GL_UNPACK_IMAGE_HEIGH
  {"UNPACK_IMAGE_HEIGH", GL_UNPACK_IMAGE_HEIGH},
#endif
#ifdef GL_TEXTURE_3
  {"TEXTURE_3", GL_TEXTURE_3},
#endif
#ifdef GL_PROXY_TEXTURE_3
  {"PROXY_TEXTURE_3", GL_PROXY_TEXTURE_3},
#endif
#ifdef GL_TEXTURE_DEPT
  {"TEXTURE_DEPT", GL_TEXTURE_DEPT},
#endif
#ifdef GL_TEXTURE_WRAP_
  {"TEXTURE_WRAP_", GL_TEXTURE_WRAP_},
#endif
#ifdef GL_MAX_3D_TEXTURE_SIZ
  {"MAX_3D_TEXTURE_SIZ", GL_MAX_3D_TEXTURE_SIZ},
#endif
#ifdef GL_UNSIGNED_BYTE_2_3_3_RE
  {"UNSIGNED_BYTE_2_3_3_RE", GL_UNSIGNED_BYTE_2_3_3_RE},
#endif
#ifdef GL_UNSIGNED_SHORT_5_6_
  {"UNSIGNED_SHORT_5_6_", GL_UNSIGNED_SHORT_5_6_},
#endif
#ifdef GL_UNSIGNED_SHORT_5_6_5_RE
  {"UNSIGNED_SHORT_5_6_5_RE", GL_UNSIGNED_SHORT_5_6_5_RE},
#endif
#ifdef GL_UNSIGNED_SHORT_4_4_4_4_RE
  {"UNSIGNED_SHORT_4_4_4_4_RE", GL_UNSIGNED_SHORT_4_4_4_4_RE},
#endif
#ifdef GL_UNSIGNED_SHORT_1_5_5_5_RE
  {"UNSIGNED_SHORT_1_5_5_5_RE", GL_UNSIGNED_SHORT_1_5_5_5_RE},
#endif
#ifdef GL_UNSIGNED_INT_8_8_8_8_RE
  {"UNSIGNED_INT_8_8_8_8_RE", GL_UNSIGNED_INT_8_8_8_8_RE},
#endif
#ifdef GL_UNSIGNED_INT_2_10_10_10_RE
  {"UNSIGNED_INT_2_10_10_10_RE", GL_UNSIGNED_INT_2_10_10_10_RE},
#endif
#ifdef GL_BG
  {"BG", GL_BG},
#endif
#ifdef GL_BGR
  {"BGR", GL_BGR},
#endif
#ifdef GL_MAX_ELEMENTS_VERTICE
  {"MAX_ELEMENTS_VERTICE", GL_MAX_ELEMENTS_VERTICE},
#endif
#ifdef GL_MAX_ELEMENTS_INDICE
  {"MAX_ELEMENTS_INDICE", GL_MAX_ELEMENTS_INDICE},
#endif
#ifdef GL_CLAMP_TO_EDG
  {"CLAMP_TO_EDG", GL_CLAMP_TO_EDG},
#endif
#ifdef GL_TEXTURE_MIN_LO
  {"TEXTURE_MIN_LO", GL_TEXTURE_MIN_LO},
#endif
#ifdef GL_TEXTURE_MAX_LO
  {"TEXTURE_MAX_LO", GL_TEXTURE_MAX_LO},
#endif
#ifdef GL_TEXTURE_BASE_LEVE
  {"TEXTURE_BASE_LEVE", GL_TEXTURE_BASE_LEVE},
#endif
#ifdef GL_TEXTURE_MAX_LEVE
  {"TEXTURE_MAX_LEVE", GL_TEXTURE_MAX_LEVE},
#endif
#ifdef GL_SMOOTH_POINT_SIZE_RANG
  {"SMOOTH_POINT_SIZE_RANG", GL_SMOOTH_POINT_SIZE_RANG},
#endif
#ifdef GL_SMOOTH_POINT_SIZE_GRANULARIT
  {"SMOOTH_POINT_SIZE_GRANULARIT", GL_SMOOTH_POINT_SIZE_GRANULARIT},
#endif
#ifdef GL_SMOOTH_LINE_WIDTH_RANG
  {"SMOOTH_LINE_WIDTH_RANG", GL_SMOOTH_LINE_WIDTH_RANG},
#endif
#ifdef GL_SMOOTH_LINE_WIDTH_GRANULARIT
  {"SMOOTH_LINE_WIDTH_GRANULARIT", GL_SMOOTH_LINE_WIDTH_GRANULARIT},
#endif
#ifdef GL_ALIASED_LINE_WIDTH_RANG
  {"ALIASED_LINE_WIDTH_RANG", GL_ALIASED_LINE_WIDTH_RANG},
#endif
#ifdef GL_TEXTURE
  {"TEXTURE", GL_TEXTURE},
#endif
#ifdef GL_TEXTURE
  {"TEXTURE", GL_TEXTURE},
#endif
#ifdef GL_TEXTURE
  {"TEXTURE", GL_TEXTURE},
#endif
#ifdef GL_TEXTURE
  {"TEXTURE", GL_TEXTURE},
#endif
#ifdef GL_TEXTURE
  {"TEXTURE", GL_TEXTURE},
#endif
#ifdef GL_TEXTURE
  {"TEXTURE", GL_TEXTURE},
#endif
#ifdef GL_TEXTURE
  {"TEXTURE", GL_TEXTURE},
#endif
#ifdef GL_TEXTURE
  {"TEXTURE", GL_TEXTURE},
#endif
#ifdef GL_TEXTURE
  {"TEXTURE", GL_TEXTURE},
#endif
#ifdef GL_TEXTURE
  {"TEXTURE", GL_TEXTURE},
#endif
#ifdef GL_TEXTURE1
  {"TEXTURE1", GL_TEXTURE1},
#endif
#ifdef GL_TEXTURE1
  {"TEXTURE1", GL_TEXTURE1},
#endif
#ifdef GL_TEXTURE1
  {"TEXTURE1", GL_TEXTURE1},
#endif
#ifdef GL_TEXTURE1
  {"TEXTURE1", GL_TEXTURE1},
#endif
#ifdef GL_TEXTURE1
  {"TEXTURE1", GL_TEXTURE1},
#endif
#ifdef GL_TEXTURE1
  {"TEXTURE1", GL_TEXTURE1},
#endif
#ifdef GL_TEXTURE1
  {"TEXTURE1", GL_TEXTURE1},
#endif
#ifdef GL_TEXTURE1
  {"TEXTURE1", GL_TEXTURE1},
#endif
#ifdef GL_TEXTURE1
  {"TEXTURE1", GL_TEXTURE1},
#endif
#ifdef GL_TEXTURE1
  {"TEXTURE1", GL_TEXTURE1},
#endif
#ifdef GL_TEXTURE2
  {"TEXTURE2", GL_TEXTURE2},
#endif
#ifdef GL_TEXTURE2
  {"TEXTURE2", GL_TEXTURE2},
#endif
#ifdef GL_TEXTURE2
  {"TEXTURE2", GL_TEXTURE2},
#endif
#ifdef GL_TEXTURE2
  {"TEXTURE2", GL_TEXTURE2},
#endif
#ifdef GL_TEXTURE2
  {"TEXTURE2", GL_TEXTURE2},
#endif
#ifdef GL_TEXTURE2
  {"TEXTURE2", GL_TEXTURE2},
#endif
#ifdef GL_TEXTURE2
  {"TEXTURE2", GL_TEXTURE2},
#endif
#ifdef GL_TEXTURE2
  {"TEXTURE2", GL_TEXTURE2},
#endif
#ifdef GL_TEXTURE2
  {"TEXTURE2", GL_TEXTURE2},
#endif
#ifdef GL_TEXTURE2
  {"TEXTURE2", GL_TEXTURE2},
#endif
#ifdef GL_TEXTURE3
  {"TEXTURE3", GL_TEXTURE3},
#endif
#ifdef GL_TEXTURE3
  {"TEXTURE3", GL_TEXTURE3},
#endif
#ifdef GL_ACTIVE_TEXTUR
  {"ACTIVE_TEXTUR", GL_ACTIVE_TEXTUR},
#endif
#ifdef GL_MULTISAMPL
  {"MULTISAMPL", GL_MULTISAMPL},
#endif
#ifdef GL_SAMPLE_ALPHA_TO_COVERAG
  {"SAMPLE_ALPHA_TO_COVERAG", GL_SAMPLE_ALPHA_TO_COVERAG},
#endif
#ifdef GL_SAMPLE_ALPHA_TO_ON
  {"SAMPLE_ALPHA_TO_ON", GL_SAMPLE_ALPHA_TO_ON},
#endif
#ifdef GL_SAMPLE_COVERAG
  {"SAMPLE_COVERAG", GL_SAMPLE_COVERAG},
#endif
#ifdef GL_SAMPLE_BUFFER
  {"SAMPLE_BUFFER", GL_SAMPLE_BUFFER},
#endif
#ifdef GL_SAMPLE
  {"SAMPLE", GL_SAMPLE},
#endif
#ifdef GL_SAMPLE_COVERAGE_VALU
  {"SAMPLE_COVERAGE_VALU", GL_SAMPLE_COVERAGE_VALU},
#endif
#ifdef GL_SAMPLE_COVERAGE_INVER
  {"SAMPLE_COVERAGE_INVER", GL_SAMPLE_COVERAGE_INVER},
#endif
#ifdef GL_TEXTURE_CUBE_MA
  {"TEXTURE_CUBE_MA", GL_TEXTURE_CUBE_MA},
#endif
#ifdef GL_TEXTURE_BINDING_CUBE_MA
  {"TEXTURE_BINDING_CUBE_MA", GL_TEXTURE_BINDING_CUBE_MA},
#endif
#ifdef GL_TEXTURE_CUBE_MAP_POSITIVE_
  {"TEXTURE_CUBE_MAP_POSITIVE_", GL_TEXTURE_CUBE_MAP_POSITIVE_},
#endif
#ifdef GL_TEXTURE_CUBE_MAP_NEGATIVE_
  {"TEXTURE_CUBE_MAP_NEGATIVE_", GL_TEXTURE_CUBE_MAP_NEGATIVE_},
#endif
#ifdef GL_TEXTURE_CUBE_MAP_POSITIVE_
  {"TEXTURE_CUBE_MAP_POSITIVE_", GL_TEXTURE_CUBE_MAP_POSITIVE_},
#endif
#ifdef GL_TEXTURE_CUBE_MAP_NEGATIVE_
  {"TEXTURE_CUBE_MAP_NEGATIVE_", GL_TEXTURE_CUBE_MAP_NEGATIVE_},
#endif
#ifdef GL_TEXTURE_CUBE_MAP_POSITIVE_
  {"TEXTURE_CUBE_MAP_POSITIVE_", GL_TEXTURE_CUBE_MAP_POSITIVE_},
#endif
#ifdef GL_TEXTURE_CUBE_MAP_NEGATIVE_
  {"TEXTURE_CUBE_MAP_NEGATIVE_", GL_TEXTURE_CUBE_MAP_NEGATIVE_},
#endif
#ifdef GL_PROXY_TEXTURE_CUBE_MA
  {"PROXY_TEXTURE_CUBE_MA", GL_PROXY_TEXTURE_CUBE_MA},
#endif
#ifdef GL_MAX_CUBE_MAP_TEXTURE_SIZ
  {"MAX_CUBE_MAP_TEXTURE_SIZ", GL_MAX_CUBE_MAP_TEXTURE_SIZ},
#endif
#ifdef GL_COMPRESSED_RG
  {"COMPRESSED_RG", GL_COMPRESSED_RG},
#endif
#ifdef GL_COMPRESSED_RGB
  {"COMPRESSED_RGB", GL_COMPRESSED_RGB},
#endif
#ifdef GL_TEXTURE_COMPRESSION_HIN
  {"TEXTURE_COMPRESSION_HIN", GL_TEXTURE_COMPRESSION_HIN},
#endif
#ifdef GL_TEXTURE_COMPRESSED_IMAGE_SIZ
  {"TEXTURE_COMPRESSED_IMAGE_SIZ", GL_TEXTURE_COMPRESSED_IMAGE_SIZ},
#endif
#ifdef GL_TEXTURE_COMPRESSE
  {"TEXTURE_COMPRESSE", GL_TEXTURE_COMPRESSE},
#endif
#ifdef GL_NUM_COMPRESSED_TEXTURE_FORMAT
  {"NUM_COMPRESSED_TEXTURE_FORMAT", GL_NUM_COMPRESSED_TEXTURE_FORMAT},
#endif
#ifdef GL_COMPRESSED_TEXTURE_FORMAT
  {"COMPRESSED_TEXTURE_FORMAT", GL_COMPRESSED_TEXTURE_FORMAT},
#endif
#ifdef GL_CLAMP_TO_BORDE
  {"CLAMP_TO_BORDE", GL_CLAMP_TO_BORDE},
#endif
#ifdef GL_BLEND_DST_RG
  {"BLEND_DST_RG", GL_BLEND_DST_RG},
#endif
#ifdef GL_BLEND_SRC_RG
  {"BLEND_SRC_RG", GL_BLEND_SRC_RG},
#endif
#ifdef GL_BLEND_DST_ALPH
  {"BLEND_DST_ALPH", GL_BLEND_DST_ALPH},
#endif
#ifdef GL_BLEND_SRC_ALPH
  {"BLEND_SRC_ALPH", GL_BLEND_SRC_ALPH},
#endif
#ifdef GL_POINT_FADE_THRESHOLD_SIZ
  {"POINT_FADE_THRESHOLD_SIZ", GL_POINT_FADE_THRESHOLD_SIZ},
#endif
#ifdef GL_DEPTH_COMPONENT1
  {"DEPTH_COMPONENT1", GL_DEPTH_COMPONENT1},
#endif
#ifdef GL_DEPTH_COMPONENT2
  {"DEPTH_COMPONENT2", GL_DEPTH_COMPONENT2},
#endif
#ifdef GL_DEPTH_COMPONENT3
  {"DEPTH_COMPONENT3", GL_DEPTH_COMPONENT3},
#endif
#ifdef GL_MIRRORED_REPEA
  {"MIRRORED_REPEA", GL_MIRRORED_REPEA},
#endif
#ifdef GL_MAX_TEXTURE_LOD_BIA
  {"MAX_TEXTURE_LOD_BIA", GL_MAX_TEXTURE_LOD_BIA},
#endif
#ifdef GL_TEXTURE_LOD_BIA
  {"TEXTURE_LOD_BIA", GL_TEXTURE_LOD_BIA},
#endif
#ifdef GL_INCR_WRA
  {"INCR_WRA", GL_INCR_WRA},
#endif
#ifdef GL_DECR_WRA
  {"DECR_WRA", GL_DECR_WRA},
#endif
#ifdef GL_TEXTURE_DEPTH_SIZ
  {"TEXTURE_DEPTH_SIZ", GL_TEXTURE_DEPTH_SIZ},
#endif
#ifdef GL_TEXTURE_COMPARE_MOD
  {"TEXTURE_COMPARE_MOD", GL_TEXTURE_COMPARE_MOD},
#endif
#ifdef GL_TEXTURE_COMPARE_FUN
  {"TEXTURE_COMPARE_FUN", GL_TEXTURE_COMPARE_FUN},
#endif
#ifdef GL_BLEND_COLO
  {"BLEND_COLO", GL_BLEND_COLO},
#endif
#ifdef GL_BLEND_EQUATIO
  {"BLEND_EQUATIO", GL_BLEND_EQUATIO},
#endif
#ifdef GL_CONSTANT_COLO
  {"CONSTANT_COLO", GL_CONSTANT_COLO},
#endif
#ifdef GL_ONE_MINUS_CONSTANT_COLO
  {"ONE_MINUS_CONSTANT_COLO", GL_ONE_MINUS_CONSTANT_COLO},
#endif
#ifdef GL_CONSTANT_ALPH
  {"CONSTANT_ALPH", GL_CONSTANT_ALPH},
#endif
#ifdef GL_ONE_MINUS_CONSTANT_ALPH
  {"ONE_MINUS_CONSTANT_ALPH", GL_ONE_MINUS_CONSTANT_ALPH},
#endif
#ifdef GL_FUNC_AD
  {"FUNC_AD", GL_FUNC_AD},
#endif
#ifdef GL_FUNC_REVERSE_SUBTRAC
  {"FUNC_REVERSE_SUBTRAC", GL_FUNC_REVERSE_SUBTRAC},
#endif
#ifdef GL_FUNC_SUBTRAC
  {"FUNC_SUBTRAC", GL_FUNC_SUBTRAC},
#endif
#ifdef GL_MI
  {"MI", GL_MI},
#endif
#ifdef GL_MA
  {"MA", GL_MA},
#endif
#ifdef GL_BUFFER_SIZ
  {"BUFFER_SIZ", GL_BUFFER_SIZ},
#endif
#ifdef GL_BUFFER_USAG
  {"BUFFER_USAG", GL_BUFFER_USAG},
#endif
#ifdef GL_QUERY_COUNTER_BIT
  {"QUERY_COUNTER_BIT", GL_QUERY_COUNTER_BIT},
#endif
#ifdef GL_CURRENT_QUER
  {"CURRENT_QUER", GL_CURRENT_QUER},
#endif
#ifdef GL_QUERY_RESUL
  {"QUERY_RESUL", GL_QUERY_RESUL},
#endif
#ifdef GL_QUERY_RESULT_AVAILABL
  {"QUERY_RESULT_AVAILABL", GL_QUERY_RESULT_AVAILABL},
#endif
#ifdef GL_ARRAY_BUFFE
  {"ARRAY_BUFFE", GL_ARRAY_BUFFE},
#endif
#ifdef GL_ELEMENT_ARRAY_BUFFE
  {"ELEMENT_ARRAY_BUFFE", GL_ELEMENT_ARRAY_BUFFE},
#endif
#ifdef GL_ARRAY_BUFFER_BINDIN
  {"ARRAY_BUFFER_BINDIN", GL_ARRAY_BUFFER_BINDIN},
#endif
#ifdef GL_ELEMENT_ARRAY_BUFFER_BINDIN
  {"ELEMENT_ARRAY_BUFFER_BINDIN", GL_ELEMENT_ARRAY_BUFFER_BINDIN},
#endif
#ifdef GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDIN
  {"VERTEX_ATTRIB_ARRAY_BUFFER_BINDIN", GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDIN},
#endif
#ifdef GL_READ_ONL
  {"READ_ONL", GL_READ_ONL},
#endif
#ifdef GL_WRITE_ONL
  {"WRITE_ONL", GL_WRITE_ONL},
#endif
#ifdef GL_READ_WRIT
  {"READ_WRIT", GL_READ_WRIT},
#endif
#ifdef GL_BUFFER_ACCES
  {"BUFFER_ACCES", GL_BUFFER_ACCES},
#endif
#ifdef GL_BUFFER_MAPPE
  {"BUFFER_MAPPE", GL_BUFFER_MAPPE},
#endif
#ifdef GL_BUFFER_MAP_POINTE
  {"BUFFER_MAP_POINTE", GL_BUFFER_MAP_POINTE},
#endif
#ifdef GL_STREAM_DRA
  {"STREAM_DRA", GL_STREAM_DRA},
#endif
#ifdef GL_STREAM_REA
  {"STREAM_REA", GL_STREAM_REA},
#endif
#ifdef GL_STREAM_COP
  {"STREAM_COP", GL_STREAM_COP},
#endif
#ifdef GL_STATIC_DRA
  {"STATIC_DRA", GL_STATIC_DRA},
#endif
#ifdef GL_STATIC_REA
  {"STATIC_REA", GL_STATIC_REA},
#endif
#ifdef GL_STATIC_COP
  {"STATIC_COP", GL_STATIC_COP},
#endif
#ifdef GL_DYNAMIC_DRA
  {"DYNAMIC_DRA", GL_DYNAMIC_DRA},
#endif
#ifdef GL_DYNAMIC_REA
  {"DYNAMIC_REA", GL_DYNAMIC_REA},
#endif
#ifdef GL_DYNAMIC_COP
  {"DYNAMIC_COP", GL_DYNAMIC_COP},
#endif
#ifdef GL_SAMPLES_PASSE
  {"SAMPLES_PASSE", GL_SAMPLES_PASSE},
#endif
#ifdef GL_SRC1_ALPH
  {"SRC1_ALPH", GL_SRC1_ALPH},
#endif
#ifdef GL_BLEND_EQUATION_RG
  {"BLEND_EQUATION_RG", GL_BLEND_EQUATION_RG},
#endif
#ifdef GL_VERTEX_ATTRIB_ARRAY_ENABLE
  {"VERTEX_ATTRIB_ARRAY_ENABLE", GL_VERTEX_ATTRIB_ARRAY_ENABLE},
#endif
#ifdef GL_VERTEX_ATTRIB_ARRAY_SIZ
  {"VERTEX_ATTRIB_ARRAY_SIZ", GL_VERTEX_ATTRIB_ARRAY_SIZ},
#endif
#ifdef GL_VERTEX_ATTRIB_ARRAY_STRID
  {"VERTEX_ATTRIB_ARRAY_STRID", GL_VERTEX_ATTRIB_ARRAY_STRID},
#endif
#ifdef GL_VERTEX_ATTRIB_ARRAY_TYP
  {"VERTEX_ATTRIB_ARRAY_TYP", GL_VERTEX_ATTRIB_ARRAY_TYP},
#endif
#ifdef GL_CURRENT_VERTEX_ATTRI
  {"CURRENT_VERTEX_ATTRI", GL_CURRENT_VERTEX_ATTRI},
#endif
#ifdef GL_VERTEX_PROGRAM_POINT_SIZ
  {"VERTEX_PROGRAM_POINT_SIZ", GL_VERTEX_PROGRAM_POINT_SIZ},
#endif
#ifdef GL_VERTEX_ATTRIB_ARRAY_POINTE
  {"VERTEX_ATTRIB_ARRAY_POINTE", GL_VERTEX_ATTRIB_ARRAY_POINTE},
#endif
#ifdef GL_STENCIL_BACK_FUN
  {"STENCIL_BACK_FUN", GL_STENCIL_BACK_FUN},
#endif
#ifdef GL_STENCIL_BACK_FAI
  {"STENCIL_BACK_FAI", GL_STENCIL_BACK_FAI},
#endif
#ifdef GL_STENCIL_BACK_PASS_DEPTH_FAI
  {"STENCIL_BACK_PASS_DEPTH_FAI", GL_STENCIL_BACK_PASS_DEPTH_FAI},
#endif
#ifdef GL_STENCIL_BACK_PASS_DEPTH_PAS
  {"STENCIL_BACK_PASS_DEPTH_PAS", GL_STENCIL_BACK_PASS_DEPTH_PAS},
#endif
#ifdef GL_MAX_DRAW_BUFFER
  {"MAX_DRAW_BUFFER", GL_MAX_DRAW_BUFFER},
#endif
#ifdef GL_DRAW_BUFFER
  {"DRAW_BUFFER", GL_DRAW_BUFFER},
#endif
#ifdef GL_DRAW_BUFFER
  {"DRAW_BUFFER", GL_DRAW_BUFFER},
#endif
#ifdef GL_DRAW_BUFFER
  {"DRAW_BUFFER", GL_DRAW_BUFFER},
#endif
#ifdef GL_DRAW_BUFFER
  {"DRAW_BUFFER", GL_DRAW_BUFFER},
#endif
#ifdef GL_DRAW_BUFFER
  {"DRAW_BUFFER", GL_DRAW_BUFFER},
#endif
#ifdef GL_DRAW_BUFFER
  {"DRAW_BUFFER", GL_DRAW_BUFFER},
#endif
#ifdef GL_DRAW_BUFFER
  {"DRAW_BUFFER", GL_DRAW_BUFFER},
#endif
#ifdef GL_DRAW_BUFFER
  {"DRAW_BUFFER", GL_DRAW_BUFFER},
#endif
#ifdef GL_DRAW_BUFFER
  {"DRAW_BUFFER", GL_DRAW_BUFFER},
#endif
#ifdef GL_DRAW_BUFFER
  {"DRAW_BUFFER", GL_DRAW_BUFFER},
#endif
#ifdef GL_DRAW_BUFFER1
  {"DRAW_BUFFER1", GL_DRAW_BUFFER1},
#endif
#ifdef GL_DRAW_BUFFER1
  {"DRAW_BUFFER1", GL_DRAW_BUFFER1},
#endif
#ifdef GL_DRAW_BUFFER1
  {"DRAW_BUFFER1", GL_DRAW_BUFFER1},
#endif
#ifdef GL_DRAW_BUFFER1
  {"DRAW_BUFFER1", GL_DRAW_BUFFER1},
#endif
#ifdef GL_DRAW_BUFFER1
  {"DRAW_BUFFER1", GL_DRAW_BUFFER1},
#endif
#ifdef GL_DRAW_BUFFER1
  {"DRAW_BUFFER1", GL_DRAW_BUFFER1},
#endif
#ifdef GL_BLEND_EQUATION_ALPH
  {"BLEND_EQUATION_ALPH", GL_BLEND_EQUATION_ALPH},
#endif
#ifdef GL_MAX_VERTEX_ATTRIB
  {"MAX_VERTEX_ATTRIB", GL_MAX_VERTEX_ATTRIB},
#endif
#ifdef GL_VERTEX_ATTRIB_ARRAY_NORMALIZE
  {"VERTEX_ATTRIB_ARRAY_NORMALIZE", GL_VERTEX_ATTRIB_ARRAY_NORMALIZE},
#endif
#ifdef GL_MAX_TEXTURE_IMAGE_UNIT
  {"MAX_TEXTURE_IMAGE_UNIT", GL_MAX_TEXTURE_IMAGE_UNIT},
#endif
#ifdef GL_FRAGMENT_SHADE
  {"FRAGMENT_SHADE", GL_FRAGMENT_SHADE},
#endif
#ifdef GL_VERTEX_SHADE
  {"VERTEX_SHADE", GL_VERTEX_SHADE},
#endif
#ifdef GL_MAX_FRAGMENT_UNIFORM_COMPONENT
  {"MAX_FRAGMENT_UNIFORM_COMPONENT", GL_MAX_FRAGMENT_UNIFORM_COMPONENT},
#endif
#ifdef GL_MAX_VERTEX_UNIFORM_COMPONENT
  {"MAX_VERTEX_UNIFORM_COMPONENT", GL_MAX_VERTEX_UNIFORM_COMPONENT},
#endif
#ifdef GL_MAX_VARYING_FLOAT
  {"MAX_VARYING_FLOAT", GL_MAX_VARYING_FLOAT},
#endif
#ifdef GL_MAX_VERTEX_TEXTURE_IMAGE_UNIT
  {"MAX_VERTEX_TEXTURE_IMAGE_UNIT", GL_MAX_VERTEX_TEXTURE_IMAGE_UNIT},
#endif
#ifdef GL_MAX_COMBINED_TEXTURE_IMAGE_UNIT
  {"MAX_COMBINED_TEXTURE_IMAGE_UNIT", GL_MAX_COMBINED_TEXTURE_IMAGE_UNIT},
#endif
#ifdef GL_SHADER_TYP
  {"SHADER_TYP", GL_SHADER_TYP},
#endif
#ifdef GL_FLOAT_VEC
  {"FLOAT_VEC", GL_FLOAT_VEC},
#endif
#ifdef GL_FLOAT_VEC
  {"FLOAT_VEC", GL_FLOAT_VEC},
#endif
#ifdef GL_FLOAT_VEC
  {"FLOAT_VEC", GL_FLOAT_VEC},
#endif
#ifdef GL_INT_VEC
  {"INT_VEC", GL_INT_VEC},
#endif
#ifdef GL_INT_VEC
  {"INT_VEC", GL_INT_VEC},
#endif
#ifdef GL_INT_VEC
  {"INT_VEC", GL_INT_VEC},
#endif
#ifdef GL_BOO
  {"BOO", GL_BOO},
#endif
#ifdef GL_BOOL_VEC
  {"BOOL_VEC", GL_BOOL_VEC},
#endif
#ifdef GL_BOOL_VEC
  {"BOOL_VEC", GL_BOOL_VEC},
#endif
#ifdef GL_BOOL_VEC
  {"BOOL_VEC", GL_BOOL_VEC},
#endif
#ifdef GL_FLOAT_MAT
  {"FLOAT_MAT", GL_FLOAT_MAT},
#endif
#ifdef GL_FLOAT_MAT
  {"FLOAT_MAT", GL_FLOAT_MAT},
#endif
#ifdef GL_FLOAT_MAT
  {"FLOAT_MAT", GL_FLOAT_MAT},
#endif
#ifdef GL_SAMPLER_1
  {"SAMPLER_1", GL_SAMPLER_1},
#endif
#ifdef GL_SAMPLER_2
  {"SAMPLER_2", GL_SAMPLER_2},
#endif
#ifdef GL_SAMPLER_3
  {"SAMPLER_3", GL_SAMPLER_3},
#endif
#ifdef GL_SAMPLER_CUB
  {"SAMPLER_CUB", GL_SAMPLER_CUB},
#endif
#ifdef GL_SAMPLER_1D_SHADO
  {"SAMPLER_1D_SHADO", GL_SAMPLER_1D_SHADO},
#endif
#ifdef GL_SAMPLER_2D_SHADO
  {"SAMPLER_2D_SHADO", GL_SAMPLER_2D_SHADO},
#endif
#ifdef GL_DELETE_STATU
  {"DELETE_STATU", GL_DELETE_STATU},
#endif
#ifdef GL_COMPILE_STATU
  {"COMPILE_STATU", GL_COMPILE_STATU},
#endif
#ifdef GL_LINK_STATU
  {"LINK_STATU", GL_LINK_STATU},
#endif
#ifdef GL_VALIDATE_STATU
  {"VALIDATE_STATU", GL_VALIDATE_STATU},
#endif
#ifdef GL_INFO_LOG_LENGT
  {"INFO_LOG_LENGT", GL_INFO_LOG_LENGT},
#endif
#ifdef GL_ATTACHED_SHADER
  {"ATTACHED_SHADER", GL_ATTACHED_SHADER},
#endif
#ifdef GL_ACTIVE_UNIFORM
  {"ACTIVE_UNIFORM", GL_ACTIVE_UNIFORM},
#endif
#ifdef GL_ACTIVE_UNIFORM_MAX_LENGT
  {"ACTIVE_UNIFORM_MAX_LENGT", GL_ACTIVE_UNIFORM_MAX_LENGT},
#endif
#ifdef GL_SHADER_SOURCE_LENGT
  {"SHADER_SOURCE_LENGT", GL_SHADER_SOURCE_LENGT},
#endif
#ifdef GL_ACTIVE_ATTRIBUTE
  {"ACTIVE_ATTRIBUTE", GL_ACTIVE_ATTRIBUTE},
#endif
#ifdef GL_ACTIVE_ATTRIBUTE_MAX_LENGT
  {"ACTIVE_ATTRIBUTE_MAX_LENGT", GL_ACTIVE_ATTRIBUTE_MAX_LENGT},
#endif
#ifdef GL_FRAGMENT_SHADER_DERIVATIVE_HIN
  {"FRAGMENT_SHADER_DERIVATIVE_HIN", GL_FRAGMENT_SHADER_DERIVATIVE_HIN},
#endif
#ifdef GL_SHADING_LANGUAGE_VERSIO
  {"SHADING_LANGUAGE_VERSIO", GL_SHADING_LANGUAGE_VERSIO},
#endif
#ifdef GL_CURRENT_PROGRA
  {"CURRENT_PROGRA", GL_CURRENT_PROGRA},
#endif
#ifdef GL_POINT_SPRITE_COORD_ORIGI
  {"POINT_SPRITE_COORD_ORIGI", GL_POINT_SPRITE_COORD_ORIGI},
#endif
#ifdef GL_LOWER_LEF
  {"LOWER_LEF", GL_LOWER_LEF},
#endif
#ifdef GL_UPPER_LEF
  {"UPPER_LEF", GL_UPPER_LEF},
#endif
#ifdef GL_STENCIL_BACK_RE
  {"STENCIL_BACK_RE", GL_STENCIL_BACK_RE},
#endif
#ifdef GL_STENCIL_BACK_VALUE_MAS
  {"STENCIL_BACK_VALUE_MAS", GL_STENCIL_BACK_VALUE_MAS},
#endif
#ifdef GL_STENCIL_BACK_WRITEMAS
  {"STENCIL_BACK_WRITEMAS", GL_STENCIL_BACK_WRITEMAS},
#endif
#ifdef GL_PIXEL_PACK_BUFFE
  {"PIXEL_PACK_BUFFE", GL_PIXEL_PACK_BUFFE},
#endif
#ifdef GL_PIXEL_UNPACK_BUFFE
  {"PIXEL_UNPACK_BUFFE", GL_PIXEL_UNPACK_BUFFE},
#endif
#ifdef GL_PIXEL_PACK_BUFFER_BINDIN
  {"PIXEL_PACK_BUFFER_BINDIN", GL_PIXEL_PACK_BUFFER_BINDIN},
#endif
#ifdef GL_PIXEL_UNPACK_BUFFER_BINDIN
  {"PIXEL_UNPACK_BUFFER_BINDIN", GL_PIXEL_UNPACK_BUFFER_BINDIN},
#endif
#ifdef GL_FLOAT_MAT2x
  {"FLOAT_MAT2x", GL_FLOAT_MAT2x},
#endif
#ifdef GL_FLOAT_MAT2x
  {"FLOAT_MAT2x", GL_FLOAT_MAT2x},
#endif
#ifdef GL_FLOAT_MAT3x
  {"FLOAT_MAT3x", GL_FLOAT_MAT3x},
#endif
#ifdef GL_FLOAT_MAT3x
  {"FLOAT_MAT3x", GL_FLOAT_MAT3x},
#endif
#ifdef GL_FLOAT_MAT4x
  {"FLOAT_MAT4x", GL_FLOAT_MAT4x},
#endif
#ifdef GL_FLOAT_MAT4x
  {"FLOAT_MAT4x", GL_FLOAT_MAT4x},
#endif
#ifdef GL_SRG
  {"SRG", GL_SRG},
#endif
#ifdef GL_SRGB
  {"SRGB", GL_SRGB},
#endif
#ifdef GL_SRGB_ALPH
  {"SRGB_ALPH", GL_SRGB_ALPH},
#endif
#ifdef GL_SRGB8_ALPHA
  {"SRGB8_ALPHA", GL_SRGB8_ALPHA},
#endif
#ifdef GL_COMPRESSED_SRG
  {"COMPRESSED_SRG", GL_COMPRESSED_SRG},
#endif
#ifdef GL_COMPRESSED_SRGB_ALPH
  {"COMPRESSED_SRGB_ALPH", GL_COMPRESSED_SRGB_ALPH},
#endif
#ifdef GL_COMPARE_REF_TO_TEXTUR
  {"COMPARE_REF_TO_TEXTUR", GL_COMPARE_REF_TO_TEXTUR},
#endif
#ifdef GL_CLIP_DISTANCE
  {"CLIP_DISTANCE", GL_CLIP_DISTANCE},
#endif
#ifdef GL_CLIP_DISTANCE
  {"CLIP_DISTANCE", GL_CLIP_DISTANCE},
#endif
#ifdef GL_CLIP_DISTANCE
  {"CLIP_DISTANCE", GL_CLIP_DISTANCE},
#endif
#ifdef GL_CLIP_DISTANCE
  {"CLIP_DISTANCE", GL_CLIP_DISTANCE},
#endif
#ifdef GL_CLIP_DISTANCE
  {"CLIP_DISTANCE", GL_CLIP_DISTANCE},
#endif
#ifdef GL_CLIP_DISTANCE
  {"CLIP_DISTANCE", GL_CLIP_DISTANCE},
#endif
#ifdef GL_CLIP_DISTANCE
  {"CLIP_DISTANCE", GL_CLIP_DISTANCE},
#endif
#ifdef GL_CLIP_DISTANCE
  {"CLIP_DISTANCE", GL_CLIP_DISTANCE},
#endif
#ifdef GL_MAX_CLIP_DISTANCE
  {"MAX_CLIP_DISTANCE", GL_MAX_CLIP_DISTANCE},
#endif
#ifdef GL_MAJOR_VERSIO
  {"MAJOR_VERSIO", GL_MAJOR_VERSIO},
#endif
#ifdef GL_MINOR_VERSIO
  {"MINOR_VERSIO", GL_MINOR_VERSIO},
#endif
#ifdef GL_NUM_EXTENSION
  {"NUM_EXTENSION", GL_NUM_EXTENSION},
#endif
#ifdef GL_CONTEXT_FLAG
  {"CONTEXT_FLAG", GL_CONTEXT_FLAG},
#endif
#ifdef GL_COMPRESSED_RE
  {"COMPRESSED_RE", GL_COMPRESSED_RE},
#endif
#ifdef GL_COMPRESSED_R
  {"COMPRESSED_R", GL_COMPRESSED_R},
#endif
#ifdef GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BI
  {"CONTEXT_FLAG_FORWARD_COMPATIBLE_BI", GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BI},
#endif
#ifdef GL_RGBA32
  {"RGBA32", GL_RGBA32},
#endif
#ifdef GL_RGB32
  {"RGB32", GL_RGB32},
#endif
#ifdef GL_RGBA16
  {"RGBA16", GL_RGBA16},
#endif
#ifdef GL_RGB16
  {"RGB16", GL_RGB16},
#endif
#ifdef GL_VERTEX_ATTRIB_ARRAY_INTEGE
  {"VERTEX_ATTRIB_ARRAY_INTEGE", GL_VERTEX_ATTRIB_ARRAY_INTEGE},
#endif
#ifdef GL_MAX_ARRAY_TEXTURE_LAYER
  {"MAX_ARRAY_TEXTURE_LAYER", GL_MAX_ARRAY_TEXTURE_LAYER},
#endif
#ifdef GL_MIN_PROGRAM_TEXEL_OFFSE
  {"MIN_PROGRAM_TEXEL_OFFSE", GL_MIN_PROGRAM_TEXEL_OFFSE},
#endif
#ifdef GL_MAX_PROGRAM_TEXEL_OFFSE
  {"MAX_PROGRAM_TEXEL_OFFSE", GL_MAX_PROGRAM_TEXEL_OFFSE},
#endif
#ifdef GL_CLAMP_READ_COLO
  {"CLAMP_READ_COLO", GL_CLAMP_READ_COLO},
#endif
#ifdef GL_FIXED_ONL
  {"FIXED_ONL", GL_FIXED_ONL},
#endif
#ifdef GL_MAX_VARYING_COMPONENT
  {"MAX_VARYING_COMPONENT", GL_MAX_VARYING_COMPONENT},
#endif
#ifdef GL_TEXTURE_1D_ARRA
  {"TEXTURE_1D_ARRA", GL_TEXTURE_1D_ARRA},
#endif
#ifdef GL_PROXY_TEXTURE_1D_ARRA
  {"PROXY_TEXTURE_1D_ARRA", GL_PROXY_TEXTURE_1D_ARRA},
#endif
#ifdef GL_TEXTURE_2D_ARRA
  {"TEXTURE_2D_ARRA", GL_TEXTURE_2D_ARRA},
#endif
#ifdef GL_PROXY_TEXTURE_2D_ARRA
  {"PROXY_TEXTURE_2D_ARRA", GL_PROXY_TEXTURE_2D_ARRA},
#endif
#ifdef GL_TEXTURE_BINDING_1D_ARRA
  {"TEXTURE_BINDING_1D_ARRA", GL_TEXTURE_BINDING_1D_ARRA},
#endif
#ifdef GL_TEXTURE_BINDING_2D_ARRA
  {"TEXTURE_BINDING_2D_ARRA", GL_TEXTURE_BINDING_2D_ARRA},
#endif
#ifdef GL_R11F_G11F_B10
  {"R11F_G11F_B10", GL_R11F_G11F_B10},
#endif
#ifdef GL_UNSIGNED_INT_10F_11F_11F_RE
  {"UNSIGNED_INT_10F_11F_11F_RE", GL_UNSIGNED_INT_10F_11F_11F_RE},
#endif
#ifdef GL_RGB9_E
  {"RGB9_E", GL_RGB9_E},
#endif
#ifdef GL_UNSIGNED_INT_5_9_9_9_RE
  {"UNSIGNED_INT_5_9_9_9_RE", GL_UNSIGNED_INT_5_9_9_9_RE},
#endif
#ifdef GL_TEXTURE_SHARED_SIZ
  {"TEXTURE_SHARED_SIZ", GL_TEXTURE_SHARED_SIZ},
#endif
#ifdef GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGT
  {"TRANSFORM_FEEDBACK_VARYING_MAX_LENGT", GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGT},
#endif
#ifdef GL_TRANSFORM_FEEDBACK_BUFFER_MOD
  {"TRANSFORM_FEEDBACK_BUFFER_MOD", GL_TRANSFORM_FEEDBACK_BUFFER_MOD},
#endif
#ifdef GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENT
  {"MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENT", GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENT},
#endif
#ifdef GL_TRANSFORM_FEEDBACK_VARYING
  {"TRANSFORM_FEEDBACK_VARYING", GL_TRANSFORM_FEEDBACK_VARYING},
#endif
#ifdef GL_TRANSFORM_FEEDBACK_BUFFER_STAR
  {"TRANSFORM_FEEDBACK_BUFFER_STAR", GL_TRANSFORM_FEEDBACK_BUFFER_STAR},
#endif
#ifdef GL_TRANSFORM_FEEDBACK_BUFFER_SIZ
  {"TRANSFORM_FEEDBACK_BUFFER_SIZ", GL_TRANSFORM_FEEDBACK_BUFFER_SIZ},
#endif
#ifdef GL_PRIMITIVES_GENERATE
  {"PRIMITIVES_GENERATE", GL_PRIMITIVES_GENERATE},
#endif
#ifdef GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTE
  {"TRANSFORM_FEEDBACK_PRIMITIVES_WRITTE", GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTE},
#endif
#ifdef GL_RASTERIZER_DISCAR
  {"RASTERIZER_DISCAR", GL_RASTERIZER_DISCAR},
#endif
#ifdef GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENT
  {"MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENT", GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENT},
#endif
#ifdef GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIB
  {"MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIB", GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIB},
#endif
#ifdef GL_INTERLEAVED_ATTRIB
  {"INTERLEAVED_ATTRIB", GL_INTERLEAVED_ATTRIB},
#endif
#ifdef GL_SEPARATE_ATTRIB
  {"SEPARATE_ATTRIB", GL_SEPARATE_ATTRIB},
#endif
#ifdef GL_TRANSFORM_FEEDBACK_BUFFE
  {"TRANSFORM_FEEDBACK_BUFFE", GL_TRANSFORM_FEEDBACK_BUFFE},
#endif
#ifdef GL_TRANSFORM_FEEDBACK_BUFFER_BINDIN
  {"TRANSFORM_FEEDBACK_BUFFER_BINDIN", GL_TRANSFORM_FEEDBACK_BUFFER_BINDIN},
#endif
#ifdef GL_RGBA32U
  {"RGBA32U", GL_RGBA32U},
#endif
#ifdef GL_RGB32U
  {"RGB32U", GL_RGB32U},
#endif
#ifdef GL_RGBA16U
  {"RGBA16U", GL_RGBA16U},
#endif
#ifdef GL_RGB16U
  {"RGB16U", GL_RGB16U},
#endif
#ifdef GL_RGBA8U
  {"RGBA8U", GL_RGBA8U},
#endif
#ifdef GL_RGB8U
  {"RGB8U", GL_RGB8U},
#endif
#ifdef GL_RGBA32
  {"RGBA32", GL_RGBA32},
#endif
#ifdef GL_RGB32
  {"RGB32", GL_RGB32},
#endif
#ifdef GL_RGBA16
  {"RGBA16", GL_RGBA16},
#endif
#ifdef GL_RGB16
  {"RGB16", GL_RGB16},
#endif
#ifdef GL_RGBA8
  {"RGBA8", GL_RGBA8},
#endif
#ifdef GL_RGB8
  {"RGB8", GL_RGB8},
#endif
#ifdef GL_RED_INTEGE
  {"RED_INTEGE", GL_RED_INTEGE},
#endif
#ifdef GL_GREEN_INTEGE
  {"GREEN_INTEGE", GL_GREEN_INTEGE},
#endif
#ifdef GL_BLUE_INTEGE
  {"BLUE_INTEGE", GL_BLUE_INTEGE},
#endif
#ifdef GL_RGB_INTEGE
  {"RGB_INTEGE", GL_RGB_INTEGE},
#endif
#ifdef GL_RGBA_INTEGE
  {"RGBA_INTEGE", GL_RGBA_INTEGE},
#endif
#ifdef GL_BGR_INTEGE
  {"BGR_INTEGE", GL_BGR_INTEGE},
#endif
#ifdef GL_BGRA_INTEGE
  {"BGRA_INTEGE", GL_BGRA_INTEGE},
#endif
#ifdef GL_SAMPLER_1D_ARRA
  {"SAMPLER_1D_ARRA", GL_SAMPLER_1D_ARRA},
#endif
#ifdef GL_SAMPLER_2D_ARRA
  {"SAMPLER_2D_ARRA", GL_SAMPLER_2D_ARRA},
#endif
#ifdef GL_SAMPLER_1D_ARRAY_SHADO
  {"SAMPLER_1D_ARRAY_SHADO", GL_SAMPLER_1D_ARRAY_SHADO},
#endif
#ifdef GL_SAMPLER_2D_ARRAY_SHADO
  {"SAMPLER_2D_ARRAY_SHADO", GL_SAMPLER_2D_ARRAY_SHADO},
#endif
#ifdef GL_SAMPLER_CUBE_SHADO
  {"SAMPLER_CUBE_SHADO", GL_SAMPLER_CUBE_SHADO},
#endif
#ifdef GL_UNSIGNED_INT_VEC
  {"UNSIGNED_INT_VEC", GL_UNSIGNED_INT_VEC},
#endif
#ifdef GL_UNSIGNED_INT_VEC
  {"UNSIGNED_INT_VEC", GL_UNSIGNED_INT_VEC},
#endif
#ifdef GL_UNSIGNED_INT_VEC
  {"UNSIGNED_INT_VEC", GL_UNSIGNED_INT_VEC},
#endif
#ifdef GL_INT_SAMPLER_1
  {"INT_SAMPLER_1", GL_INT_SAMPLER_1},
#endif
#ifdef GL_INT_SAMPLER_2
  {"INT_SAMPLER_2", GL_INT_SAMPLER_2},
#endif
#ifdef GL_INT_SAMPLER_3
  {"INT_SAMPLER_3", GL_INT_SAMPLER_3},
#endif
#ifdef GL_INT_SAMPLER_CUB
  {"INT_SAMPLER_CUB", GL_INT_SAMPLER_CUB},
#endif
#ifdef GL_INT_SAMPLER_1D_ARRA
  {"INT_SAMPLER_1D_ARRA", GL_INT_SAMPLER_1D_ARRA},
#endif
#ifdef GL_INT_SAMPLER_2D_ARRA
  {"INT_SAMPLER_2D_ARRA", GL_INT_SAMPLER_2D_ARRA},
#endif
#ifdef GL_UNSIGNED_INT_SAMPLER_1
  {"UNSIGNED_INT_SAMPLER_1", GL_UNSIGNED_INT_SAMPLER_1},
#endif
#ifdef GL_UNSIGNED_INT_SAMPLER_2
  {"UNSIGNED_INT_SAMPLER_2", GL_UNSIGNED_INT_SAMPLER_2},
#endif
#ifdef GL_UNSIGNED_INT_SAMPLER_3
  {"UNSIGNED_INT_SAMPLER_3", GL_UNSIGNED_INT_SAMPLER_3},
#endif
#ifdef GL_UNSIGNED_INT_SAMPLER_CUB
  {"UNSIGNED_INT_SAMPLER_CUB", GL_UNSIGNED_INT_SAMPLER_CUB},
#endif
#ifdef GL_UNSIGNED_INT_SAMPLER_1D_ARRA
  {"UNSIGNED_INT_SAMPLER_1D_ARRA", GL_UNSIGNED_INT_SAMPLER_1D_ARRA},
#endif
#ifdef GL_UNSIGNED_INT_SAMPLER_2D_ARRA
  {"UNSIGNED_INT_SAMPLER_2D_ARRA", GL_UNSIGNED_INT_SAMPLER_2D_ARRA},
#endif
#ifdef GL_QUERY_WAI
  {"QUERY_WAI", GL_QUERY_WAI},
#endif
#ifdef GL_QUERY_NO_WAI
  {"QUERY_NO_WAI", GL_QUERY_NO_WAI},
#endif
#ifdef GL_QUERY_BY_REGION_WAI
  {"QUERY_BY_REGION_WAI", GL_QUERY_BY_REGION_WAI},
#endif
#ifdef GL_QUERY_BY_REGION_NO_WAI
  {"QUERY_BY_REGION_NO_WAI", GL_QUERY_BY_REGION_NO_WAI},
#endif
#ifdef GL_BUFFER_ACCESS_FLAG
  {"BUFFER_ACCESS_FLAG", GL_BUFFER_ACCESS_FLAG},
#endif
#ifdef GL_BUFFER_MAP_LENGT
  {"BUFFER_MAP_LENGT", GL_BUFFER_MAP_LENGT},
#endif
#ifdef GL_BUFFER_MAP_OFFSE
  {"BUFFER_MAP_OFFSE", GL_BUFFER_MAP_OFFSE},
#endif
#ifdef GL_DEPTH_COMPONENT32
  {"DEPTH_COMPONENT32", GL_DEPTH_COMPONENT32},
#endif
#ifdef GL_DEPTH32F_STENCIL
  {"DEPTH32F_STENCIL", GL_DEPTH32F_STENCIL},
#endif
#ifdef GL_FLOAT_32_UNSIGNED_INT_24_8_RE
  {"FLOAT_32_UNSIGNED_INT_24_8_RE", GL_FLOAT_32_UNSIGNED_INT_24_8_RE},
#endif
#ifdef GL_INVALID_FRAMEBUFFER_OPERATIO
  {"INVALID_FRAMEBUFFER_OPERATIO", GL_INVALID_FRAMEBUFFER_OPERATIO},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODIN
  {"FRAMEBUFFER_ATTACHMENT_COLOR_ENCODIN", GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODIN},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYP
  {"FRAMEBUFFER_ATTACHMENT_COMPONENT_TYP", GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYP},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_RED_SIZ
  {"FRAMEBUFFER_ATTACHMENT_RED_SIZ", GL_FRAMEBUFFER_ATTACHMENT_RED_SIZ},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZ
  {"FRAMEBUFFER_ATTACHMENT_GREEN_SIZ", GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZ},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZ
  {"FRAMEBUFFER_ATTACHMENT_BLUE_SIZ", GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZ},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZ
  {"FRAMEBUFFER_ATTACHMENT_ALPHA_SIZ", GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZ},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZ
  {"FRAMEBUFFER_ATTACHMENT_DEPTH_SIZ", GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZ},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZ
  {"FRAMEBUFFER_ATTACHMENT_STENCIL_SIZ", GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZ},
#endif
#ifdef GL_FRAMEBUFFER_DEFAUL
  {"FRAMEBUFFER_DEFAUL", GL_FRAMEBUFFER_DEFAUL},
#endif
#ifdef GL_FRAMEBUFFER_UNDEFINE
  {"FRAMEBUFFER_UNDEFINE", GL_FRAMEBUFFER_UNDEFINE},
#endif
#ifdef GL_DEPTH_STENCIL_ATTACHMEN
  {"DEPTH_STENCIL_ATTACHMEN", GL_DEPTH_STENCIL_ATTACHMEN},
#endif
#ifdef GL_MAX_RENDERBUFFER_SIZ
  {"MAX_RENDERBUFFER_SIZ", GL_MAX_RENDERBUFFER_SIZ},
#endif
#ifdef GL_DEPTH_STENCI
  {"DEPTH_STENCI", GL_DEPTH_STENCI},
#endif
#ifdef GL_UNSIGNED_INT_24_
  {"UNSIGNED_INT_24_", GL_UNSIGNED_INT_24_},
#endif
#ifdef GL_DEPTH24_STENCIL
  {"DEPTH24_STENCIL", GL_DEPTH24_STENCIL},
#endif
#ifdef GL_TEXTURE_STENCIL_SIZ
  {"TEXTURE_STENCIL_SIZ", GL_TEXTURE_STENCIL_SIZ},
#endif
#ifdef GL_TEXTURE_RED_TYP
  {"TEXTURE_RED_TYP", GL_TEXTURE_RED_TYP},
#endif
#ifdef GL_TEXTURE_GREEN_TYP
  {"TEXTURE_GREEN_TYP", GL_TEXTURE_GREEN_TYP},
#endif
#ifdef GL_TEXTURE_BLUE_TYP
  {"TEXTURE_BLUE_TYP", GL_TEXTURE_BLUE_TYP},
#endif
#ifdef GL_TEXTURE_ALPHA_TYP
  {"TEXTURE_ALPHA_TYP", GL_TEXTURE_ALPHA_TYP},
#endif
#ifdef GL_TEXTURE_DEPTH_TYP
  {"TEXTURE_DEPTH_TYP", GL_TEXTURE_DEPTH_TYP},
#endif
#ifdef GL_UNSIGNED_NORMALIZE
  {"UNSIGNED_NORMALIZE", GL_UNSIGNED_NORMALIZE},
#endif
#ifdef GL_FRAMEBUFFER_BINDIN
  {"FRAMEBUFFER_BINDIN", GL_FRAMEBUFFER_BINDIN},
#endif
#ifdef GL_DRAW_FRAMEBUFFER_BINDIN
  {"DRAW_FRAMEBUFFER_BINDIN", GL_DRAW_FRAMEBUFFER_BINDIN},
#endif
#ifdef GL_RENDERBUFFER_BINDIN
  {"RENDERBUFFER_BINDIN", GL_RENDERBUFFER_BINDIN},
#endif
#ifdef GL_READ_FRAMEBUFFE
  {"READ_FRAMEBUFFE", GL_READ_FRAMEBUFFE},
#endif
#ifdef GL_DRAW_FRAMEBUFFE
  {"DRAW_FRAMEBUFFE", GL_DRAW_FRAMEBUFFE},
#endif
#ifdef GL_READ_FRAMEBUFFER_BINDIN
  {"READ_FRAMEBUFFER_BINDIN", GL_READ_FRAMEBUFFER_BINDIN},
#endif
#ifdef GL_RENDERBUFFER_SAMPLE
  {"RENDERBUFFER_SAMPLE", GL_RENDERBUFFER_SAMPLE},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYP
  {"FRAMEBUFFER_ATTACHMENT_OBJECT_TYP", GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYP},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAM
  {"FRAMEBUFFER_ATTACHMENT_OBJECT_NAM", GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAM},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVE
  {"FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVE", GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVE},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FAC
  {"FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FAC", GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FAC},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYE
  {"FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYE", GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYE},
#endif
#ifdef GL_FRAMEBUFFER_COMPLET
  {"FRAMEBUFFER_COMPLET", GL_FRAMEBUFFER_COMPLET},
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_ATTACHMEN
  {"FRAMEBUFFER_INCOMPLETE_ATTACHMEN", GL_FRAMEBUFFER_INCOMPLETE_ATTACHMEN},
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMEN
  {"FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMEN", GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMEN},
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFE
  {"FRAMEBUFFER_INCOMPLETE_DRAW_BUFFE", GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFE},
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFE
  {"FRAMEBUFFER_INCOMPLETE_READ_BUFFE", GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFE},
#endif
#ifdef GL_FRAMEBUFFER_UNSUPPORTE
  {"FRAMEBUFFER_UNSUPPORTE", GL_FRAMEBUFFER_UNSUPPORTE},
#endif
#ifdef GL_MAX_COLOR_ATTACHMENT
  {"MAX_COLOR_ATTACHMENT", GL_MAX_COLOR_ATTACHMENT},
#endif
#ifdef GL_COLOR_ATTACHMENT
  {"COLOR_ATTACHMENT", GL_COLOR_ATTACHMENT},
#endif
#ifdef GL_COLOR_ATTACHMENT
  {"COLOR_ATTACHMENT", GL_COLOR_ATTACHMENT},
#endif
#ifdef GL_COLOR_ATTACHMENT
  {"COLOR_ATTACHMENT", GL_COLOR_ATTACHMENT},
#endif
#ifdef GL_COLOR_ATTACHMENT
  {"COLOR_ATTACHMENT", GL_COLOR_ATTACHMENT},
#endif
#ifdef GL_COLOR_ATTACHMENT
  {"COLOR_ATTACHMENT", GL_COLOR_ATTACHMENT},
#endif
#ifdef GL_COLOR_ATTACHMENT
  {"COLOR_ATTACHMENT", GL_COLOR_ATTACHMENT},
#endif
#ifdef GL_COLOR_ATTACHMENT
  {"COLOR_ATTACHMENT", GL_COLOR_ATTACHMENT},
#endif
#ifdef GL_COLOR_ATTACHMENT
  {"COLOR_ATTACHMENT", GL_COLOR_ATTACHMENT},
#endif
#ifdef GL_COLOR_ATTACHMENT
  {"COLOR_ATTACHMENT", GL_COLOR_ATTACHMENT},
#endif
#ifdef GL_COLOR_ATTACHMENT
  {"COLOR_ATTACHMENT", GL_COLOR_ATTACHMENT},
#endif
#ifdef GL_COLOR_ATTACHMENT1
  {"COLOR_ATTACHMENT1", GL_COLOR_ATTACHMENT1},
#endif
#ifdef GL_COLOR_ATTACHMENT1
  {"COLOR_ATTACHMENT1", GL_COLOR_ATTACHMENT1},
#endif
#ifdef GL_COLOR_ATTACHMENT1
  {"COLOR_ATTACHMENT1", GL_COLOR_ATTACHMENT1},
#endif
#ifdef GL_COLOR_ATTACHMENT1
  {"COLOR_ATTACHMENT1", GL_COLOR_ATTACHMENT1},
#endif
#ifdef GL_COLOR_ATTACHMENT1
  {"COLOR_ATTACHMENT1", GL_COLOR_ATTACHMENT1},
#endif
#ifdef GL_COLOR_ATTACHMENT1
  {"COLOR_ATTACHMENT1", GL_COLOR_ATTACHMENT1},
#endif
#ifdef GL_COLOR_ATTACHMENT1
  {"COLOR_ATTACHMENT1", GL_COLOR_ATTACHMENT1},
#endif
#ifdef GL_COLOR_ATTACHMENT1
  {"COLOR_ATTACHMENT1", GL_COLOR_ATTACHMENT1},
#endif
#ifdef GL_COLOR_ATTACHMENT1
  {"COLOR_ATTACHMENT1", GL_COLOR_ATTACHMENT1},
#endif
#ifdef GL_COLOR_ATTACHMENT1
  {"COLOR_ATTACHMENT1", GL_COLOR_ATTACHMENT1},
#endif
#ifdef GL_COLOR_ATTACHMENT2
  {"COLOR_ATTACHMENT2", GL_COLOR_ATTACHMENT2},
#endif
#ifdef GL_COLOR_ATTACHMENT2
  {"COLOR_ATTACHMENT2", GL_COLOR_ATTACHMENT2},
#endif
#ifdef GL_COLOR_ATTACHMENT2
  {"COLOR_ATTACHMENT2", GL_COLOR_ATTACHMENT2},
#endif
#ifdef GL_COLOR_ATTACHMENT2
  {"COLOR_ATTACHMENT2", GL_COLOR_ATTACHMENT2},
#endif
#ifdef GL_COLOR_ATTACHMENT2
  {"COLOR_ATTACHMENT2", GL_COLOR_ATTACHMENT2},
#endif
#ifdef GL_COLOR_ATTACHMENT2
  {"COLOR_ATTACHMENT2", GL_COLOR_ATTACHMENT2},
#endif
#ifdef GL_COLOR_ATTACHMENT2
  {"COLOR_ATTACHMENT2", GL_COLOR_ATTACHMENT2},
#endif
#ifdef GL_COLOR_ATTACHMENT2
  {"COLOR_ATTACHMENT2", GL_COLOR_ATTACHMENT2},
#endif
#ifdef GL_COLOR_ATTACHMENT2
  {"COLOR_ATTACHMENT2", GL_COLOR_ATTACHMENT2},
#endif
#ifdef GL_COLOR_ATTACHMENT2
  {"COLOR_ATTACHMENT2", GL_COLOR_ATTACHMENT2},
#endif
#ifdef GL_COLOR_ATTACHMENT3
  {"COLOR_ATTACHMENT3", GL_COLOR_ATTACHMENT3},
#endif
#ifdef GL_COLOR_ATTACHMENT3
  {"COLOR_ATTACHMENT3", GL_COLOR_ATTACHMENT3},
#endif
#ifdef GL_DEPTH_ATTACHMEN
  {"DEPTH_ATTACHMEN", GL_DEPTH_ATTACHMEN},
#endif
#ifdef GL_STENCIL_ATTACHMEN
  {"STENCIL_ATTACHMEN", GL_STENCIL_ATTACHMEN},
#endif
#ifdef GL_FRAMEBUFFE
  {"FRAMEBUFFE", GL_FRAMEBUFFE},
#endif
#ifdef GL_RENDERBUFFE
  {"RENDERBUFFE", GL_RENDERBUFFE},
#endif
#ifdef GL_RENDERBUFFER_WIDT
  {"RENDERBUFFER_WIDT", GL_RENDERBUFFER_WIDT},
#endif
#ifdef GL_RENDERBUFFER_HEIGH
  {"RENDERBUFFER_HEIGH", GL_RENDERBUFFER_HEIGH},
#endif
#ifdef GL_RENDERBUFFER_INTERNAL_FORMA
  {"RENDERBUFFER_INTERNAL_FORMA", GL_RENDERBUFFER_INTERNAL_FORMA},
#endif
#ifdef GL_STENCIL_INDEX
  {"STENCIL_INDEX", GL_STENCIL_INDEX},
#endif
#ifdef GL_STENCIL_INDEX
  {"STENCIL_INDEX", GL_STENCIL_INDEX},
#endif
#ifdef GL_STENCIL_INDEX
  {"STENCIL_INDEX", GL_STENCIL_INDEX},
#endif
#ifdef GL_STENCIL_INDEX1
  {"STENCIL_INDEX1", GL_STENCIL_INDEX1},
#endif
#ifdef GL_RENDERBUFFER_RED_SIZ
  {"RENDERBUFFER_RED_SIZ", GL_RENDERBUFFER_RED_SIZ},
#endif
#ifdef GL_RENDERBUFFER_GREEN_SIZ
  {"RENDERBUFFER_GREEN_SIZ", GL_RENDERBUFFER_GREEN_SIZ},
#endif
#ifdef GL_RENDERBUFFER_BLUE_SIZ
  {"RENDERBUFFER_BLUE_SIZ", GL_RENDERBUFFER_BLUE_SIZ},
#endif
#ifdef GL_RENDERBUFFER_ALPHA_SIZ
  {"RENDERBUFFER_ALPHA_SIZ", GL_RENDERBUFFER_ALPHA_SIZ},
#endif
#ifdef GL_RENDERBUFFER_DEPTH_SIZ
  {"RENDERBUFFER_DEPTH_SIZ", GL_RENDERBUFFER_DEPTH_SIZ},
#endif
#ifdef GL_RENDERBUFFER_STENCIL_SIZ
  {"RENDERBUFFER_STENCIL_SIZ", GL_RENDERBUFFER_STENCIL_SIZ},
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPL
  {"FRAMEBUFFER_INCOMPLETE_MULTISAMPL", GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPL},
#endif
#ifdef GL_MAX_SAMPLE
  {"MAX_SAMPLE", GL_MAX_SAMPLE},
#endif
#ifdef GL_FRAMEBUFFER_SRG
  {"FRAMEBUFFER_SRG", GL_FRAMEBUFFER_SRG},
#endif
#ifdef GL_HALF_FLOA
  {"HALF_FLOA", GL_HALF_FLOA},
#endif
#ifdef GL_MAP_READ_BI
  {"MAP_READ_BI", GL_MAP_READ_BI},
#endif
#ifdef GL_MAP_WRITE_BI
  {"MAP_WRITE_BI", GL_MAP_WRITE_BI},
#endif
#ifdef GL_MAP_INVALIDATE_RANGE_BI
  {"MAP_INVALIDATE_RANGE_BI", GL_MAP_INVALIDATE_RANGE_BI},
#endif
#ifdef GL_MAP_INVALIDATE_BUFFER_BI
  {"MAP_INVALIDATE_BUFFER_BI", GL_MAP_INVALIDATE_BUFFER_BI},
#endif
#ifdef GL_MAP_FLUSH_EXPLICIT_BI
  {"MAP_FLUSH_EXPLICIT_BI", GL_MAP_FLUSH_EXPLICIT_BI},
#endif
#ifdef GL_MAP_UNSYNCHRONIZED_BI
  {"MAP_UNSYNCHRONIZED_BI", GL_MAP_UNSYNCHRONIZED_BI},
#endif
#ifdef GL_COMPRESSED_RED_RGTC
  {"COMPRESSED_RED_RGTC", GL_COMPRESSED_RED_RGTC},
#endif
#ifdef GL_COMPRESSED_SIGNED_RED_RGTC
  {"COMPRESSED_SIGNED_RED_RGTC", GL_COMPRESSED_SIGNED_RED_RGTC},
#endif
#ifdef GL_COMPRESSED_RG_RGTC
  {"COMPRESSED_RG_RGTC", GL_COMPRESSED_RG_RGTC},
#endif
#ifdef GL_COMPRESSED_SIGNED_RG_RGTC
  {"COMPRESSED_SIGNED_RG_RGTC", GL_COMPRESSED_SIGNED_RG_RGTC},
#endif
#ifdef GL_R
  {"R", GL_R},
#endif
#ifdef GL_RG_INTEGE
  {"RG_INTEGE", GL_RG_INTEGE},
#endif
#ifdef GL_R
  {"R", GL_R},
#endif
#ifdef GL_R1
  {"R1", GL_R1},
#endif
#ifdef GL_RG
  {"RG", GL_RG},
#endif
#ifdef GL_RG1
  {"RG1", GL_RG1},
#endif
#ifdef GL_R16
  {"R16", GL_R16},
#endif
#ifdef GL_R32
  {"R32", GL_R32},
#endif
#ifdef GL_RG16
  {"RG16", GL_RG16},
#endif
#ifdef GL_RG32
  {"RG32", GL_RG32},
#endif
#ifdef GL_R8
  {"R8", GL_R8},
#endif
#ifdef GL_R8U
  {"R8U", GL_R8U},
#endif
#ifdef GL_R16
  {"R16", GL_R16},
#endif
#ifdef GL_R16U
  {"R16U", GL_R16U},
#endif
#ifdef GL_R32
  {"R32", GL_R32},
#endif
#ifdef GL_R32U
  {"R32U", GL_R32U},
#endif
#ifdef GL_RG8
  {"RG8", GL_RG8},
#endif
#ifdef GL_RG8U
  {"RG8U", GL_RG8U},
#endif
#ifdef GL_RG16
  {"RG16", GL_RG16},
#endif
#ifdef GL_RG16U
  {"RG16U", GL_RG16U},
#endif
#ifdef GL_RG32
  {"RG32", GL_RG32},
#endif
#ifdef GL_RG32U
  {"RG32U", GL_RG32U},
#endif
#ifdef GL_VERTEX_ARRAY_BINDIN
  {"VERTEX_ARRAY_BINDIN", GL_VERTEX_ARRAY_BINDIN},
#endif
#ifdef GL_SAMPLER_2D_REC
  {"SAMPLER_2D_REC", GL_SAMPLER_2D_REC},
#endif
#ifdef GL_SAMPLER_2D_RECT_SHADO
  {"SAMPLER_2D_RECT_SHADO", GL_SAMPLER_2D_RECT_SHADO},
#endif
#ifdef GL_SAMPLER_BUFFE
  {"SAMPLER_BUFFE", GL_SAMPLER_BUFFE},
#endif
#ifdef GL_INT_SAMPLER_2D_REC
  {"INT_SAMPLER_2D_REC", GL_INT_SAMPLER_2D_REC},
#endif
#ifdef GL_INT_SAMPLER_BUFFE
  {"INT_SAMPLER_BUFFE", GL_INT_SAMPLER_BUFFE},
#endif
#ifdef GL_UNSIGNED_INT_SAMPLER_2D_REC
  {"UNSIGNED_INT_SAMPLER_2D_REC", GL_UNSIGNED_INT_SAMPLER_2D_REC},
#endif
#ifdef GL_UNSIGNED_INT_SAMPLER_BUFFE
  {"UNSIGNED_INT_SAMPLER_BUFFE", GL_UNSIGNED_INT_SAMPLER_BUFFE},
#endif
#ifdef GL_TEXTURE_BUFFE
  {"TEXTURE_BUFFE", GL_TEXTURE_BUFFE},
#endif
#ifdef GL_MAX_TEXTURE_BUFFER_SIZ
  {"MAX_TEXTURE_BUFFER_SIZ", GL_MAX_TEXTURE_BUFFER_SIZ},
#endif
#ifdef GL_TEXTURE_BINDING_BUFFE
  {"TEXTURE_BINDING_BUFFE", GL_TEXTURE_BINDING_BUFFE},
#endif
#ifdef GL_TEXTURE_BUFFER_DATA_STORE_BINDIN
  {"TEXTURE_BUFFER_DATA_STORE_BINDIN", GL_TEXTURE_BUFFER_DATA_STORE_BINDIN},
#endif
#ifdef GL_TEXTURE_RECTANGL
  {"TEXTURE_RECTANGL", GL_TEXTURE_RECTANGL},
#endif
#ifdef GL_TEXTURE_BINDING_RECTANGL
  {"TEXTURE_BINDING_RECTANGL", GL_TEXTURE_BINDING_RECTANGL},
#endif
#ifdef GL_PROXY_TEXTURE_RECTANGL
  {"PROXY_TEXTURE_RECTANGL", GL_PROXY_TEXTURE_RECTANGL},
#endif
#ifdef GL_MAX_RECTANGLE_TEXTURE_SIZ
  {"MAX_RECTANGLE_TEXTURE_SIZ", GL_MAX_RECTANGLE_TEXTURE_SIZ},
#endif
#ifdef GL_R8_SNOR
  {"R8_SNOR", GL_R8_SNOR},
#endif
#ifdef GL_RG8_SNOR
  {"RG8_SNOR", GL_RG8_SNOR},
#endif
#ifdef GL_RGB8_SNOR
  {"RGB8_SNOR", GL_RGB8_SNOR},
#endif
#ifdef GL_RGBA8_SNOR
  {"RGBA8_SNOR", GL_RGBA8_SNOR},
#endif
#ifdef GL_R16_SNOR
  {"R16_SNOR", GL_R16_SNOR},
#endif
#ifdef GL_RG16_SNOR
  {"RG16_SNOR", GL_RG16_SNOR},
#endif
#ifdef GL_RGB16_SNOR
  {"RGB16_SNOR", GL_RGB16_SNOR},
#endif
#ifdef GL_RGBA16_SNOR
  {"RGBA16_SNOR", GL_RGBA16_SNOR},
#endif
#ifdef GL_SIGNED_NORMALIZE
  {"SIGNED_NORMALIZE", GL_SIGNED_NORMALIZE},
#endif
#ifdef GL_PRIMITIVE_RESTAR
  {"PRIMITIVE_RESTAR", GL_PRIMITIVE_RESTAR},
#endif
#ifdef GL_PRIMITIVE_RESTART_INDE
  {"PRIMITIVE_RESTART_INDE", GL_PRIMITIVE_RESTART_INDE},
#endif
#ifdef GL_COPY_READ_BUFFE
  {"COPY_READ_BUFFE", GL_COPY_READ_BUFFE},
#endif
#ifdef GL_COPY_WRITE_BUFFE
  {"COPY_WRITE_BUFFE", GL_COPY_WRITE_BUFFE},
#endif
#ifdef GL_UNIFORM_BUFFE
  {"UNIFORM_BUFFE", GL_UNIFORM_BUFFE},
#endif
#ifdef GL_UNIFORM_BUFFER_BINDIN
  {"UNIFORM_BUFFER_BINDIN", GL_UNIFORM_BUFFER_BINDIN},
#endif
#ifdef GL_UNIFORM_BUFFER_STAR
  {"UNIFORM_BUFFER_STAR", GL_UNIFORM_BUFFER_STAR},
#endif
#ifdef GL_UNIFORM_BUFFER_SIZ
  {"UNIFORM_BUFFER_SIZ", GL_UNIFORM_BUFFER_SIZ},
#endif
#ifdef GL_MAX_VERTEX_UNIFORM_BLOCK
  {"MAX_VERTEX_UNIFORM_BLOCK", GL_MAX_VERTEX_UNIFORM_BLOCK},
#endif
#ifdef GL_MAX_GEOMETRY_UNIFORM_BLOCK
  {"MAX_GEOMETRY_UNIFORM_BLOCK", GL_MAX_GEOMETRY_UNIFORM_BLOCK},
#endif
#ifdef GL_MAX_FRAGMENT_UNIFORM_BLOCK
  {"MAX_FRAGMENT_UNIFORM_BLOCK", GL_MAX_FRAGMENT_UNIFORM_BLOCK},
#endif
#ifdef GL_MAX_COMBINED_UNIFORM_BLOCK
  {"MAX_COMBINED_UNIFORM_BLOCK", GL_MAX_COMBINED_UNIFORM_BLOCK},
#endif
#ifdef GL_MAX_UNIFORM_BUFFER_BINDING
  {"MAX_UNIFORM_BUFFER_BINDING", GL_MAX_UNIFORM_BUFFER_BINDING},
#endif
#ifdef GL_MAX_UNIFORM_BLOCK_SIZ
  {"MAX_UNIFORM_BLOCK_SIZ", GL_MAX_UNIFORM_BLOCK_SIZ},
#endif
#ifdef GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENT
  {"MAX_COMBINED_VERTEX_UNIFORM_COMPONENT", GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENT},
#endif
#ifdef GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENT
  {"MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENT", GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENT},
#endif
#ifdef GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENT
  {"MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENT", GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENT},
#endif
#ifdef GL_UNIFORM_BUFFER_OFFSET_ALIGNMEN
  {"UNIFORM_BUFFER_OFFSET_ALIGNMEN", GL_UNIFORM_BUFFER_OFFSET_ALIGNMEN},
#endif
#ifdef GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGT
  {"ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGT", GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGT},
#endif
#ifdef GL_ACTIVE_UNIFORM_BLOCK
  {"ACTIVE_UNIFORM_BLOCK", GL_ACTIVE_UNIFORM_BLOCK},
#endif
#ifdef GL_UNIFORM_TYP
  {"UNIFORM_TYP", GL_UNIFORM_TYP},
#endif
#ifdef GL_UNIFORM_SIZ
  {"UNIFORM_SIZ", GL_UNIFORM_SIZ},
#endif
#ifdef GL_UNIFORM_NAME_LENGT
  {"UNIFORM_NAME_LENGT", GL_UNIFORM_NAME_LENGT},
#endif
#ifdef GL_UNIFORM_BLOCK_INDE
  {"UNIFORM_BLOCK_INDE", GL_UNIFORM_BLOCK_INDE},
#endif
#ifdef GL_UNIFORM_OFFSE
  {"UNIFORM_OFFSE", GL_UNIFORM_OFFSE},
#endif
#ifdef GL_UNIFORM_ARRAY_STRID
  {"UNIFORM_ARRAY_STRID", GL_UNIFORM_ARRAY_STRID},
#endif
#ifdef GL_UNIFORM_MATRIX_STRID
  {"UNIFORM_MATRIX_STRID", GL_UNIFORM_MATRIX_STRID},
#endif
#ifdef GL_UNIFORM_IS_ROW_MAJO
  {"UNIFORM_IS_ROW_MAJO", GL_UNIFORM_IS_ROW_MAJO},
#endif
#ifdef GL_UNIFORM_BLOCK_BINDIN
  {"UNIFORM_BLOCK_BINDIN", GL_UNIFORM_BLOCK_BINDIN},
#endif
#ifdef GL_UNIFORM_BLOCK_DATA_SIZ
  {"UNIFORM_BLOCK_DATA_SIZ", GL_UNIFORM_BLOCK_DATA_SIZ},
#endif
#ifdef GL_UNIFORM_BLOCK_NAME_LENGT
  {"UNIFORM_BLOCK_NAME_LENGT", GL_UNIFORM_BLOCK_NAME_LENGT},
#endif
#ifdef GL_UNIFORM_BLOCK_ACTIVE_UNIFORM
  {"UNIFORM_BLOCK_ACTIVE_UNIFORM", GL_UNIFORM_BLOCK_ACTIVE_UNIFORM},
#endif
#ifdef GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICE
  {"UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICE", GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICE},
#endif
#ifdef GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADE
  {"UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADE", GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADE},
#endif
#ifdef GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADE
  {"UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADE", GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADE},
#endif
#ifdef GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADE
  {"UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADE", GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADE},
#endif
#ifdef GL_INVALID_INDE
  {"INVALID_INDE", GL_INVALID_INDE},
#endif
#ifdef GL_CONTEXT_CORE_PROFILE_BI
  {"CONTEXT_CORE_PROFILE_BI", GL_CONTEXT_CORE_PROFILE_BI},
#endif
#ifdef GL_CONTEXT_COMPATIBILITY_PROFILE_BI
  {"CONTEXT_COMPATIBILITY_PROFILE_BI", GL_CONTEXT_COMPATIBILITY_PROFILE_BI},
#endif
#ifdef GL_LINES_ADJACENC
  {"LINES_ADJACENC", GL_LINES_ADJACENC},
#endif
#ifdef GL_LINE_STRIP_ADJACENC
  {"LINE_STRIP_ADJACENC", GL_LINE_STRIP_ADJACENC},
#endif
#ifdef GL_TRIANGLES_ADJACENC
  {"TRIANGLES_ADJACENC", GL_TRIANGLES_ADJACENC},
#endif
#ifdef GL_TRIANGLE_STRIP_ADJACENC
  {"TRIANGLE_STRIP_ADJACENC", GL_TRIANGLE_STRIP_ADJACENC},
#endif
#ifdef GL_PROGRAM_POINT_SIZ
  {"PROGRAM_POINT_SIZ", GL_PROGRAM_POINT_SIZ},
#endif
#ifdef GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNIT
  {"MAX_GEOMETRY_TEXTURE_IMAGE_UNIT", GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNIT},
#endif
#ifdef GL_FRAMEBUFFER_ATTACHMENT_LAYERE
  {"FRAMEBUFFER_ATTACHMENT_LAYERE", GL_FRAMEBUFFER_ATTACHMENT_LAYERE},
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGET
  {"FRAMEBUFFER_INCOMPLETE_LAYER_TARGET", GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGET},
#endif
#ifdef GL_GEOMETRY_SHADE
  {"GEOMETRY_SHADE", GL_GEOMETRY_SHADE},
#endif
#ifdef GL_GEOMETRY_VERTICES_OU
  {"GEOMETRY_VERTICES_OU", GL_GEOMETRY_VERTICES_OU},
#endif
#ifdef GL_GEOMETRY_INPUT_TYP
  {"GEOMETRY_INPUT_TYP", GL_GEOMETRY_INPUT_TYP},
#endif
#ifdef GL_GEOMETRY_OUTPUT_TYP
  {"GEOMETRY_OUTPUT_TYP", GL_GEOMETRY_OUTPUT_TYP},
#endif
#ifdef GL_MAX_GEOMETRY_UNIFORM_COMPONENT
  {"MAX_GEOMETRY_UNIFORM_COMPONENT", GL_MAX_GEOMETRY_UNIFORM_COMPONENT},
#endif
#ifdef GL_MAX_GEOMETRY_OUTPUT_VERTICE
  {"MAX_GEOMETRY_OUTPUT_VERTICE", GL_MAX_GEOMETRY_OUTPUT_VERTICE},
#endif
#ifdef GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENT
  {"MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENT", GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENT},
#endif
#ifdef GL_MAX_VERTEX_OUTPUT_COMPONENT
  {"MAX_VERTEX_OUTPUT_COMPONENT", GL_MAX_VERTEX_OUTPUT_COMPONENT},
#endif
#ifdef GL_MAX_GEOMETRY_INPUT_COMPONENT
  {"MAX_GEOMETRY_INPUT_COMPONENT", GL_MAX_GEOMETRY_INPUT_COMPONENT},
#endif
#ifdef GL_MAX_GEOMETRY_OUTPUT_COMPONENT
  {"MAX_GEOMETRY_OUTPUT_COMPONENT", GL_MAX_GEOMETRY_OUTPUT_COMPONENT},
#endif
#ifdef GL_MAX_FRAGMENT_INPUT_COMPONENT
  {"MAX_FRAGMENT_INPUT_COMPONENT", GL_MAX_FRAGMENT_INPUT_COMPONENT},
#endif
#ifdef GL_CONTEXT_PROFILE_MAS
  {"CONTEXT_PROFILE_MAS", GL_CONTEXT_PROFILE_MAS},
#endif
#ifdef GL_DEPTH_CLAM
  {"DEPTH_CLAM", GL_DEPTH_CLAM},
#endif
#ifdef GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTIO
  {"QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTIO", GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTIO},
#endif
#ifdef GL_FIRST_VERTEX_CONVENTIO
  {"FIRST_VERTEX_CONVENTIO", GL_FIRST_VERTEX_CONVENTIO},
#endif
#ifdef GL_LAST_VERTEX_CONVENTIO
  {"LAST_VERTEX_CONVENTIO", GL_LAST_VERTEX_CONVENTIO},
#endif
#ifdef GL_PROVOKING_VERTE
  {"PROVOKING_VERTE", GL_PROVOKING_VERTE},
#endif
#ifdef GL_TEXTURE_CUBE_MAP_SEAMLES
  {"TEXTURE_CUBE_MAP_SEAMLES", GL_TEXTURE_CUBE_MAP_SEAMLES},
#endif
#ifdef GL_MAX_SERVER_WAIT_TIMEOU
  {"MAX_SERVER_WAIT_TIMEOU", GL_MAX_SERVER_WAIT_TIMEOU},
#endif
#ifdef GL_OBJECT_TYP
  {"OBJECT_TYP", GL_OBJECT_TYP},
#endif
#ifdef GL_SYNC_CONDITIO
  {"SYNC_CONDITIO", GL_SYNC_CONDITIO},
#endif
#ifdef GL_SYNC_STATU
  {"SYNC_STATU", GL_SYNC_STATU},
#endif
#ifdef GL_SYNC_FLAG
  {"SYNC_FLAG", GL_SYNC_FLAG},
#endif
#ifdef GL_SYNC_FENC
  {"SYNC_FENC", GL_SYNC_FENC},
#endif
#ifdef GL_SYNC_GPU_COMMANDS_COMPLET
  {"SYNC_GPU_COMMANDS_COMPLET", GL_SYNC_GPU_COMMANDS_COMPLET},
#endif
#ifdef GL_UNSIGNALE
  {"UNSIGNALE", GL_UNSIGNALE},
#endif
#ifdef GL_SIGNALE
  {"SIGNALE", GL_SIGNALE},
#endif
#ifdef GL_ALREADY_SIGNALE
  {"ALREADY_SIGNALE", GL_ALREADY_SIGNALE},
#endif
#ifdef GL_TIMEOUT_EXPIRE
  {"TIMEOUT_EXPIRE", GL_TIMEOUT_EXPIRE},
#endif
#ifdef GL_CONDITION_SATISFIE
  {"CONDITION_SATISFIE", GL_CONDITION_SATISFIE},
#endif
#ifdef GL_WAIT_FAILE
  {"WAIT_FAILE", GL_WAIT_FAILE},
#endif
#ifdef GL_TIMEOUT_IGNORE
  {"TIMEOUT_IGNORE", GL_TIMEOUT_IGNORE},
#endif
#ifdef GL_SYNC_FLUSH_COMMANDS_BI
  {"SYNC_FLUSH_COMMANDS_BI", GL_SYNC_FLUSH_COMMANDS_BI},
#endif
#ifdef GL_SAMPLE_POSITIO
  {"SAMPLE_POSITIO", GL_SAMPLE_POSITIO},
#endif
#ifdef GL_SAMPLE_MAS
  {"SAMPLE_MAS", GL_SAMPLE_MAS},
#endif
#ifdef GL_SAMPLE_MASK_VALU
  {"SAMPLE_MASK_VALU", GL_SAMPLE_MASK_VALU},
#endif
#ifdef GL_MAX_SAMPLE_MASK_WORD
  {"MAX_SAMPLE_MASK_WORD", GL_MAX_SAMPLE_MASK_WORD},
#endif
#ifdef GL_TEXTURE_2D_MULTISAMPL
  {"TEXTURE_2D_MULTISAMPL", GL_TEXTURE_2D_MULTISAMPL},
#endif
#ifdef GL_PROXY_TEXTURE_2D_MULTISAMPL
  {"PROXY_TEXTURE_2D_MULTISAMPL", GL_PROXY_TEXTURE_2D_MULTISAMPL},
#endif
#ifdef GL_TEXTURE_2D_MULTISAMPLE_ARRA
  {"TEXTURE_2D_MULTISAMPLE_ARRA", GL_TEXTURE_2D_MULTISAMPLE_ARRA},
#endif
#ifdef GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRA
  {"PROXY_TEXTURE_2D_MULTISAMPLE_ARRA", GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRA},
#endif
#ifdef GL_TEXTURE_BINDING_2D_MULTISAMPL
  {"TEXTURE_BINDING_2D_MULTISAMPL", GL_TEXTURE_BINDING_2D_MULTISAMPL},
#endif
#ifdef GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRA
  {"TEXTURE_BINDING_2D_MULTISAMPLE_ARRA", GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRA},
#endif
#ifdef GL_TEXTURE_SAMPLE
  {"TEXTURE_SAMPLE", GL_TEXTURE_SAMPLE},
#endif
#ifdef GL_TEXTURE_FIXED_SAMPLE_LOCATION
  {"TEXTURE_FIXED_SAMPLE_LOCATION", GL_TEXTURE_FIXED_SAMPLE_LOCATION},
#endif
#ifdef GL_SAMPLER_2D_MULTISAMPL
  {"SAMPLER_2D_MULTISAMPL", GL_SAMPLER_2D_MULTISAMPL},
#endif
#ifdef GL_INT_SAMPLER_2D_MULTISAMPL
  {"INT_SAMPLER_2D_MULTISAMPL", GL_INT_SAMPLER_2D_MULTISAMPL},
#endif
#ifdef GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPL
  {"UNSIGNED_INT_SAMPLER_2D_MULTISAMPL", GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPL},
#endif
#ifdef GL_SAMPLER_2D_MULTISAMPLE_ARRA
  {"SAMPLER_2D_MULTISAMPLE_ARRA", GL_SAMPLER_2D_MULTISAMPLE_ARRA},
#endif
#ifdef GL_INT_SAMPLER_2D_MULTISAMPLE_ARRA
  {"INT_SAMPLER_2D_MULTISAMPLE_ARRA", GL_INT_SAMPLER_2D_MULTISAMPLE_ARRA},
#endif
#ifdef GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRA
  {"UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRA", GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRA},
#endif
#ifdef GL_MAX_COLOR_TEXTURE_SAMPLE
  {"MAX_COLOR_TEXTURE_SAMPLE", GL_MAX_COLOR_TEXTURE_SAMPLE},
#endif
#ifdef GL_MAX_DEPTH_TEXTURE_SAMPLE
  {"MAX_DEPTH_TEXTURE_SAMPLE", GL_MAX_DEPTH_TEXTURE_SAMPLE},
#endif
#ifdef GL_MAX_INTEGER_SAMPLE
  {"MAX_INTEGER_SAMPLE", GL_MAX_INTEGER_SAMPLE},
#endif
#ifdef GL_VERTEX_ATTRIB_ARRAY_DIVISO
  {"VERTEX_ATTRIB_ARRAY_DIVISO", GL_VERTEX_ATTRIB_ARRAY_DIVISO},
#endif
#ifdef GL_SRC1_COLO
  {"SRC1_COLO", GL_SRC1_COLO},
#endif
#ifdef GL_ONE_MINUS_SRC1_COLO
  {"ONE_MINUS_SRC1_COLO", GL_ONE_MINUS_SRC1_COLO},
#endif
#ifdef GL_ONE_MINUS_SRC1_ALPH
  {"ONE_MINUS_SRC1_ALPH", GL_ONE_MINUS_SRC1_ALPH},
#endif
#ifdef GL_MAX_DUAL_SOURCE_DRAW_BUFFER
  {"MAX_DUAL_SOURCE_DRAW_BUFFER", GL_MAX_DUAL_SOURCE_DRAW_BUFFER},
#endif
#ifdef GL_ANY_SAMPLES_PASSE
  {"ANY_SAMPLES_PASSE", GL_ANY_SAMPLES_PASSE},
#endif
#ifdef GL_SAMPLER_BINDIN
  {"SAMPLER_BINDIN", GL_SAMPLER_BINDIN},
#endif
#ifdef GL_RGB10_A2U
  {"RGB10_A2U", GL_RGB10_A2U},
#endif
#ifdef GL_TEXTURE_SWIZZLE_
  {"TEXTURE_SWIZZLE_", GL_TEXTURE_SWIZZLE_},
#endif
#ifdef GL_TEXTURE_SWIZZLE_
  {"TEXTURE_SWIZZLE_", GL_TEXTURE_SWIZZLE_},
#endif
#ifdef GL_TEXTURE_SWIZZLE_
  {"TEXTURE_SWIZZLE_", GL_TEXTURE_SWIZZLE_},
#endif
#ifdef GL_TEXTURE_SWIZZLE_
  {"TEXTURE_SWIZZLE_", GL_TEXTURE_SWIZZLE_},
#endif
#ifdef GL_TEXTURE_SWIZZLE_RGB
  {"TEXTURE_SWIZZLE_RGB", GL_TEXTURE_SWIZZLE_RGB},
#endif
#ifdef GL_TIME_ELAPSE
  {"TIME_ELAPSE", GL_TIME_ELAPSE},
#endif
#ifdef GL_TIMESTAM
  {"TIMESTAM", GL_TIMESTAM},
#endif
#ifdef GL_INT_2_10_10_10_RE
  {"INT_2_10_10_10_RE", GL_INT_2_10_10_10_RE},
#endif

    ));

    ks_dict_merge(res->attr, E_gl->attr);



    return res;
}


/* C-extension */
KS_CEXT_DECL(get);
