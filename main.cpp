#include <iostream>

#include "rtweekend.h"
#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "vec3.h"
#include "material.h"
#include "checkerboard.h"
#include "moving_sphere.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "torus.h"
#include "triangle.h"
#include "triangle_mesh.h"



color ray_color(const ray& r, const color& background , const hittable& world, int depth){ // would be interesting to simulate attenuative reflection
    hit_record rec;
    
    if(depth <= 0){
        return color(0,0,0);
    }

    if(!world.hit(r,0.001, infinity, rec))
        return background;
    
    ray scattered; 
    color attenuation;
    color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);


    if(!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted;
  

   
    return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
}

hittable_list two_spheres(){
    hittable_list objects;
    auto checker = make_shared<checker_texture>(color(0.2,0.3,0.1), color(0.9,0.9,0.9));
    objects.add(make_shared<sphere>(point3(0,-10,0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(point3(0, 10,0), 10, make_shared<lambertian>(checker)));

    return objects;

}

hittable_list two_perlin_spheres(){
    hittable_list objects;
    auto anti_pertex = make_shared<noise_texture>(12.0,true);
    auto pertex = make_shared<noise_texture>(12.0);

    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(anti_pertex)));
    objects.add(make_shared<sphere>(point3(0, 2,0), 2, make_shared<lambertian>(pertex)));

    return objects;

}

hittable_list marble_board(){
    hittable_list objects;
    auto anti_pertex = make_shared<noise_texture>(12.0,true);
    auto pertex = make_shared<noise_texture>(12.0);

    auto cboard = make_shared<checkerboard>(point3(-32,-2,0), point3(0, 1, 0.8), 0.5, 128, 128, vec3(0,0,-1), make_shared<lambertian>(pertex), make_shared<lambertian>(anti_pertex));
    
    objects.add(cboard);

    return objects;


}

hittable_list textured_triangle_mesh_cornell_box(){
    hittable_list objects;

    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0,555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    // /* Textured Triangle */
    auto mesh_tex = make_shared<barycentric_intrp>(color(1,0,0), color(0,1,0), color(0,0,1));
    auto mesh_mat = make_shared<lambertian>(mesh_tex);
    shared_ptr<hittable> cube_mesh = make_shared<triangle_mesh>("cube.obj", mesh_mat,-1); 
    cube_mesh = make_shared<scale>(cube_mesh, 100);
    cube_mesh = make_shared<rotate_y>(cube_mesh, 35);
    cube_mesh = make_shared<translate>(cube_mesh, vec3(265, 20, 390));
    objects.add(cube_mesh);
    


    return hittable_list(objects);
}

hittable_list textured_triangle_cornell_box(){
    /* Cornell Box */
    
    hittable_list objects;

    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0,555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    // /* Textured Triangle */
    auto triangle_color = make_shared<barycentric_intrp>(color(1,0,0), color(0,1,0), color(0,0,1));
    auto triangle_mat = make_shared<lambertian>(triangle_color);
    shared_ptr<hittable> equilateral = make_shared<triangle>(point3(0, 0, 0), point3(50, 100 * sqrt(3) / 2.0, 0), point3(100, 0, 0), triangle_mat, true);
    equilateral = make_shared<scale>(equilateral, 2);
    equilateral = make_shared<rotate_y>(equilateral, 15);
    equilateral = make_shared<translate>(equilateral, vec3(265, 20, 390));
    objects.add(equilateral);
    


    return hittable_list(objects);


}

hittable_list moo_moo_sphere(){
    hittable_list objects;
    auto mm = make_shared<image_texture>("moomoo.jpg");
    auto mm_surface = make_shared<lambertian>(mm);
    auto mm_ball = make_shared<sphere>(point3(0,2,0), 2, mm_surface);

    
    //auto cboard = make_shared<checkerboard>(point3(-32,-2,0), point3(0, 1, 0.8), 0.5, 128, 128, vec3(0,0,-1), material_marble, material_black);
    //objects.add(cboard);
    auto pertex = make_shared<noise_texture>(12.0);
    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertex)));
    objects.add(mm_ball);

    return hittable_list(objects);
}

hittable_list simple_light(){
    hittable_list objects;

    auto pertex = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertex)));
    objects.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertex)));
    
    auto difflight = make_shared<diffuse_light>(color(4,4,4));
    objects.add(make_shared<xy_rect>(3,5,1,3,-2, difflight));

    return objects;
}

hittable_list cornell_box(){
    hittable_list objects;

    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0,555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    objects.add(box1);

    shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));
    objects.add(box2);

    return objects;


}
hittable_list cornell_smoke() {
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));

    shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));

    objects.add(make_shared<constant_medium>(box1, 0.01, color(0,0,0)));
    objects.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

    return objects;
}

hittable_list doughnuts_and_smoke(){
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    auto sunyellow = make_shared<lambertian>(color(0.95,0.82,0.25));


    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));

    shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));

    shared_ptr<hittable> doughnut = make_shared<torus>(point3(0,0,0),90,30, sunyellow);
    doughnut = make_shared<translate>(doughnut, vec3(180,230,280));
    doughnut = make_shared<rotate_y>(doughnut, - 12);
    objects.add(doughnut);
    // objects.add(make_shared<constant_medium>( doughnut,0.1,color(0.95,0.82,0.25)));
    // objects.add(make_shared<constant_medium>(box1, 0.01, color(0,0,0)));
    // objects.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

    return objects;
}

