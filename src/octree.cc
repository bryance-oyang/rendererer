/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "octree.h"

/** divides parent box into 8 children boxes */
static std::vector<Box> mk_sub_boxes(Box &parent)
{
	std::vector<Box> children;
	float tmp[2][3];
	float children_corners[8][2][3];

	/* set min for boxes: tmp[0] is old min, tmp[1] is mid */
	for (int i = 0; i < 3; i++) {
		tmp[0][i] = parent.corners[0][i];
		tmp[1][i] = (parent.corners[0][i] + parent.corners[1][i]) / 2;
	}
	/* boxes numbered in binary: e.g. 010 -> x use 0, y use 1, z use 0;
	use # meaning lower or upper for tmp */
	for (int i = 0; i < 8; i++) {
		children_corners[i][0][0] = tmp[!!(i & 4)][0];
		children_corners[i][0][1] = tmp[!!(i & 2)][1];
		children_corners[i][0][2] = tmp[i & 1][2];
	}

	/* set max for boxes: tmp[0] is mid, tmp[1] is old max */
	for (int i = 0; i < 3; i++) {
		tmp[0][i] = tmp[1][i];
		tmp[1][i] = parent.corners[1][i];
	}
	for (int i = 0; i < 8; i++) {
		children_corners[i][1][0] = tmp[!!(i & 4)][0];
		children_corners[i][1][1] = tmp[!!(i & 2)][1];
		children_corners[i][1][2] = tmp[i & 1][2];
	}

	for (int i = 0; i < 8; i++) {
		children.emplace_back(children_corners[i]);
	}

	return children;
}

Octree::Octree(Box &bounding_box, std::vector<std::shared_ptr<Face>> all_faces,
	std::vector<std::shared_ptr<Box>> bounding_boxes,
	size_t max_faces_per_box, size_t max_recursion_depth)
: box{bounding_box}
{
	// base case
	if (all_faces.size() <= max_faces_per_box || max_recursion_depth == 0) {
		for (auto &f : all_faces) {
			faces.emplace_back(*f);
		}
	}

	std::vector<Box> sub_boxes = mk_sub_boxes(bounding_box);

	// assign faces to sub
}
