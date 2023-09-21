

#ifndef TRIANGLE_MESH_H
#define TRIANGLE_MESH_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#include <algorithm>
#include <iostream>
#include <unordered_map>

#include "ray.h"
#include "triangle.h"
#include "material.h"
#include "rtweekend.h"
#include "bvh.h"


// .obj reference  https://en.wikipedia.org/wiki/Wavefront_.obj_file
// .obj vertices can be represented in one of the following formats:
// vertex coordinates, uv_coordinates, and normal coordinates
// vertex coordinates, uv coordinates
// vertex coordinates, normal coordinates
// vertex coordinates

enum vertex_format{
    V_UV_NORMAL = 7,
    V_UV = 3,
    V_NORMAL = 5,
    V = 1

};

// defines an integer code for each format
enum vertex_info{
    VERTEX = 1, 
    UV = 2,
    NORMAL = 4
};


class triangle_mesh : public hittable{

    public:
        triangle_mesh(){}
        triangle_mesh(const char* filename, shared_ptr<material> m, int w) : mat_ptr(m), winding(w){
            

            std::ifstream file;

            try{   

                file.open(filename);
                if (file.fail()) throw;
                std::string line = "";   
                std::istringstream iss;
                std::string buffer;
                float param;

                vertex_format input_format_code = V;


                while(getline(file, line)){
                    iss.clear();
                    iss.str(line);
                    iss >> buffer;
                    
                    if(buffer.compare("vt") == 0){
                        input_format_code = static_cast<vertex_format>(input_format_code | UV);
                        vec2 uv;
                        iss >> uv[0] >> uv[1];
                        uvs.push_back(uv);

                    }else if(buffer.compare("vn") == 0){
                        input_format_code =  static_cast<vertex_format>(input_format_code | NORMAL);
                        vec3 normal;
                        iss >> normal[0] >> normal[1] >> normal[2];
                        normals.push_back(normal);
                        

                    }else if(buffer.compare("v") == 0){
                        input_format_code =  static_cast<vertex_format>(input_format_code | VERTEX);
                        vec3 vert;
                        iss >> vert[0] >> vert[1] >> vert[2];
                        verts.push_back(vert);
                    }else if(buffer.compare("f") == 0){
                        // tokenize the strings based on delimeter
                        // extract indices from each token

                        while(!iss.eof()){
                            if(!(iss >> buffer))
                                break;
                            
                            
                            verts_idx.push_back(0);
                            normals_idx.push_back(0);
                            uvs_idx.push_back(0);
                            
                            switch(input_format_code){
                                // case with verts, uvs, normals
                                case(V_UV_NORMAL):
                                    sscanf(buffer.c_str(), "%i%*c%i%*c%i%*c", &verts_idx.back(), &uvs_idx.back(), &normals_idx.back());
                                    // convert line indices to array indices
                                    verts_idx.back() = verts_idx.back() - 1;
                                    uvs_idx.back() = uvs_idx.back() - 1;
                                    normals_idx.back() = normals_idx.back() - 1;
                                    
                                    break;
                                
                                // case with verts, normals
                                case(V_NORMAL):
                                    std::cerr<< "Vertex/Normal parsing not yet implemented."<<std::endl;
                                    break;

                                // case with verts, uvs
                                case(V_UV):
                                    sscanf(buffer.c_str(), "%i%*c%i%*c", &verts_idx.back(), &uvs_idx.back());
                                    
                                    //convert line indices to array indices
                                    verts_idx.back() = verts_idx.back() - 1;
                                    uvs_idx.back() = uvs_idx.back() - 1;
                                    break;


                                // case with only verts
                                case(V):
                                    verts_idx.back() = stoi(buffer);
                                    // convert line indices to array indices
                                    verts_idx.back() = verts_idx.back() - 1;
                                   
                                    break;    
                            }

                            

                        }

        


                        /**
                         * Triangulation Algorithm
                         */

                        int t = 0; 
                        int t_left = ( (t - 1 * winding) % (int)verts_idx.size() + (int)verts_idx.size() ) % (int)verts_idx.size();
                        int t_right = ( (t + 1 * winding) % (int)verts_idx.size() + (int)verts_idx.size() ) % (int)verts_idx.size();

                        while((int)verts_idx.size() > 3){
                            // confirm that three points form an 'ear'
                            if(is_ear(verts[verts_idx[t]],verts[verts_idx[t_right]],verts[verts_idx[t_left]],normals[normals_idx[t]])){

                                // loop over all other points, check if any one lies in triangle formed by excluded three
                                bool no_intersection = true;
                                for(int i = 0;  i < (int)verts_idx.size(); i++){
                                    if(i == t || i == t_left || i == t_right) continue;
                                    if(in_triangle(verts[verts_idx[t]], verts[verts_idx[t_right]], verts[verts_idx[t_left]], verts[verts_idx[i]])){
                                        no_intersection = false;
                                        break;
                                    }
                                }

                                if(no_intersection){



                                    // tris.push_back(make_shared<triangle>(verts[verts_idx[t]], verts[verts_idx[t_right]], verts[verts_idx[t_left]], normals[normals_idx[t]], mat_ptr, true));
                                    
                                    update_triangle_list(input_format_code, t, t_left, t_right);
                                    sanitize_indices(input_format_code, t);
                                    //
                                    // tris.push_back(make_shared<triangle>(verts[verts_idx[t]], verts[verts_idx[t_left]], verts[verts_idx[t_right]], 
                                    // uvs[uvs_idx[t]], uvs[uvs_idx[t_left]], uvs[uvs_idx[t_right]], 
                                    // normals[normals_idx[t]], normals[normals_idx[t_left]], normals[normals_idx[t_right]], 
                                    // mat_ptr, true));
                                    // sanitize indices 
                                    // verts_idx.erase(verts_idx.begin() + t);
                                    // uvs_idx.erase(uvs_idx.begin() + t);
                                    // normals_idx.erase(normals_idx.begin() + t);    

                                    t = 0;

                                }else{
                                    t = (t + 1) % (int)verts_idx.size();    
                                }

                            }else{
                                t = (t + 1) % (int)verts_idx.size();    
                            }

                            t_left = ( (t - 1 * winding) % (int)verts_idx.size() + (int)verts_idx.size() ) % (int)verts_idx.size();
                            t_right = ( (t + 1 * winding) % (int)verts_idx.size() + (int)verts_idx.size() ) % (int)verts_idx.size();
                        }
                        // 
                        // tris.push_back(make_shared<triangle>(verts[verts_idx[t]], verts[verts_idx[t_right]], verts[verts_idx[t_left]], normals[normals_idx[t]], mat_ptr, true));
                        

                        update_triangle_list(input_format_code, t, t_left, t_right);
                        clear_indices(input_format_code);
                        // 
                        // tris.push_back(make_shared<triangle>(verts[verts_idx[t]], verts[verts_idx[t_left]], verts[verts_idx[t_right]], 
                        // uvs[uvs_idx[t]], uvs[uvs_idx[t_left]], uvs[uvs_idx[t_right]], 
                        // normals[normals_idx[t]], normals[normals_idx[t_left]], normals[normals_idx[t_right]],
                        // mat_ptr, true));
                        // verts_idx.clear();
                        // uvs_idx.clear();
                        // normals_idx.clear();
                    
                    }


                }

                file.close();

            }catch(...){
                std::cerr<<"There was a problem with reading the mesh file.";
                file.close();
            }

            std::cerr<<"Mesh "<< filename <<" initialized."<<std::endl;
            mesh_bvh = make_shared<bvh_node>(tris, (size_t)0, tris.size(), 0, infinity);
            std::cerr<<"BVH Tree initialized."<<std::endl;


        }

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;
        // test if point r is in triangle defined by v0, v1, v2
        bool in_triangle(const vec3 &v0, const vec3 &v1, const vec3 &v2, const vec3 &vp);
        /*
        v0: 'left' indexed vertex
        v1: 'center' indexed vertex
        v2: 'right' indexed vertex
        */
        bool is_ear(const vec3& v0, const vec3 &v1, const vec3 &v2, vec3 &normal);



