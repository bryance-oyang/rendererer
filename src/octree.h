/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef OCTREE_H
#define OCTREE_H

#include "geometry.h"

/** octree used to optimize ray face intersection finding */
class Octree {
public:
	/** octree children */
	std::unique_ptr<Octree> sub[8];
	/** bounding box for octree */
	Box box;
	/** faces that are in the box */
	std::vector<Face> faces;
	/** true if no more sub octrees */
	bool terminal;

	Octree() {};
	Octree(const Box &bounding_box, const std::vector<Face*> &all_faces,
		const std::vector<std::shared_ptr<Box>> &faces_bounding_boxes,
		size_t max_faces_per_box, size_t max_recursion_depth);

	bool _base_intersect(Vec *point, Face **face, const Ray &r);
	bool first_ray_face_intersect(Vec *point, Face **face, const Ray &r);
};

#endif /* OCTREE_H */
