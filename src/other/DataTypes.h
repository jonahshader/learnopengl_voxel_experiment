//
// Created by Jonah on 8/17/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_DATATYPES_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_DATATYPES_H

/*
 * this is for structs and enums that are not ecs components
 */

class DataTypes {
public:
    DataTypes() = delete;

    enum Direction {
        X_PLUS,
        X_MINUS,
        Y_PLUS,
        Y_MINUS,
        Z_PLUS,
        Z_MINUS
    };

    struct DirectionComponents {
        signed char x;
        signed char y;
        signed char z;
    };

    enum Axis {
        X,
        Y,
        Z
    };

    static DirectionComponents directionToComponents(Direction &dir);

};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_DATATYPES_H