hittable_list smoke_box_and_torus(){
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    auto sunyellow = make_shared<lambertian>(color(0.95,0.82,0.25));


    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));

    shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));

    shared_ptr<hittable> doughnut = make_shared<torus>(point3(0,0,0),90,30, sunyellow);
    doughnut = make_shared<translate>(doughnut, vec3(180,230,280));
    doughnut = make_shared<rotate_y>(doughnut, - 25);
    objects.add(make_shared<constant_medium>( doughnut,0.007,color(0.95,0.82,0.25)));
    objects.add(make_shared<constant_medium>(box1, 0.01, color(0,0,0)));
    // objects.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

    return objects;

}

hittable_list random_scene(){
    hittable_list world;
    
    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(checker)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = random() * random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, random_double(0,.5), 0);
                    world.add(make_shared<moving_sphere>(
                        center, center2, 0.0, 1.0, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;

}




int main(){

    // Image
    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 600; // 3840
    int image_height = static_cast<int>(image_width / aspect_ratio);
    int samples_per_pixel = 100;
    int sqrt_ssp = (int)sqrt(samples_per_pixel);
    int max_depth = 50;

    //world
    hittable_list world;
    point3 lookfrom;
    point3 lookat;
    auto vfov = 40.0;
    auto aperture = 0.0;
    color background(0,0,0);

    switch(12){
        case 1:
            world = random_scene();
            background = color(0.7,0.8,1);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
            aperture = 0.1;
            break;

        case 2:
            world = two_spheres();
            background = color(0.7,0.8,1);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
            break; 
        
        
        case 3:
            world = moo_moo_sphere();
            background = color(0.7,0.8,1);
            lookfrom = point3(5,1,-4);
            lookat = point3(0,0,0);
            vfov = 40.0;
        case 4:           
            world = two_perlin_spheres();
            background = color(0.7,0.8,1);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
        case 5:
            world = marble_board();
            background = color(0.7,0.8,1);
            lookfrom = point3(0, 0.7, 0);
            lookat = point3(0,0.75,-1);
            vfov = 20.0;
        case 6:
            world = simple_light();
            background = color(0,0,0);
            lookfrom = point3(26,3,6);
            lookat = point3(0,2,0);
            vfov = 20.0;
            break;

        case 7:
            world = cornell_box();
            aspect_ratio = 1.0;
            image_width = 600;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 5000;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            background = color(0,0,0);
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            max_depth = 150;
            break;

        case 8:
            world = cornell_smoke();
            aspect_ratio = 1.0;
            image_width = 1920;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 500;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            lookfrom = point3(278,278,-800);
            lookat = point3(278,278,0);
            vfov = 40.0;
            break;
            
        case 9:
            world = doughnuts_and_smoke();
            aspect_ratio = 1.0;
            image_width = 600;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 50;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            lookfrom = point3(278,278,-800);
            lookat = point3(278,278,0);
            vfov = 40.0;
            break;

        case 10:
            world = smoke_box_and_torus();
            aspect_ratio = 1.0;
            image_width = 600;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 100;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            lookfrom = point3(278,278,-800);
            lookat = point3(278,278,0);
            vfov = 40.0;
            break;

        case 11:
            world = textured_triangle_cornell_box();
            aspect_ratio = 1.0;
            image_width = 600;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 100;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            lookfrom = point3(278,278,-800);
            lookat = point3(278,278,0);
            vfov = 40.0;
            break;
        
        default:
        case 12:
            world = textured_triangle_mesh_cornell_box();
            aspect_ratio = 1.0;
            image_width = 600;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 100;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            lookfrom = point3(278,278,-800);
            lookat = point3(278,278,0);
            vfov = 40.0;
            break;



        
            

    }

    
    auto dist_to_focus = 1.0;
    vec3 vup(0,1,0);

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);


    std::cout<<"P3\n"<<image_width<<' '<<image_height<<'\n'<<255<<'\n';
    for(int j = image_height - 1; j>=0; j--){
        std::cerr << "\rScanlines remaining: "<< j << ' '<<std::flush;
        for(int i = 0; i < image_width; i++){
            
            color pixel_color(0,0,0);
            // for (int s = 0; s < samples_per_pixel; ++s) {
            //     auto u = (i + random_double()) / (image_width-1);
            //     auto v = (j + random_double()) / (image_height-1);
            //     ray r = cam.get_ray(u, v);
            //     pixel_color += ray_color(r, world, max_depth);
            // }
            // snippet distributing samples_per_pixel rays EVENLY over a pixel
            auto u = double(i) / (image_width - 1);
            auto v = double(j) / (image_height - 1);
            for(int shift_u = 0; shift_u < sqrt_ssp; shift_u ++){
                u += 1.0/(sqrt_ssp * (image_width - 1));
                for(int shift_v = 0; shift_v < sqrt_ssp; shift_v ++){
                    v += 1.0/(sqrt_ssp * (image_height - 1));
                
                    ray r = cam.get_ray(u,v);
                    pixel_color += ray_color(r, background, world, max_depth);
                }
                v = double(j) / (image_height - 1);
            }
            write_color(std::cout, pixel_color, samples_per_pixel); 

            

        }
    }


}