    public:
        


        std::vector<vec3> verts;
        std::vector<vec3> normals;
        std::vector<vec2> uvs;
        
        std::vector<int> verts_idx;
        std::vector<int> normals_idx;
        std::vector<int> uvs_idx;

        std::vector<shared_ptr<hittable>> tris; 
        shared_ptr<bvh_node> mesh_bvh;
        shared_ptr<material> mat_ptr; 

        int winding;

        bool doubleface = true;



    private:
        bool triangle_hit(const vec3& v0, const vec3& v1, const vec3& v2, 
                          const vec2& vt0, const vec2& vt1, const vec2& vt2,
                           const vec3& normal,const ray& r, double t_min, double t_max, hit_record& rec) const;

        void update_triangle_list(vertex_format input_format_code, int t, int t_left, int t_right){ 
            switch(input_format_code){
                                case(V_UV_NORMAL): 
                                    tris.push_back(make_shared<triangle>(verts[verts_idx[t]], verts[verts_idx[t_left]], verts[verts_idx[t_right]], 
                                    uvs[uvs_idx[t]], uvs[uvs_idx[t_left]], uvs[uvs_idx[t_right]], 
                                    normals[normals_idx[t]], normals[normals_idx[t_left]], normals[normals_idx[t_right]], 
                                    mat_ptr, doubleface));                
                                    break;
                                
                                case(V_NORMAL):
                                    // TO BE IMPLEMENTED
                                    break;

                                // case with verts, uvs
                                case(V_UV):
                                    tris.push_back(make_shared<triangle>(verts[verts_idx[t]], verts[verts_idx[t_left]], verts[verts_idx[t_right]], 
                                    uvs[uvs_idx[t]], uvs[uvs_idx[t_left]], uvs[uvs_idx[t_right]], 
                                    mat_ptr, doubleface)); 

                                    break;


                                // case with only verts
                                case(V):
                                    tris.push_back(make_shared<triangle>(verts[verts_idx[t]], verts[verts_idx[t_left]], verts[verts_idx[t_right]],
                                    mat_ptr, doubleface));  
                                    break;    
                            }
        }

