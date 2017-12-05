/**
* @date 2017/12/04
* @author 640240H ��c�O
* @brief 2017�N�xA2�^�[���@�\�t�g�E�F�A�U ��Q��ۑ� C++�ڐA�p
* 3�����g���
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "gravity3.h"

const double G = 1.0;  // gravity constant
const double ALLOWABLE_ERROR = 0.00001;

double distance_vector(struct Vector3 const* v1, struct Vector3 const* v2) {
    return sqrt(pow(v1->x - v2->x, 2) + pow(v1->y - v2->y, 2) + pow(v1->z - v2->z, 2));
}

void mul_vector(struct Vector3* vec, double const val) {
    vec->x *= val;
    vec->y *= val;
    vec->z *= val;
}

void sub_vector(struct Vector3* v1, struct Vector3 const* v2) {
    v1->x -= v2->x;
    v1->y -= v2->y;
    v1->z -= v2->z;
}

void add_vector(struct Vector3* v1, struct Vector3 const* v2) {
    v1->x += v2->x;
    v1->y += v2->y;
    v1->z += v2->z;
}

void copy_vector(struct Vector3* des, struct Vector3 const* src) {
    des->x = src->x;
    des->y = src->y;
    des->z = src->z;
}


/**
* @fn �f�[�^�t�@�C����ǂݍ���Ő��̏����ʒu��ݒ肷��.
* @param data �f�[�^�t�@�C���@�f�[�^�̌`���͎��̒ʂ�
*               �擪�s�ɐ��̐��𔼊p�����̎��R���ln�Ŏw�肷��
*               �Â�n�s�ɂ͊e���̎���,�����ʒux,y,x,����x,y,z��7�l�����̏��ԂŔ��p�����̎����l�Ŏw�肷��
*               �Ō�̃f�[�^�s�̖��������s����
* @param p �ǂݍ��񂾒l�ŏ��������鐯�I�u�W�F�N�g
*/
int initialize_stars(FILE* data, struct Star **p) {
    int size = 0;
    *p = NULL;
    if ( fscanf_s(data, "%d\n", &size) == 1 && size > 0 ) {
        double m, x, y, z, vx, vy, vz;
        int i = 0;
        *p = ( struct Star * )malloc(sizeof(struct Star) * size);
        struct Star *stars = *p;
        while ( i < size && fscanf_s(data, "%lf,%lf,%lf,%lf,%lf,%lf,%lf\n", &m, &x, &y, &z, &vx, &vy, &vz) == 7 ) {
            stars[i].m = m;
            stars[i].r = ( struct Vector3 * )malloc(sizeof(struct Vector3));
            stars[i].v = ( struct Vector3 * )malloc(sizeof(struct Vector3));
            stars[i].pre_r = ( struct Vector3 * )malloc(sizeof(struct Vector3));
            stars[i].r->x = x;
            stars[i].r->y = y;
            stars[i].r->z = z;
            stars[i].v->x = vx;
            stars[i].v->y = vy;
            stars[i].v->z = vz;
            i++;
        }
        return i;
    }
    return 0;
}

void free_stars(int size, struct Star *stars) {
    for ( int i = 0; i < size; i++ ) {
        free(stars[i].r);
        free(stars[i].v);
        free(stars[i].pre_r);
    }
    free(stars);
}

/**
* @fn �w�肵�����̉����x���v�Z����.
* @param index �����x���v�Z����Ώۂ̐�
* @param size �S�Ă̐��̐�
* @param stars ���I�u�W�F�N�g�̔z��
* @param acceleration �v�Z�����l���������ރx�N�g���I�u�W�F�N�g
*/
void calc_acceleration(const int index, const int size, struct Vector3 *acceleration, struct Star *stars) {
    int i;
    struct Vector3 temp;
    acceleration->x = 0;
    acceleration->y = 0;
    acceleration->z = 0;
    for ( i = 0; i < size; i++ ) {
        if ( i != index ) {
            copy_vector(&temp, stars[i].r);
            sub_vector(&temp, stars[index].r);
            mul_vector(
                &temp,
                G * stars[i].m * pow(distance_vector(stars[i].r, stars[index].r), -3));
            add_vector(acceleration, &temp);
        }
    }
}

