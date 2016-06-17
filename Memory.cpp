//
// Created by mmaximo on 17/05/16.
//

#ifndef CT204_CONTROL_VECTOR2_H
#define CT204_CONTROL_VECTOR2_H

struct Vector2 {
    double x;
    double y;

    Vector2(double x, double y);
    Vector2();
    static double dot(Vector2 v1, Vector2 v2);
    double distanceTo(Vector2& other);
};

#endif //CT204_CONTROL_VECTOR2_H
