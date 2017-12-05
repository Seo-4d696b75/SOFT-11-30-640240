/**
* @date 2017/12/02
* @author 640240H 仙田薫
* @brief 2017年度A2ターム　ソフトウェア�U 第２回課題 C++移植用
* 2次元拡大版
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "gravity1.h"

const double G = 1.0;  // gravity constant
const double ALLOWABLE_ERROR = 0.00001;

double distance_vector(struct Vector2 const* v1, struct Vector2 const* v2) {
	return sqrt(pow(v1->x - v2->x, 2) + pow(v1->y - v2->y, 2));
}

void mul_vector(struct Vector2* vec, double const val) {
	vec->x *= val;
	vec->y *= val;
}

void sub_vector(struct Vector2* v1, struct Vector2 const* v2) {
	v1->x -= v2->x;
	v1->y -= v2->y;
}

void add_vector(struct Vector2* v1, struct Vector2 const* v2) {
	v1->x += v2->x;
	v1->y += v2->y;
}

void copy_vector(struct Vector2* des, struct Vector2 const* src) {
	des->x = src->x;
	des->y = src->y;
}


/**
* @fn データファイルを読み込んで星の初期位置を設定する.
* @param data データファイル　データの形式は次の通り
*               先頭行に星の数を半角数字の自然数値nで指定する
*               つづくn行には各星の質量,初期位置x,y,初速x,yの5値をこの順番で半角数字の実数値で指定する
*               最後のデータ行の末尾も改行する
* @param p 読み込んだ値で初期化する星オブジェクト
*/
int initialize_stars(FILE* data, struct Star **p) {
	int size = 0;
	*p = NULL;
	if ( fscanf_s(data, "%d\n", &size) == 1 && size > 0 ) {
		double m, x, y, vx, vy;
		int i = 0;
		*p = (struct Star *)malloc(sizeof(struct Star) * size);
		struct Star *stars = *p;
		while ( i < size && fscanf_s(data, "%lf,%lf,%lf,%lf,%lf\n", &m, &x, &y, &vx, &vy) == 5 ) {
			stars[i].m = m;
			stars[i].r = (struct Vector2 *)malloc(sizeof(struct Vector2));
			stars[i].v = ( struct Vector2 *)malloc(sizeof(struct Vector2));
            stars[i].pre_r = ( struct Vector2 * )malloc(sizeof(struct Vector2));
			stars[i].r->x = x;
			stars[i].r->y = y;
			stars[i].v->x = vx;
			stars[i].v->y = vy;
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
* @fn 指定した星の加速度を計算する.
* @param index 加速度を計算する対象の星
* @param size 全ての星の数
* @param stars 星オブジェクトの配列
* @param acceleration 計算した値を書き込むベクトルオブジェクト
*/
void calc_acceleration(const int index, const int size, struct Vector2 *acceleration, struct Star *stars) {
    int i;
    struct Vector2 temp;
    acceleration->x = 0;
    acceleration->y = 0;
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
* @fn オイラー法を用いて次の時刻の位置・速度を計算する.
* @param dt 時刻の変化量
* @param size 全ての星の数
* @param stars 星オブジェクトの配列
*/
void euler(const int size, const double dt, struct Star *stars) {
    struct Vector2 temp;
    //!!Caution!! Not write new position value while calculating the acceleration of other stars
    for ( int i = 0; i < size; i++ ) {
        calc_acceleration(i,size,&temp, stars);
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
* @fn ルンゲ・クッタ法を用いて次の時刻の位置・速度を計算する.
* @param dt 時刻の変化量
* @param size 全ての星の数
* @param stars 星オブジェクトの配列
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
    struct Vector2 **r = ( struct Vector2 **)malloc(sizeof(struct Vector2) * size);
    struct Vector2 **v = ( struct Vector2 **)malloc(sizeof(struct Vector2) * size);
    for ( i = 0; i < size; i++ ) {
        //store previous position
        copy_vector(stars[i].pre_r, stars[i].r);
        r[i] = ( struct Vector2 * )malloc(sizeof(struct Vector2)*4);
        v[i] = ( struct Vector2 * )malloc(sizeof(struct Vector2)*4);
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

/**
* @fn 二つの線分が交差するか判定する.
* @param a,b 1つ目の線分の両端点
* @param c,d 2つ目の線分の両端点
*/
int is_cross(struct Vector2 const *a, struct Vector2 const *b, struct Vector2 const *c, struct Vector2 const *d) {
    double ta = ( c->x - d->x )*( a->y - c->y ) + ( c->y - d->y )*( c->x - a->x );
    double tb = ( c->x - d->x )*( b->y - c->y ) + ( c->y - d->y )*( c->x - b->x );
    double tc = ( a->x - b->x )*( c->y - a->y ) + ( a->y - b->y )*( a->x - c->x );
    double td = ( a->x - b->x )*( d->y - a->y ) + ( a->y - b->y )*( a->x - d->x );
    return tc * td < 0 && ta * tb < 0;
}

/**
* @fn 線分上に点が存在するか判定する. 線分両端上も線分上とする
* @param a,b 線分の両端点
* @param p 判定対象の点
*/
int is_on_segment(struct Vector2 const *a, struct Vector2 const *b, struct Vector2 const *p) {
    double outer = ( a->x - p->x ) * ( b->y - p->y ) - ( a->y - p->y ) * ( b->x - p->x );
    double inner = ( a->x - p->x ) * ( b->x - p->x ) + ( a->y - p->y ) * ( b->y - p->y );
    return fabs(outer) < ALLOWABLE_ERROR && inner <= 0;
}

/**
* @fn 二つの線分が交差するか判定する.
* @param a,b 線分AB
* @param c,d 線分CD
* @return 交差するとき1 交差しないとき0
* @detail
* 線分の交差を次のように定義する
* (i)  二つの線分が交差するとき
* (ii) 二つの線分が同一直線上に存在してAND線分が共通部分を持つとき
*/
int get_intersection(struct Vector2 const *a, struct Vector2 const *b, struct Vector2 const *c, struct Vector2 const *d) {
    double t = ( d->y - c->y )*( b->x - a->x ) - ( d->x - c->x )*( b->y - a->y );
    if ( fabs(t) < ALLOWABLE_ERROR ) {
        return is_on_segment(c, d, b) || is_on_segment(c, d, a) || is_on_segment(a, b, c) || is_on_segment(a, b, d);
    }else{
        double k = (( d->y - c->y )*( c->x - a->x ) - ( d->x - c->x )*( c->y - a->y )) / t;
        return k > 0 && k < 1;
        // p = (1-k)*a + k*b
    }
}

int is_collision(struct Star *a, struct Star *b, double dt) {
    //(両星間の相対速度の対面方向成分) * dt < (両星間の距離)
    double d = distance_vector(a->r, b->r);
    struct Vector2 v;   //相対速度
    copy_vector(&v, b->v);
    sub_vector(&v, a->v);
    double s = (( b->r->x - a->r->x )*v.x + ( b->r->y - a->r->y )*v.y) / d; //対面方向の成分
    return d < s * dt;
}

int collision(int const size, const double dt, struct Star *stars) {
    struct Vector2 temp1, temp2;
    for ( int i=0 ; i < size - 1; i++ ) {
        for ( int j = i + 1; j < size; j++ ) {
            copy_vector(&temp1, stars[i].v);
            mul_vector(&temp1, dt);
            add_vector(&temp1, stars[i].r);
            copy_vector(&temp2, stars[j].v);
            mul_vector(&temp2, dt);
            add_vector(&temp2, stars[j].r);
            if ( is_collision(&stars[i], &stars[j], dt) ) {
                //衝突は現在の位置からの速度ベクトルの交差で判定
                //衝突後の星はstars[i]//交点を新しい座標に設定
                add_vector(stars[i].r, stars[j].r);
                mul_vector(stars[i].r, 0.5);
                //運動量保存
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