/**
* @fn �I�C���[�@��p���Ď��̎����̈ʒu�E���x���v�Z����.
* @param dt �����̕ω���
* @param size �S�Ă̐��̐�
* @param stars ���I�u�W�F�N�g�̔z��
*/
void euler(const int size, const double dt, struct Star *stars) {
    struct Vector3 temp;
    //!!Caution!! Not write new position value while calculating the acceleration of other stars
    for ( int i = 0; i < size; i++ ) {
        calc_acceleration(i, size, &temp, stars);
        // dv = a * dt
        mul_vector(&temp, dt);
        add_vector(stars[i].v, &temp);
        //write new position value to temp member
        copy_vector(stars[i].pre_r, stars[i].r);
        // dr = v * dt
        copy_vector(&temp, stars[i].v);
        mul_vector(&temp, dt);
        add_vector(stars[i].pre_r, &temp);
    }
    //swap old and new value
    for ( int i = 0; i < size; i++ ) {
        copy_vector(&temp, stars[i].r);
        copy_vector(stars[i].r, stars[i].pre_r);
        copy_vector(stars[i].pre_r, &temp);
    }
}


/**
* @fn �����Q�E�N�b�^�@��p���Ď��̎����̈ʒu�E���x���v�Z����.
* @param dt �����̕ω���
* @param size �S�Ă̐��̐�
* @param stars ���I�u�W�F�N�g�̔z��
*/
void runge_kutta(const int size, const double dt, struct Star *stars) {
    /*
    t:time
    r:position of star (vector)
    v:velosity of star (vector)
    target equation : 1/dt(dr/dt) = f(t,r,v)
    f(t,r,v) = sum (G * m' * (r'-r) * |r'-r|^-3 )  not depending on v or t
    r', m' is mass and position of other stars
    => dv/dt = f(r) AND dr/dt = v
    calc following values;
    v1 = dt * f(r)           r1 = dt * v
    v2 = dt * f(r+r1/2)      r2 = dt * (v+v1/2)
    v3 = dt * f(r+r2/2)      r3 = dt * (v+v2/2)
    v4 = dt * f(r+r3  )      r4 = dt * (v+v3  )
    r(next) = r + (r1+2*r2+2*r3+r4)/6
    v(next) = v + (v1+2*v2+2*v3+v4)/6
    */

    int i;
    struct Vector3 **r = ( struct Vector3 ** )malloc(sizeof(struct Vector3) * size);
    struct Vector3 **v = ( struct Vector3 ** )malloc(sizeof(struct Vector3) * size);
    for ( i = 0; i < size; i++ ) {
        //store previous position
        copy_vector(stars[i].pre_r, stars[i].r);
        r[i] = ( struct Vector3 * )malloc(sizeof(struct Vector3) * 4);
        v[i] = ( struct Vector3 * )malloc(sizeof(struct Vector3) * 4);
    }

    for ( i = 0; i < size; i++ ) {
        //v1 = dt * f(r)
        calc_acceleration(i, size, &v[i][0], stars);
        mul_vector(&v[i][0], dt);
        //r1 = dt * v
        copy_vector(&r[i][0], stars[i].v);
        mul_vector(&r[i][0], dt);
    }
    for ( i = 0; i < size; i++ ) {
        // set r+r1/2
        copy_vector(stars[i].r, &r[i][0]);
        mul_vector(stars[i].r, 0.5);
        add_vector(stars[i].r, stars[i].pre_r);
    }
    for ( i = 0; i < size; i++ ) {
        //v2 = dt * f(r+r1/2)
        calc_acceleration(i, size, &v[i][1], stars);
        mul_vector(&v[i][1], dt);
        //r2 = dt * (v+v1/2)
        copy_vector(&r[i][1], &v[i][0]);
        mul_vector(&r[i][1], 0.5);
        add_vector(&r[i][1], stars[i].v);
        mul_vector(&r[i][1], dt);
    }
    for ( i = 0; i < size; i++ ) {
        // set r+r2/2
        copy_vector(stars[i].r, &r[i][1]);
        mul_vector(stars[i].r, 0.5);
        add_vector(stars[i].r, stars[i].pre_r);
    }
    for ( i = 0; i < size; i++ ) {
        //v3 = dt * f(r+r2/2)
        calc_acceleration(i, size, &v[i][2], stars);
        mul_vector(&v[i][2], dt);
        //r3 = dt * (v+v2/2)
        copy_vector(&r[i][2], &v[i][1]);
        mul_vector(&r[i][2], 0.5);
        add_vector(&r[i][2], stars[i].v);
        mul_vector(&r[i][2], dt);
    }
    for ( i = 0; i < size; i++ ) {
        // set r+r3
        copy_vector(stars[i].r, &r[i][2]);
        add_vector(stars[i].r, stars[i].pre_r);
    }
    for ( i = 0; i < size; i++ ) {
        //v4 = dt * f(r+r3)
        calc_acceleration(i, size, &v[i][3], stars);
        mul_vector(&v[i][3], dt);
        //r4 = dt * (v+v3)
        copy_vector(&r[i][3], &v[i][2]);
        add_vector(&r[i][3], stars[i].v);
        mul_vector(&r[i][3], dt);
    }
    for ( i = 0; i < size; i++ ) {
        copy_vector(stars[i].r, stars[i].pre_r);
        //r(next) = r + (r1+2*r2+2*r3+r4)/6
        mul_vector(&r[i][0], 1.0 / 6.0);
        mul_vector(&r[i][1], 2.0 / 6.0);
        mul_vector(&r[i][2], 2.0 / 6.0);
        mul_vector(&r[i][3], 1.0 / 6.0);
        add_vector(stars[i].r, &r[i][0]);
        add_vector(stars[i].r, &r[i][1]);
        add_vector(stars[i].r, &r[i][2]);
        add_vector(stars[i].r, &r[i][3]);
        //v(next) = v + (v1+2*v2+2*v3+v4)/6
        mul_vector(&v[i][0], 1.0 / 6.0);
        mul_vector(&v[i][1], 2.0 / 6.0);
        mul_vector(&v[i][2], 2.0 / 6.0);
        mul_vector(&v[i][3], 1.0 / 6.0);
        add_vector(stars[i].v, &v[i][0]);
        add_vector(stars[i].v, &v[i][1]);
        add_vector(stars[i].v, &v[i][2]);
        add_vector(stars[i].v, &v[i][3]);
    }

    //free memory
    for ( i = 0; i < size; i++ ) {
        free(r[i]);
        free(v[i]);
    }
    free(r);
    free(v);
}

