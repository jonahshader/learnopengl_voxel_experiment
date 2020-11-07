//
// Created by Jonah on 11/3/2020.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_TEXTUREMANAGER_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_TEXTUREMANAGER_H

#include <string>
#include <unordered_map>

class TextureManager {
public:
    TextureManager();
    struct TextureInSpriteSheet {
        std::string name;
        unsigned int textureUnit;
        unsigned int tx, ty, w, h;
    };

    struct TextureInArray {
        std::string name;
        unsigned int textureUnit;
        unsigned int w, h;
        unsigned int index;
    };

    void addSpriteSheet(std::string pathToImage, std::string pathToAtlas);
    void addSpriteArray(std::string pathToImage, std::string pathToAtlas);

    TextureInSpriteSheet getTextureInfoSS(std::string textureName);
    TextureInArray getTextureInfoA(std::string textureName);

private:
    std::unordered_map<std::string, TextureInSpriteSheet> nameToSSInfo;
    std::unordered_map<std::string, TextureInArray> nameToArrayInfo;

    unsigned int textureUnitsUsed;

};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_TEXTUREMANAGER_H
