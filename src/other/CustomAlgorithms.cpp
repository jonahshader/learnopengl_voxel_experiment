//
// Created by Jonah on 9/28/2020.
//

#include "CustomAlgorithms.h"

#include <iostream>
#include <sstream>

void CustomAlgorithms::progressPointOnLine(glm::ivec3 &voxel, glm::vec3 &position, glm::vec3 &direction) {
    
}

// modified version of https://www.geeksforgeeks.org/extract-integers-string-c/
std::vector<int> CustomAlgorithms::stringToInts(std::string input) {
    std::vector<int> ints;

    std::stringstream ss;

    /* Storing the whole string into string stream */
    ss << input;

    /* Running loop till the end of the stream */
    std::string temp;
    int found;
    while (!ss.eof()) {

        /* extracting word by word from stream */
        ss >> temp;

        /* Checking the given word is integer or not */
        if (std::stringstream(temp) >> found) {
            ints.emplace_back(found);
        }
        /* To save from space at the end of string */
        temp = "";
    }

    return ints;
}