int is_collision(struct Star *a, struct Star *b, double dt) {
    //(�����Ԃ̑��Α��x�̑Ζʕ�������) * dt < (�����Ԃ̋���)
    double d = distance_vector(a->r, b->r);
    struct Vector3 v;   //���Α��x
    copy_vector(&v, b->v);
    sub_vector(&v, a->v);
    double s = ( ( b->r->x - a->r->x )*v.x + ( b->r->y - a->r->y )*v.y + ( b->r->z - a->r->z )*v.z ) / d; //�Ζʕ����̐���
    return d < s * dt;
}

int collision(int const size, const double dt, struct Star *stars) {
    struct Vector3 temp1, temp2;
    for ( int i = 0; i < size - 1; i++ ) {
        for ( int j = i + 1; j < size; j++ ) {
            copy_vector(&temp1, stars[i].v);
            mul_vector(&temp1, dt);
            add_vector(&temp1, stars[i].r);
            copy_vector(&temp2, stars[j].v);
            mul_vector(&temp2, dt);
            add_vector(&temp2, stars[j].r);
            if ( is_collision(&stars[i], &stars[j], dt) ) {
                //�Փ˂͌��݂̈ʒu����̑��x�x�N�g���̌����Ŕ���
                //�Փˌ�̐���stars[i]//��_��V�������W�ɐݒ�
                add_vector(stars[i].r, stars[j].r);
                mul_vector(stars[i].r, 0.5);
                //�^���ʕۑ�
                mul_vector(stars[i].v, stars[i].m);
                mul_vector(stars[j].v, stars[j].m);
                add_vector(stars[i].v, stars[j].v);
                stars[i].m += stars[j].m;
                mul_vector(stars[i].v, 1.0 / stars[i].m);
                for ( int k = j + 1; k < size; k++ ) {
                    stars[k - 1].m = stars[k].m;
                    copy_vector(stars[k - 1].r, stars[k].r);
                    copy_vector(stars[k - 1].v, stars[k].v);
                }
                return size - 1;
            }
        }
    }
    return size;
}

