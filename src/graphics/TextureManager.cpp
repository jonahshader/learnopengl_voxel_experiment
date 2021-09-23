//
// Created by Jonah on 11/3/2020.
//

#include "TextureManager.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include "other/CustomAlgorithms.h"

#include <glad/glad.h>

//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

TextureManager::TextureManager() :
        nameToSSInfo(),
        texDims(),
        nameToArrayInfo(),
        textureUnitsUsed(0)
{
}

void TextureManager::addSpriteSheet(std::string pathToImage, std::string pathToAtlas) {
// load.
// read line 2 which is the filename of the spritesheet
// skip format, filter, repeat.

// if the line starts with non space and not above, then it is the name of a texture
// record it and the xy and size into TextureInSpriteSheet struct
// add to unordered_map
// repeat until done

// load into gpu
    glActiveTexture(GL_TEXTURE0 + textureUnitsUsed);
    // load texture
    int textureWidth, textureHeight, nChannels;

    stbi_set_flip_vertically_on_load(false);
    unsigned char *textureData = stbi_load(pathToImage.c_str(), &textureWidth, &textureHeight, &nChannels, 0);

    unsigned int glTexture;
    glGenTextures(1, &glTexture);
    glBindTexture(GL_TEXTURE_2D, glTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (textureData) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }


    stbi_image_free(textureData);

    std::ifstream atlasFile(pathToAtlas);
    std::string line;

    // get passed empty lines
    do {
        std::getline(atlasFile, line);
    } while (line.find_first_not_of("\t\n ") == std::string::npos);

    // next line is filename, don't need this
    std::getline(atlasFile, line);

    TextureInSpriteSheet info;
    // store texture unit
    info.textureUnit = textureUnitsUsed;

    while (std::getline(atlasFile, line)) {
        if (line.find("format") == std::string::npos &&
            line.find("filter") == std::string::npos &&
            line.find("repeat") == std::string::npos) {
            // if not starting with space, then it is a new texture
            if (line.c_str()[0] != ' ') {
                if (!info.name.empty()) {
                    // save info to map
                    nameToSSInfo[info.name] = info;
                }
                info.name = line;
            } else if (line.find("xy") != std::string::npos) {
                auto ints = CustomAlgorithms::stringToInts(line);
                info.sx = ints[0];
                info.sy = ints[1];
            } else if (line.find("size") != std::string::npos) {
                auto ints = CustomAlgorithms::stringToInts(line);
                info.sw = ints[0];
                info.sh = ints[1];
            }
        }
    }

    // add last texture
    nameToSSInfo[info.name] = info;

    TextureDimensions dim;
    dim.textureWidth = textureWidth;
    dim.textureHeight = textureHeight;
    texDims.emplace_back(dim);
    ++textureUnitsUsed;
}

// TODO: add retainAlpha argument
void TextureManager::addSpriteArray(std::string pathToImage, std::string pathToAtlas) {
// load.
// read line 2 which is the filename of the spritesheet
// skip format, filter, repeat.
// get size

// if the line starts with non space and not above, then it is the name of a texture
// record it and the xy and size into TextureInSpriteSheet struct
// add to unordered_map
// repeat until done


    std::ifstream atlasFile(pathToAtlas);
    std::string line;

    // get passed empty lines
    do {
        std::getline(atlasFile, line);
    } while (line.find_first_not_of("\t\n ") == std::string::npos);

    // next line is filename, don't need this
    std::getline(atlasFile, line);

    TextureInArray info;

    // store texture unit
    info.textureUnit = textureUnitsUsed;

    int w = 0;
    int h = 0;
    int numTextures = 0;

    while (std::getline(atlasFile, line)) {
        std::cout << line << std::endl;
        if (line.find("format") == std::string::npos &&
            line.find("filter") == std::string::npos &&
            line.find("repeat") == std::string::npos) {
            if (line.find("size") != std::string::npos) {
                auto ints = CustomAlgorithms::stringToInts(line);
                w = ints[0];
                h = ints[1];
            } else if (line.c_str()[0] != ' ') {
                if (!info.name.empty()) {
                    // save info to map
                    nameToArrayInfo[info.name] = info;
//                    std::cout << "added tex: " << line << std::endl;
                    ++numTextures;
                }
                info.name = line;

            } else if (line.find("index: ") != std::string::npos) {
                auto ints = CustomAlgorithms::stringToInts(line);
                info.index = ints[0];
            } else {
                std::cout << "Bad line" << std::endl;
            }
        } else {
//            std::cout << "Unwanted line" << std::endl;
        }
    }

    // save last texture
    ++numTextures;
    nameToArrayInfo[info.name] = info;
//    std::cout << "added tex: " << info.name << std::endl;

    // load into gpu
    glActiveTexture(GL_TEXTURE0 + textureUnitsUsed);
    // load texture
    int textureWidth, textureHeight, nChannels;

    stbi_set_flip_vertically_on_load(false);
    unsigned char *textureData = stbi_load(pathToImage.c_str(), &textureWidth, &textureHeight, &nChannels, 0);

    unsigned int glTexture;
    glGenTextures(1, &glTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, glTexture);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

//    std::cout << "Num textures" << numTextures << std::endl;
//    std::cout << "w: " << w << " h: " << h << std::endl;
    if (textureData) {
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, w, h, numTextures, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }


    stbi_image_free(textureData);

    TextureDimensions dims;
    dims.textureWidth = textureWidth;
    dims.textureHeight = textureHeight;
    texDims.emplace_back(dims);
    ++textureUnitsUsed;
}


TextureManager::TextureInSpriteSheet TextureManager::getTextureInfoSS(std::string textureName) {
    return nameToSSInfo[textureName];
}

TextureManager::TextureInArray TextureManager::getTextureInfoA(std::string textureName) {
    return nameToArrayInfo[textureName];
}

TextureManager::TextureDimensions TextureManager::getTextureDimensions(int texture) {
    return texDims[texture];
}

void TextureManager::combineSpritesIntoArray(std::vector<std::string> imagePaths) {
    std::vector<unsigned char> data;

}

