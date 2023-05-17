#ifndef RT_WEEKEND
#define RT_WEEKEND

#include <iostream>
#include <cmath>
#include <memory>
#include <limits>
#include <cstdlib>


using std::shared_ptr;
using std::make_shared;
using std::sqrt;

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932384626433832795028841971;
const double EQN_EPS = 1e-12;

enum winding {clockwise = 1, c_clockwise = -1};


inline double degrees_to_radians(double degrees){
    return degrees * pi / 180;
}

inline double random_double(){
    // return random number in [0,1)
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max){
    // return random number in [min,max)
    return min + (max - min) * random_double();
}

inline double clamp(double x, double min, double max){
    if(x < min) return min;
    if(x > max) return max;
    return x;
}

inline int random_int(int min, int max) {
    // Returns a random integer in [min,max].
    return static_cast<int>(random_double(min, max+1));
}

// inline int random_int(int min, int max){
//     return min + rand() % (max - min + 1);
// }

// #include "ray.h"
// #include "vec3.h"

/*
Below adapted from Graphical Gems https://github.com/erich666/GraphicsGems/blob/master/gems/Roots3And4.c
*/
inline bool is_zero(double val){
    return val > - EQN_EPS && val < EQN_EPS;
}

inline int solve_quadratic(const double c[3], double* s){
    // convert to form x^2+px+q = 0
    
    double p,q,D;
    // std::cout<<"Coefficients are "<<s[0]<<" "<<s[1]<<" "<<s[2]<<std::endl; 
    p = c[1]/(2 * c[2]);
    q = c[0]/c[2];

    D = p * p - q;

    if(is_zero(D)){
        s[0] = -p;
        return 1;
    }else if(D < 0){
        return 0;
    }else{
        double sqrt_D = sqrt(D);

        s[0] = -p + sqrt_D;
        s[1] = - p - sqrt_D;
        return 2; 
    }

    // if(D < 0){
    //     return 0;
    // }

  


}

inline int solve_cubic(const double c[4], double* s){
    int i, num;
    double sub;
    double A,B,C;
    double sq_A, p, q;
    double cb_p, D;
    
    // convert to form x^3 + Ax^2 + Bx + C
    
    A = c[2]/c[3];
    B = c[1]/c[3];
    C = c[0]/c[3];

    /* sub y = x - A/3 to remove quadratic */
    sq_A = A * A;
    p = 1.0/3 * (-1.0/3 * sq_A + B);
    q = 1.0/2 * (2.0/27 * A * sq_A - 1.0/3 * A * B + C);

    cb_p = p * p * p;
    D = q * q + cb_p;

    if(is_zero(D)){
        if(is_zero(q)){ //triple all real, root is 0
            s[0] = 0;
            num = 1;
        } 
        else  // one single and one double solution
        {
            double u = cbrt(-q);
            s[0] = 2 * u;
            s[1] = -1 * u;
            num = 2;
        }
    }
    else if (D < 0){ // three real solutions
        double phi = 1.0/3 * acos(-q/sqrt(-cb_p));
        double t = 2 * sqrt(-p);

        s[0] = t * cos(phi);
        s[1] = -t * cos(phi + pi / 3);
        s[2] = -t * cos(phi - pi / 3);
        num = 3;
    } else { // one real solution
        double sqrt_D = sqrt(D);
        double u = cbrt(sqrt_D - q);
        double v = - cbrt(sqrt_D + q);

        s[0] = u + v;
        num = 1;
    }
    
    sub = 1.0/3 * A;
    for(i = 0; i < num; ++i){
        s[i] -= sub;
    }

    return num;
}

inline double solve_quartic(const double c[5], double* s){
    double coeffs[5];
    double z, u, v, sub;
    double A, B, C, D;
    double sq_A, p, q, r;
    int i, num;

    // convert to form x^4 + Ax^3 + Bx^2 + Cx + D
    A = c[3]/c[4];
    B = c[2]/c[4];
    C = c[1]/c[4];
    D = c[0]/c[4];


    /*  substitute x = y - A/4 to eliminate cubic term:
	x^4 + px^2 + qx + r = 0 */
 
    sq_A = A * A;
    p = - 3.0/8 * sq_A + B;
    q = 1.0/8 * sq_A * A - 1.0/2 * A * B + C;
    r = -3.0/256 * sq_A * sq_A + 1.0/16*sq_A*B - 1.0/4*A*C + D;
    


    if(is_zero(r)){

        // no absolute term y(y^3 + py + q) = 0
        coeffs[0] = q; 
        coeffs[1] = p;
        coeffs[2] = 0;
        coeffs[3] = 1;

        num = solve_cubic(coeffs,s);

        s[num++] = 0;
    }else{

        // this resource explains what is going on in the code: https://web.williams.edu/Mathematics/sjmiller/public_html/209/handouts/Cherowitzo_SolvingPolyEqsIII.pdf
        /* solve resolvant cubic */
        coeffs[0] = 1.0/2 * r * p - 1.0/8 * q * q;
        coeffs[1] = -r;
        coeffs[2] = -1.0/2 * p;
        coeffs[3] = 1;

        solve_cubic(coeffs, s);
        /* take guaranteed real solution */
        z = s[0];

        /* build and solve two quadratics*/
        u = z * z - r;
        v = 2 * z - p;

        if(is_zero(u)){
            u = 0;
        }else if( u > 0){
            u = sqrt(u);
        }else{
            return 0;
        }

        if(is_zero(v)){
            v = 0;
        }else if(v > 0){
            v = sqrt(v);
        }else{
            return 0;
        }

        coeffs[0] = z - u;
        coeffs[1] = q < 0 ? -v : v;
        coeffs[2] = 1;
        num = solve_quadratic(coeffs,s);
        // std::cout<<"First quadratic yields "<<num<<" roots."<<std::endl;

        coeffs[0] = z + u;
        coeffs[1] = q < 0 ? -v : v;
        coeffs[2] = 1;

        double* truncated_s = s + 2;
        num += solve_quadratic(coeffs,  s + 2);
        // std::cout<<"Second quadratic gives "<< num <<" roots in total."<<std::endl;


    }
    
    // resubstitute

    sub = 1.0/4 * A;
    
    for(i = 0; i < num; i++){
        s[i] -= sub;
    }

    return num;


}


#endif