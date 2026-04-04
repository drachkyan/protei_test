#pragma once

struct ENodeConfig {
    const int id;
    const double x;
    const double power;
    const double radius;

    ENodeConfig(int id_, double x_, double power_, double radius_): 
        id(id_), x(x_), power(power_), radius(radius_) {};
};