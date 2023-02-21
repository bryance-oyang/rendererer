/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cfloat>
#include <vector>
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

/**
 * Recursively puts faces into octree structure. If the number of faces exceeds
 * max_faces_per_box, subdivide the box into 8 sub-boxes and recurse down. Does
 * not put faces into the box unless we are at the finest level with nfaces <
 * max_faces_per_box or max_recursion_depth is exceeded.
 *
 * @param bounding_box bounding box for the octree box
 * @param all_faces face list
 * @param faces_bounding_boxes bounding boxes for the faces
 * @param max_faces_per_box if exceeded by nfaces, we subdivide the box into 8
 * and recurse, splitting faces into the boxes they belong in
 * @param max_recursion_depth maximum additional number of times to
 * subdivide/refine octree, overruling max_faces_per_box, gets -- every
 * recursive call
 */
Octree::Octree(Box &bounding_box, std::vector<std::shared_ptr<Face>> &all_faces,
	std::vector<std::shared_ptr<Box>> &faces_bounding_boxes,
	size_t max_faces_per_box, size_t max_recursion_depth)
: box{bounding_box}
{
	// base case
	if (all_faces.size() <= max_faces_per_box || max_recursion_depth == 0) {
		for (auto &f : all_faces) {
			faces.emplace_back(*f);
		}
		terminal = true;
		return;
	}

	// recursive case
	terminal = false;
	std::vector<Box> sub_boxes = mk_sub_boxes(bounding_box);

	// assign faces to sub
	std::vector<std::shared_ptr<Face>> sub_all_faces[8];
	std::vector<std::shared_ptr<Box>> sub_bounding_boxes[8];
	for (size_t i = 0; i < all_faces.size(); i++) {
		for (int j = 0; j < 8; j++) {
			if (box_touch_box(*faces_bounding_boxes[i], sub_boxes[j])) {
				sub_all_faces[j].emplace_back(all_faces[i]);
				sub_bounding_boxes[j].emplace_back(faces_bounding_boxes[i]);
			}
		}
	}

	// recurse into sub-boxes
	for (int i = 0; i < 8; i++) {
		sub[i] = std::make_unique<Octree>(sub_boxes[i], sub_all_faces[i],
			sub_bounding_boxes[i], max_faces_per_box, max_recursion_depth - 1);
	}
}

/** find first intersection of ray with triangles in base */
std::unique_ptr<Intersection> Octree::_base_intersect(Ray &r)
{
	std::unique_ptr<Intersection> intersection;
	float tmin = FLT_MAX;

	// assume intersection not found to begin
	intersection = nullptr;

	// find first intersection by lowest t
	for (auto &face : faces) {
		Vec point;
		float t = ray_face_intersect(point, r, face);
		if (t > 0 && t < tmin && vec_in_box(point, box)) {
			intersection = std::make_unique<Intersection>(point, face);
		}
	}

	return intersection;
}

/**
 * Check that i not in order[0...n-1]
 */
static bool not_in(int i, int n, const int *order)
{
	for (int j = 0; j < n; j++) {
		if (i == order[j]) {
			return false;
		}
	}
	return true;
}

/**
 * This function is to find the nth closest box hit starting with n = 0, given
 * box_hit_times. Ignores negative entries in box_hit_times. Must have called
 * already for n < current n so that order[0...(n-1)] have correct values.
 *
 * @param n nth closest to find, starting at 0
 * @param order This function sets order[n] given order[0...(n-1)]. A list of
 * indices from 0...(n-1) indicating the order: box_hit_times[order[0]] is the
 * smallest, box_hit_times[order[1]] is 2nd smallest, etc, but ignores negative
 * entries in box_hit_times. If nth smallest is not found, puts -1 into
 * order[n].
 * @param box_hit_times the parameter t of the ray when box is hit, or negative if not hit
 */
static void order_hit_boxes(int n, int *order, const float *box_hit_times)
{
	float nth_smallest = FLT_MAX;
	order[n] = -1;
	for (int i = 0; i < 8; i++) {
		if (box_hit_times[i] < 0)
			continue;
		if (box_hit_times[i] <= nth_smallest) {
			if (n == 0 || box_hit_times[order[n-1]] < box_hit_times[i] || (box_hit_times[order[n-1]] == box_hit_times[i] && not_in(i, n, order))) {
				nth_smallest = box_hit_times[i];
				order[n] = i;
			}
		}
	}
}

/** recursively find first intersection with face in octree */
std::unique_ptr<Intersection> Octree::first_ray_face_intercept(Ray &r)
{
	// base case
	if (this->terminal) {
		return _base_intersect(r);
	}

	/* first check if ray origin inside box */
	int origin_box = -1;
	for (int i = 0; i < 8; i++) {
		if (vec_in_box(r.orig, sub[i]->box)) {
			origin_box = i;
			break;
		}
	}
	if (origin_box >= 0) {
		auto result = sub[origin_box]->first_ray_face_intercept(r);
		if (result) {
			return result;
		}
	}

	/* find interceptions with sub-boxes and the times they are hit */
	float box_hit_times[8]; /* set to -1 if not hit */
	int order[8];
	for (int i = 0; i < 8; i++) {
		box_hit_times[i] = ray_box_intersect(r, sub[i]->box);
	}

	int i = 0;
	if (origin_box >= 0) {
		/* skip origin_box since already searched */
		order[0] = origin_box;
		i = 1;
	}
	/* recurse into each sub-box in order until hit is found */
	for (; i < 8; i++) {
		order_hit_boxes(i, order, box_hit_times);
		if (order[i] < 0) {
			return nullptr;
		}

		auto result = sub[order[i]]->first_ray_face_intercept(r);
		if (result) {
			return result;
		}
	}
	return nullptr;
}
