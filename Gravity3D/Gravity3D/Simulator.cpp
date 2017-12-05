#include "Simulator.h"
#include "gravity3.h"
#include "DxLib.h"
#include <math.h>


Simulator::Simulator(int argc, char **argv, int w, int h, int d) {

    this->w = w;
    this->h = h;
    this->d = d;
    cnt = 0;

    if ( argc > 1 ) {
        FILE* data;
        fopen_s(&data, argv[1], "r");
        if ( data == NULL ) {
            fprintf(stderr, "error: cannot open %s.\n", argv[1]);
        } else {
            size = initialize_stars(data, &stars);
            fclose(data);
            original_size = size;
        }
    } else {
        fprintf(stderr, "data file not specified.\n");
    }
}

bool Simulator::Update() {
    if ( IsAnyStarOnScreen() ) {
        cnt++;
        //detect collision
        size = collision(size, dt, stars);
        //update
        //euler(size,dt,stars);
        runge_kutta(size, dt, stars);
        //draw
        OnDraw();
        return true;
    } else {
        return false;
    }

}

void Simulator::OnDraw() {
    const int color = GetColor(0xff, 0xff, 0xff);
    DrawFormatString(3, 3, color, "time : %5.1f", cnt * dt);
    for ( int i = 0; i < size; i++ ) {
        struct Star *star = &stars[i];
        const float r = (float)( pow(star->m, 1.0 / 3.0) * 10 );
        struct Vector3 *position = star->r;
        const float x = (float)( position->x * unit);
        const float y = (float)( position->y * unit);
        const float z = (float)( position->z * unit);
        //DrawCircleAA(x, y, r, 32, color, true);
        DrawSphere3D(VGet(x, y, z), r, 32, color, color, true);
        DrawFormatString(0, 30 * ( i + 1 ), color, "%2d > m:%4.1f r:(%5.1f,%5.1f)", i, star->m, position->x, position->y);
    }
}

Simulator::~Simulator() {
    free_stars(original_size, stars);
}

bool Simulator::IsAnyStarOnScreen() {
    const double wmax = w / unit / 2 + 2;
    const double hmax = h / unit / 2 + 2;
    const double dmax = d / unit / 2 + 2;
    for ( int i = 0; i < size; i++ ) {
        struct Vector3 *position = stars[i].r;
        if ( fabs(position->x) <= wmax && fabs(position->y) <= hmax && fabs(position->z) <= dmax ) {
            return true;
        }
    }
    return false;
}