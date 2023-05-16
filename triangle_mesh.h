

#ifndef TRIANGLE_MESH_H
#define TRIANGLE_MESH_H

#include <vector>
#include <iostream>
#include <fstream>

#include "ray.h"
#include "triangle.h"
#include "material.h"




class triangle_mesh{

    public:
        triangle_mesh(){}
        triangle_mesh(const char* filename, shared_ptr<material> m) : mat(m){
            // generate face list, vertex index list, vertex list
            std::ifstream file;
            std::vector<double> verts;
            std::vector<double> normals;
            std::vector<double> uvs;  

        }


    public:
        std::vector<shared_ptr<triangle>> mesh;
        shared_ptr<material> mat; 


};


#endif