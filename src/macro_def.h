/**
 * @file
 * @brief This contains the basic rendering settings and other useful misc
 */
#ifndef MACRO_DEF_H
#define MACRO_DEF_H

#include <cmath>

#define BENCHMARKING 0
#define SAMPLES_PER_BROADCAST ((unsigned long)(1 << 16))
#define MAX_BOUNCES_PER_PATH 8

#ifndef DEBUG
/* nondebug */

#define NTHREAD 8
#define IMAGE_WIDTH (1 << 8)
#define IMAGE_HEIGHT (1 << 8)
#define AVG_SAMPLE_PER_PIX (1 << 13)

#else /* DEBUG */
/* debug */

#define NTHREAD 8
#define IMAGE_WIDTH (1 << 2)
#define IMAGE_HEIGHT (1 << 2)
#define AVG_SAMPLE_PER_PIX (1 << 2)

#endif /* DEBUG */

#define SPEED_OF_LIGHT 299792458.0f
/** This should be 3 for direct srgb color any other for physical wavelengths */
#define NWAVELEN 100

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

/** this must be 1, or else material glass computation needs changing */
#define SPACE_INDEX_REFRACT 1.0f

#endif /* MACRO_DEF_H */
