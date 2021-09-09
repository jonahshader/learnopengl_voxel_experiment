//
// Created by Jonah on 9/3/2021.
//

#include "TerrainFunctions.h"

// posTerrain_t posTerrain_t
posTerrain_t operator+(const posTerrain_t a, const posTerrain_t b) {
    return [=](FN_VEC3 pos) {
        return a(pos) + b(pos);
    };
}

posTerrain_t operator-(const posTerrain_t a, const posTerrain_t b) {
    return [=](FN_VEC3 pos) {
        return a(pos) - b(pos);
    };
}

posTerrain_t operator*(const posTerrain_t a, const posTerrain_t b) {
    return [=](FN_VEC3 pos) {
        return a(pos) * b(pos);
    };
}

posTerrain_t operator/(const posTerrain_t a, const posTerrain_t b) {
    return [=](FN_VEC3 pos) {
        return a(pos) / b(pos);
    };
}

// posTerrain_t and decimal
posTerrain_t operator+(const posTerrain_t a, const FN_DECIMAL b) {
    return [=](FN_VEC3 pos) {
        return a(pos) + b;
    };
}

posTerrain_t operator-(const posTerrain_t a, const FN_DECIMAL b) {
    return [=](FN_VEC3 pos) {
        return a(pos) - b;
    };
}

posTerrain_t operator*(const posTerrain_t a, const FN_DECIMAL b) {
    return [=](FN_VEC3 pos) {
        return a(pos) * b;
    };
}

posTerrain_t operator/(const posTerrain_t a, const FN_DECIMAL b) {
    return [=](FN_VEC3 pos) {
        return a(pos) / b;
    };
}

// decimal posTerrain_t
posTerrain_t operator+(const FN_DECIMAL a, const posTerrain_t b) {
    return [=](FN_VEC3 pos) {
        return a + b(pos);
    };
}

posTerrain_t operator-(const FN_DECIMAL a, const posTerrain_t b) {
    return [=](FN_VEC3 pos) {
        return a - b(pos);
    };
}

posTerrain_t operator*(const FN_DECIMAL a, const posTerrain_t b) {
    return [=](FN_VEC3 pos) {
        return a * b(pos);
    };
}

posTerrain_t operator/(const FN_DECIMAL a, const posTerrain_t b) {
    return [=](FN_VEC3 pos) {
        return a / b(pos);
    };
}

void operator+=(posTerrain_t &a, const posTerrain_t &b) {
    a = a + b;
}

void operator-=(posTerrain_t &a, const posTerrain_t &b) {
    a = a - b;
}

void operator*=(posTerrain_t &a, const posTerrain_t &b) {
    a = a * b;
}

void operator/=(posTerrain_t &a, const posTerrain_t &b) {
    a = a / b;
}

void operator+=(posTerrain_t &a, const FN_DECIMAL b) {
    a = a + b;
}

void operator-=(posTerrain_t &a, const FN_DECIMAL b) {
    a = a - b;
}

void operator*=(posTerrain_t &a, const FN_DECIMAL b) {
    a = a * b;
}

void operator/=(posTerrain_t &a, const FN_DECIMAL b) {
    a = a / b;
}

posTerrain_t applyCoordShift(const posTerrain_t terrain, const coordShift_t coordShift) {
    return [=](FN_VEC3 pos) {
        return terrain(coordShift(pos));
    };
}