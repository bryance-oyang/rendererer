/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/**
 * @file
 */

#include <sstream>
#include "obj_reader.h"
#include "material.h"

ObjReader::ObjReader(const char *fname_base) : fname_base{fname_base}
{
	mtl_file = std::ifstream(this->fname_base + std::string{".mtl"});
	obj_file = std::ifstream(this->fname_base + std::string{".obj"});

	parse_mtl();
	create_all_materials();

	parse_obj();
	create_all_faces();
}

void ObjReader::parse_mtl()
{
	std::string ignore;
	std::string name;

	std::string line;
	while (std::getline(mtl_file, line)) {
		std::istringstream sline{line};

		if (line.rfind("newmtl", 0) == 0) {
			sline >> ignore >> name;
			mtl_materials.emplace_back(name);
			continue;
		}

		MTLMaterial &mat = mtl_materials.back();

		if (line.rfind("Kd", 0) == 0) {
			// Kd float float float
			sline >> ignore;
			for (int i = 0; i < 3; i++) {
				sline >> mat.Kd[i];
			}
		} else if (line.rfind("Ke", 0) == 0) {
			// Ke float float float
			sline >> ignore;
			for (int i = 0; i < 3; i++) {
				sline >> mat.Ke[i];
			}
		} else if (line.rfind("Ni", 0) == 0) {
			// Ni float
			sline >> ignore >> mat.Ni;
		} else if (line.rfind("d", 0) == 0) {
			// d float
			sline >> ignore >> mat.d;
		}
	}
}

void ObjReader::create_all_materials()
{
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

	std::string line;
	while (std::getline(obj_file, line)) {
		std::istringstream sline{line};

		if (line.rfind("o", 0) == 0) {
			sline >> ignore >> name;
			obj_objects.emplace_back();
			continue;
		}

		OBJObject &obj = obj_objects.back();

		if (line.rfind("v", 0) == 0) {
			// v float float float
			sline >> ignore;
			for (int i = 0; i < 3; i++) {
				sline >> floats[i];
			}
			obj.vertices.emplace_back(floats);
		} else if (line.rfind("usemtl", 0) == 0) {
			// usemtl name
			sline >> ignore >> name;
			obj.mat_name = name;
		} else if (line.rfind("f", 0) == 0) {
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

				// number is the vertex index
				vind[i] = std::stoi(num_str);
			}

			obj.faces.push_back(std::make_shared<Face>(obj.vertices[vind[0]], obj.vertices[vind[1]], obj.vertices[vind[2]]));
		}
	}
}

void ObjReader::create_all_faces()
{
	for (auto &object : obj_objects) {
		for (auto &face : object.faces) {
			face->material = mat_table[object.mat_name];
			all_faces.push_back(face);
		}
	}
}
