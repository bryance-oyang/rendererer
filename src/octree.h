/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef OCTREE_H
#define OCTREE_H

#include <vector>
#include <memory>
#include "material.h"

class Octree {
public:
	std::unique_ptr<Octree> sub[8];
	Box box;
	std::vector<Face> faces;

	Octree(Box &bounding_box, std::vector<std::shared_ptr<Face>> &all_faces,
		std::vector<std::shared_ptr<Box>> &bounding_boxes,
		size_t max_faces_per_box, size_t max_recursion_depth);
};

#endif /* OCTREE_H */
