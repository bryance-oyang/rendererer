/** @file
 * This contains the basic rendering settings and other useful misc
 */
#ifndef MACRO_DEF_H
#define MACRO_DEF_H

#include <cmath>

#define MAX_BOUNCES_PER_PATH 8
#define DO_MLT true
#define MLT_STARTUP_DISCARD 0.0f
#define MLT_FILM_WALK_FRAC (1.0f / 64.0f)
#define MLT_DIFFUSE_Z_WALK 0.01
#define MLT_RANDOM_DRAW_PROB 1.0f
#define MLT_MAX_CHAIN ((unsigned long)1 << 31)
#define MLT_RESTART_CHAIN_PROB (1.0f / (1 << 18))

#ifndef DEBUG
/* nondebug */
#define NTHREAD 8

#define AVG_SAMPLE_PER_PIX (1 << 10)

#define IMAGE_WIDTH (1 << 8)
#define IMAGE_HEIGHT (1 << 8)
#else /* DEBUG */
/* debug */
#define NTHREAD 1

#define AVG_SAMPLE_PER_PIX (1 << 10)
//#define AVG_SAMPLE_PER_PIX 10

#define IMAGE_WIDTH (1 << 7)
#define IMAGE_HEIGHT (1 << 7)
//#define IMAGE_WIDTH 10
//#define IMAGE_HEIGHT 10
#endif /* DEBUG */

/** This should be 3 for color or 1 for bw */
#define NFREQ 3

/** pi as float, not double for speed */
#define PI_F ((float)M_PI)
#define INV_PI_F ((float)(1.0f / PI_F))
#define INV_2PI_F ((float)(0.5f / PI_F))

/* to optimize branch prediction */
#ifndef unlikely
#define unlikely(x)		__builtin_expect(!!(x), 0)
#endif /* unlikely */

#ifndef likely
#define likely(x)		__builtin_expect(!!(x), 1)
#endif /* likely */

/** Sets a rough dynamic range. Bound division by zero: these occur if rays
 * are parallel to a plane and we try to solve for the intersection point.
 * See also global_characteristic_length_scale defined in scene.c */
#define GEOMETRY_EPSILON ((float)1e-5f)

/* octree */
#define OCTREE_MAX_FACE_PER_BOX 128
#define OCTREE_MAX_SUBDIV 6

#define SQR(x) ((x)*(x))
#define CUBE(x) ((x)*(x)*(x))

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#endif /* MACRO_DEF_H */
