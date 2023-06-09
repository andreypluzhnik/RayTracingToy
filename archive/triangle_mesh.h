

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

#include "ray.h"
#include "triangle.h"
#include "material.h"
#include "rtweekend.h"



class triangle_mesh : public hittable{

    public:
        triangle_mesh(){}
        triangle_mesh(const char* filename, shared_ptr<material> m, int w) : mat_ptr(m), winding(w){
            
            std::ifstream file;
            std::vector<int> verts_idx;
            std::vector<int> normals_idx;
            std::vector<int> uvs_idx;
        
            try{    
                file.open(filename);
                if (file.fail()) throw;
                std::string line = "";   
                std::istringstream iss;
                std::string buffer;
                float param;

                while(getline(file, line)){
                    iss.clear();
                    iss.str(line);
                    iss >> buffer;
                    
                    if(buffer.compare("vt") == 0){
                        vec2 uv;
                        iss >> uv[0] >> uv[1];

                    }else if(buffer.compare("vn") == 0){
                        vec3 normal;
                        iss >> normal[0] >> normal[1] >> normal[2];
                        normals.push_back(normal);
                        

                    }else if(buffer.compare("v") == 0){
                        vec3 vert;
                        iss >> vert[0] >> vert[1] >> vert[2];
                        verts.push_back(vert);
                    }else if(buffer.compare("f") == 0){
                        // tokenize the strings based on delimeter
                        // extract indices from each token
                        // start_pointer = verts_idx.size();

                        while(!iss.eof()){
                            iss >> buffer;
                            verts_idx.push_back(0);
                            normals_idx.push_back(0);
                            uvs_idx.push_back(0);
                            
                            sscanf(buffer.c_str(), "%i%*c%i%*c%i%*c", &verts_idx.back(), &uvs_idx.back(), &normals_idx.back());
                            
                            // convert line indices to array indices
                            verts_idx.back() = verts_idx.back() - 1;
                            uvs_idx.back() = uvs_idx.back() - 1;
                            normals_idx.back() = normals_idx.back() - 1;

                        }
                        // end_pointer = verts_idx.size() - 1;



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
                                    triangles_verts_idx.push_back(verts_idx[t]);   
                                    triangles_verts_idx.push_back(verts_idx[t_right]);
                                    triangles_verts_idx.push_back(verts_idx[t_left]);
                                    
                                    triangles_uvs_idx.push_back(uvs_idx[t]);
                                    triangles_uvs_idx.push_back(uvs_idx[t_right]);
                                    triangles_uvs_idx.push_back(uvs_idx[t_left]);

                                    triangles_normals_idx.push_back(normals_idx[t]);
                                    triangles_normals_idx.push_back(normals_idx[t_right]);
                                    triangles_normals_idx.push_back(normals_idx[t_left]);
                                    
                                    // sanitize indices 
                                    verts_idx.erase(verts_idx.begin() + t);
                                    uvs_idx.erase(uvs_idx.begin() + t);
                                    normals_idx.erase(normals_idx.begin() + t);    

                                    t = 0;

                                }else{
                                    t = (t + 1) % (int)verts_idx.size();    
                                }

                            }else{
                                t = (t + 1 * winding) % (int)verts_idx.size();    
                            }

                            t_left = ( (t - 1 * winding) % (int)verts_idx.size() + (int)verts_idx.size() ) % (int)verts_idx.size();
                            t_right = ( (t + 1 * winding) % (int)verts_idx.size() + (int)verts_idx.size() ) % (int)verts_idx.size();
                        }
                        // 
                        std::reverse(verts_idx.begin(),verts_idx.end());
                        for(int i = 0; i < 3; i++){
                            triangles_verts_idx.push_back(verts_idx.back());
                            verts_idx.pop_back();
                            
                            triangles_uvs_idx.push_back(uvs_idx.back());
                            uvs_idx.pop_back();

                            triangles_normals_idx.push_back(normals_idx.back());
                            normals_idx.pop_back();
                        }
                    

                    }


                }

            }catch(...){
                std::cerr<<"There was a problem with reading the mesh file.";
                file.close();
            }


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
        std::vector<vec3> uvs;

        std::vector<int> triangles_verts_idx;
        std::vector<int> triangles_uvs_idx;
        std::vector<int> triangles_normals_idx;

        shared_ptr<material> mat_ptr; 

        const char delimit = ' ';
        int winding;

        bool doubleface = true;




    private:
        bool triangle_hit(const vec3& v0, const vec3& v1, const vec3& v2, 
                          const vec2& vt0, const vec2& vt1, const vec2& vt2,
                           const vec3& normal,const ray& r, double t_min, double t_max, hit_record& rec) const;

        vec2 vtt0 = vec2(1.0,0);
        vec2 vtt1 = vec2(0,1.0);
        vec2 vtt2 = vec2(0,0);


};

// at the moment is copy of hittable_list hit func.
bool triangle_mesh::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    bool hit_anything = false;
    double closest_so_far = t_max;
    for(int i = 0; i < triangles_verts_idx.size()/3; i++){
        if(triangle_hit(verts[triangles_verts_idx[3 * i]], verts[triangles_verts_idx[3 * i + 1]], verts[triangles_verts_idx[3 * i + 2]], vtt0, vtt1, vtt2, normals[triangles_normals_idx[3 * i]],r, t_min, closest_so_far, rec)){
            hit_anything = true;
            closest_so_far = rec.t;
        }
    }
    return hit_anything;

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

// to be implemented
bool triangle_mesh::bounding_box(double time0, double time1, aabb& output_box) const{
    return true;
}


#endif