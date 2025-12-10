// GameSolver.hpp

#pragma once
#include <vector>
#include <string>
#include "ImageUtils.hpp"

struct groupitCard{
    int count;
    std::string color;
    std::string fill;
    std::string shape;
    int x_position;
    int y_position;

    void print() const{
        printf("{%d %s %s %s}\n", count, color.c_str(), fill.c_str(), shape.c_str());
    }
    
    bool operator ==(const groupitCard &other){
        return (count == other.count && color == other.color && 
                fill == other.fill && shape == other.shape && 
                x_position == other.x_position && y_position == other.y_position);
    }
};

bool isGroup(const std::vector<groupitCard>& possibleGroup);
std::vector<groupitCard> getGroupItInfo(const std::vector<Color>& pixelArray, int width, int height);
std::vector<groupitCard> findGroup(std::vector<groupitCard> cards);
bool gameHasEnded(const std::vector<Color>& pixelArray, int width, int height);