        // clear entire array
        void clear_indices(vertex_format input_format_code){
            switch(input_format_code){
                                case(V_UV_NORMAL): 
                                    verts_idx.clear();
                                    uvs_idx.clear();
                                    normals_idx.clear();
                                    break;

                                case(V_NORMAL):
                                    verts_idx.clear();
                                    normals_idx.clear();
                                    break;

                                // case with verts, uvs
                                case(V_UV):
                                    verts_idx.clear();
                                    uvs_idx.clear();
                                    break;


                                // case with only verts
                                case(V):
                                    verts_idx.clear();
                                    break;    
                            }

        }
        // remove element at index t
        void sanitize_indices(vertex_format input_format_code, int t){
                switch(input_format_code){
                        case(V_UV_NORMAL): 
                            verts_idx.erase(verts_idx.begin() + t);
                            uvs_idx.erase(uvs_idx.begin() + t);
                            normals_idx.erase(normals_idx.begin() + t);
                            break;
                        case(V_NORMAL):
                            verts_idx.erase(verts_idx.begin() + t);
                            normals_idx.erase(normals_idx.begin() + t);
                            break;

                        // case with verts, uvs
                        case(V_UV):
                            verts_idx.erase(verts_idx.begin() + t);
                            uvs_idx.erase(uvs_idx.begin() + t);
                            break;


                        // case with only verts
                        case(V):
                            verts_idx.erase(verts_idx.begin() + t);
                            break;    
                    }

        }




};

// at the moment is copy of hittable_list hit func.
bool triangle_mesh::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    
    return mesh_bvh->hit(r, t_min, t_max, rec);
    // hit_record temp_rec;
    // bool hit_anything = false;
    // double closest_so_far = t_max;
    
    // for(const auto& tri : tris){
    //     if(tri->hit(r, t_min, closest_so_far, temp_rec)){
    //         hit_anything = true;
    //         closest_so_far = temp_rec.t;
    //         rec = temp_rec;
    //     }
    // }

    // return hit_anything;

}

bool triangle_mesh::triangle_hit(const vec3& v0, const vec3& v1, const vec3& v2, 
                                 const vec2& vt0, const vec2& vt1, const vec2& vt2,
                                 const vec3& normal,const ray& r, double t_min, double t_max, hit_record& rec) const{
            
            
            vec3 v02 = v2 - v0;
            vec3 v01 = v1 - v0;
            vec3 p = cross(r.direction(), v02);
            float det = dot(v01, p);

            // no hit if ray is perpendicular to triangle normal
            if(is_zero(det)){
                return false;
            // no hit if backface culling
            }else if(!doubleface && det > 0){
                return false;
            }

            float invDet = 1 / det;
            vec3 vt = r.origin() - v0;
            float u = dot(p,vt) * invDet;
            if(u < 0 || u > 1) return false;

            vec3 q = cross(vt, v01);
            float v = dot(r.direction(), q) * invDet;
            if(v < 0 || u + v > 1) return false;
            float t = dot(q, v02) * invDet;

            if(t < t_min || t > t_max) return false;
            vec2 vt_r = u * vt0 + v * vt1 + (1 - u - v) * vt2;
            rec.t = t;
            rec.u = vt_r[0];
            rec.v = vt_r[1];
            rec.p = r.at(rec.t);
            rec.set_face_normal(r, unit_vector(normal));
            rec.mat_ptr = mat_ptr;
            return true;   


        }









bool triangle_mesh::in_triangle(const vec3 &v0, const vec3 &v1, const vec3 &v2, const vec3 &vp){
    // vij refers to vector from point i to point j
    vec3 v01 = v1 - v0;
    vec3 v12 = v2 - v1;
    vec3 v20 = v0 - v2;

    vec3 v0p = vp - v0;
    vec3 v1p = vp - v1;
    vec3 v2p = vp - v2;

    // to be safe, project vp onto plane formed by the three points

    vec3 compare_to = cross(v01, v0p);



    if(dot(compare_to, cross(v20, v2p)) < 0) return false;

    if(dot(compare_to, cross(v12, v1p)) < 0) return false;



    return true;


}

bool triangle_mesh::is_ear(const vec3& v0, const vec3 &v1, const vec3 &v2, vec3 &normal){
    // v1 - v0 is side formed by vertex tested to be ear(v1) and previous indexed vertex
    // v0 - v2 is side formed by vertex tested to be ear(v2) and next indexed vertex
    
    if(dot(cross(v1 - v0, v0 - v2),normal) > 0) return true;
    return false;

}

bool triangle_mesh::bounding_box(double time0, double time1, aabb& output_box) const{
    mesh_bvh->bounding_box(time0, time1, output_box);
    return true;
}


#endif