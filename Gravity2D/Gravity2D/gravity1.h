#pragma once
#include <stdio.h>

struct Star {
    double m;           // mass
    struct Vector2* r;  // position
    struct Vector2* pre_r;// position at previous step
    struct Vector2* v;  // velocity
};

struct Vector2 {
    double x;
    double y;
    };

#ifdef __cplusplus 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 
extern "C" {
#endif 　 

    
    double distance_vector(struct Vector2* v1, struct Vector2* v2);
    void mul_vector(struct Vector2* vec, double const val);
    void sub_vector(struct Vector2* v1, struct Vector2 const* v2);
    void add_vector(struct Vector2* v1, struct Vector2 const* v2);
    void copy_vector(struct Vector2* des, struct Vector2 const* src);
    int initialize_stars(FILE* data, struct Star **p);
    void free_stars(const int size, struct Star *stars);
    void euler(const int size, const double dt, struct Star *stars);
    void runge_kutta(const int size, const double dt, struct Star *stars);
    int collision(const int size, const double dt, struct Star *stars);

#ifdef __cplusplus 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 
}
#endif 　 