//
// Created by Jonah on 8/17/2020.
//

#include "DataTypes.h"

DataTypes::DirectionComponents DataTypes::directionToComponents(Direction &dir) {
    DataTypes::DirectionComponents comps{};
    switch (dir) {
        case X_PLUS:
            comps.x = 1;
            break;
        case X_MINUS:
            comps.x = -1;
            break;
        case Y_PLUS:
            comps.y = 1;
            break;
        case Y_MINUS:
            comps.y = -1;
            break;
        case Z_PLUS:
            comps.z = 1;
            break;
        case Z_MINUS:
            comps.z = -1;
            break;
        default:
            break;
    }
    return comps;
}
