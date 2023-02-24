/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef OBJ_READER_H
#define OBJ_READER_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <string>
#include "geometry.h"

/** helper class for representing mtl format materials */
class MTLMaterial {
public:
	std::string name;
	float Kd[3] = {0, 0, 0};
	float Ke[3] = {0, 0, 0};
	float Ns = 0;
	float Ni = 0;
	float d = 0;

	MTLMaterial(std::string name) : name{name} {}
};

class ObjReader {
public:
	std::string fname_base;
	std::ifstream mtl_file;
	std::ifstream obj_file;

	std::vector<MTLMaterial> mtl_materials;
	std::vector<Vec> vertices;

	std::unordered_map<std::string, std::shared_ptr<Material>> mat_table;

	std::vector<std::shared_ptr<Face>> all_faces;
	std::vector<std::shared_ptr<Material>> all_materials;

	ObjReader(const char *fname_base);

	void parse_mtl();
	void create_all_materials();

	void parse_obj();
};

#endif /* OBJ_READER_H */
