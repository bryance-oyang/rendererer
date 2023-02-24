/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/**
 * @file
 * @brief Basic parsing of .obj and .mtl files.
 */

#include <sstream>
#include "obj_reader.h"
#include "material.h"

ObjReader::ObjReader(const char *fname_base)
{
	std::string fname_base_str{fname_base};
	mtl_file = std::ifstream{fname_base_str + std::string{".mtl"}};
	obj_file = std::ifstream{fname_base_str + std::string{".obj"}};

	parse_mtl();
	create_all_materials();
	parse_obj();
}

ObjReader::ObjReader(const char *obj_fname, const char *mtl_fname)
{
	mtl_file = std::ifstream{mtl_fname};
	obj_file = std::ifstream{obj_fname};

	parse_mtl();
	create_all_materials();
	parse_obj();
}


void ObjReader::parse_mtl()
{
	std::string ignore;
	std::string name;

	std::string line;
	while (std::getline(mtl_file, line)) {
		std::istringstream sline{line};

		if (line.rfind("newmtl ", 0) == 0) {
			sline >> ignore >> name;
			mtl_materials.emplace_back(name);
			continue;
		}

		MTLMaterial &mat = mtl_materials.back();

		if (line.rfind("Kd ", 0) == 0) {
			// Kd float float float
			sline >> ignore;
			for (int i = 0; i < 3; i++) {
				sline >> mat.Kd[i];
			}
		} else if (line.rfind("Ke ", 0) == 0) {
			// Ke float float float
			sline >> ignore;
			for (int i = 0; i < 3; i++) {
				sline >> mat.Ke[i];
			}
		} else if (line.rfind("Ni ", 0) == 0) {
			// Ni float
			sline >> ignore >> mat.Ni;
		} else if (line.rfind("d ", 0) == 0) {
			// d float
			sline >> ignore >> mat.d;
		}
	}
}

void ObjReader::create_all_materials()
{
	// default material
	float default_color[3] = {0.8,0.8,0.8};
	all_materials.push_back(std::make_shared<DiffuseMaterial>(default_color));

	// from obj file
	for (auto &mtl_mat : mtl_materials) {
		if (mtl_mat.Ke[0] > 0 || mtl_mat.Ke[1] > 0 || mtl_mat.Ke[2] > 0) {
			// emitter
			all_materials.push_back(std::make_shared<EmitterMaterial>(mtl_mat.Ke));
		} else if (mtl_mat.d < 1) {
			// glass
			all_materials.push_back(std::make_shared<GlassMaterial>(mtl_mat.Ni));
		} else {
			// diffuse
			all_materials.push_back(std::make_shared<DiffuseMaterial>(mtl_mat.Kd));
		}

		mat_table[mtl_mat.name] = all_materials.back();
	}
}

void ObjReader::parse_obj()
{
	std::string ignore;
	std::string name;
	std::string tmp;
	int vind[3];
	float floats[3];
	float xyz[3];

	// default material to begin
	std::shared_ptr<Material> cur_material = all_materials[0];

	std::string line;
	while (std::getline(obj_file, line)) {
		std::istringstream sline{line};

		if (line.rfind("v ", 0) == 0) {
			// v float float float
			sline >> ignore;
			for (int i = 0; i < 3; i++) {
				sline >> floats[i];
			}

			// obj format is weird
			xyz[0] = floats[0];
			xyz[2] = floats[1];
			xyz[1] = -floats[2];

			vertices.emplace_back(xyz);
		} else if (line.rfind("usemtl ", 0) == 0) {
			// usemtl name
			sline >> ignore >> name;
			cur_material = mat_table[name];
		} else if (line.rfind("f ", 0) == 0) {
			// f # # #
			// f #/# #/# #/#
			// f #/#/# #/#/# #/#/#
			sline >> ignore;

			for (int i = 0; i < 3; i++) {
				// tmp is # or #/# or #/#/#
				sline >> tmp;

				// get all chars until first nondigit
				std::string num_str;
				for (char &c : tmp) {
					if (!isdigit(c)) {
						break;
					}
					num_str += c;
				}

				// number is the vertex index (starts from 1 in .obj)
				vind[i] = std::stoi(num_str) - 1;
			}

			std::shared_ptr<Face> face = std::make_shared<Face>(vertices[vind[0]], vertices[vind[1]], vertices[vind[2]]);
			face->material = cur_material;
			all_faces.push_back(face);
		}
	}
}
