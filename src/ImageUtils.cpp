#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cmath>
#include "ImageUtils.hpp"

/* COLOR IMPLEMENTATION */

Color::Color() : r(0), g(0), b(0) {}
Color::Color(float r, float g, float b) : r(r), g(g), b(b) {}

void Color::print(){
    printf("(%f, %f, %f)\n", r * 255.0f, g * 255.0f, b * 255.0f);
}

bool Color::operator ==(const Color &other){
    const float epsilon = 0.001f;
    return ((std::fabs(r -other.r) < epsilon) && 
            (std::fabs(g - other.g) < epsilon) && 
            (std::fabs(b - other.b) < epsilon));
}

bool Color::operator !=(const Color &other){
    return !(*this == other);
}

bool Color::isWhite(){
    // Check if pixel is bright enough
    return (r * 255.0f >= 150 && g * 255.0f >= 150 && b * 255.0f >= 150);
}

std::string Color::get_color(){
    if (r > g && r > b){
        return "red";
    }
    else if (g > r && g > b){
        return "green";
    }
    else if (b > r && b > g){
        return "blue";
    }
    return "hmm";
}

/* IMAGE IMPLEMENTATION */

Image::Image(int width, int height) : m_width(width),
                                        m_height(height),
                                        m_colors(std::vector<Color>(width*height)) {}
Color Image::getColor(int x, int y) const{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return Color(0,0,0); // Return black if out of bounds
    }
    return m_colors[y * m_width + x];
}

void Image::setColor(const Color &color, int x, int y){
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        m_colors[y * m_width + x] = color;
    }
}

void Image::readImage(const char* path){
    std::ifstream f;
    f.open(path, std::ios::in | std::ios::binary);

    if(!f.is_open()){
        std::cout << "The file could not be opened" << std::endl;
        return;
    }

    const int fileHeaderSize = 14;

    std::vector<unsigned char> fileHeader(fileHeaderSize);
    f.read(reinterpret_cast<char*>(fileHeader.data()), fileHeaderSize);

    if (fileHeader[0] != 'B' || fileHeader[1] != 'M'){
        std::cout << "The specified path is not a bitmap image" << std::endl;
        f.close();
        return;
    }

    // Read DIB header size
    unsigned char dibHeaderSizeBytes[4];
    f.read(reinterpret_cast<char*>(dibHeaderSizeBytes), 4);
    
    int dibHeaderSize = static_cast<int>(dibHeaderSizeBytes[0]) |
                        (static_cast<int>(dibHeaderSizeBytes[1]) << 8) |
                        (static_cast<int>(dibHeaderSizeBytes[1]) << 16) | 
                        (static_cast<int>(dibHeaderSizeBytes[1]) << 24);
    
    f.seekg(fileHeaderSize, std::ios::beg);

    std::vector<unsigned char> dibHeader(dibHeaderSize);
    f.read(reinterpret_cast<char*>(dibHeader.data()), dibHeaderSize);

    int fileSize = fileHeader[2] + (fileHeader[3] << 8) + (fileHeader[4] << 16) + (fileHeader[5] << 24);
    m_width = static_cast<int>(dibHeader[4]) + 
                (static_cast<int>(dibHeader[5]) << 8) + 
                (static_cast<int>(dibHeader[6]) << 16) + 
                (static_cast<int>(dibHeader[7]) << 24);
    m_height = static_cast<int>(dibHeader[8]) + 
                (static_cast<int>(dibHeader[9]) << 8) + 
                (static_cast<int>(dibHeader[10]) << 16) + 
                (static_cast<int>(dibHeader[11]) << 24);

    m_colors.resize(m_width * m_height);

    /* int additionalPadding = (4 - (fileHeaderSize + dibHeaderSize) % 4) % 4;
    f.ignore(additionalPadding); */

    const int paddingAmount = ((4 - (m_width * 3) % 4) % 4);

    for (int y = 0; y < m_height; y++){
        for (int x = 0; x < m_width; x++){
            unsigned char color[3];
            f.read(reinterpret_cast<char*>(color), 3);
            
            // BGR to RGB conversion
            m_colors[y * m_width + x].r = static_cast<float>(color[2]) / 255.0f;
            m_colors[y * m_width + x].g = static_cast<float>(color[1]) / 255.0f;
            m_colors[y * m_width + x].b = static_cast<float>(color[0]) / 255.0f;
        }
        f.ignore(paddingAmount);
    }
    f.close();
    std::cout << "File read succesfully: " << m_width << "x" << m_height << std::endl;
}

void Image::exportImage(const char* path) const{
    std::ofstream f;
    f.open(path, std::ios::out | std::ios::binary);

    if (!f.is_open()){
        std::cerr << "File could not be opened" << std::endl;
        return;
    }

    const int paddingAmount = ((4 - (m_width * 3) % 4) % 4);
    /* printf("%d\n", paddingAmount); */
    std::vector<char> bmpPadding(paddingAmount, 0);


    const int fileHeaderSize = 14;
    const int dibHeaderSize = 40;
    const int fileSize = fileHeaderSize + dibHeaderSize + m_width * m_height * 3 + paddingAmount * m_height;

    unsigned char fileHeader[fileHeaderSize] = {
        'B', 'M',
        static_cast<unsigned char>(fileSize),
        static_cast<unsigned char>(fileSize >> 8),
        static_cast<unsigned char>(fileSize >> 16),
        static_cast<unsigned char>(fileSize >> 24),
        0, 0,
        0, 0,
        static_cast<unsigned char>(fileHeaderSize + dibHeaderSize), 0, 0, 0
    };

    // Initialize everything to 0
    unsigned char dibHeader[dibHeaderSize] = {0};

    dibHeader[0] = dibHeaderSize;
    dibHeader[1] = 0;
    dibHeader[2] = 0;
    dibHeader[3] = 0;

    dibHeader[4] = static_cast<unsigned char>(m_width);
    dibHeader[5] = static_cast<unsigned char>(m_width >> 8);
    dibHeader[6] = static_cast<unsigned char>(m_width >> 16);
    dibHeader[7] = static_cast<unsigned char>(m_width >> 24);
    
    dibHeader[8] = static_cast<unsigned char>(m_height);
    dibHeader[9] = static_cast<unsigned char>(m_height >> 8);
    dibHeader[10] = static_cast<unsigned char>(m_height >> 16);
    dibHeader[11] = static_cast<unsigned char>(m_height >> 24);

    dibHeader[12] = 1;
    dibHeader[13] = 0;

    dibHeader[14] = 24;
    dibHeader[15] = 0;
    
    dibHeader[16] = 0;
    dibHeader[17] = 0;
    dibHeader[18] = 0;
    dibHeader[19] = 0;

    dibHeader[20] = 0;
    dibHeader[21] = 0;
    dibHeader[22] = 0;
    dibHeader[23] = 0;
    
    // Desired horizontal pixel resolution
    // How many pixels per meter
    dibHeader[24] = 0;
    dibHeader[25] = 0;
    dibHeader[26] = 0;
    dibHeader[27] = 0;

    // Desired vertical pixel resolution
    // How many pixels per meter
    dibHeader[28] = 0;
    dibHeader[29] = 0;
    dibHeader[30] = 0;
    dibHeader[31] = 0;
    
    dibHeader[32] = 0;
    dibHeader[33] = 0;
    dibHeader[34] = 0;
    dibHeader[35] = 0;
    
    dibHeader[36] = 0;
    dibHeader[37] = 0;
    dibHeader[38] = 0;
    dibHeader[39] = 0;

    f.write(reinterpret_cast<char*>(fileHeader), fileHeaderSize);
    f.write(reinterpret_cast<char*>(dibHeader), dibHeaderSize);

    for (int y = 0; y < m_height; y++){
        for (int x = 0; x < m_width; x++){
            const Color& c = getColor(x, y);
            unsigned char r = static_cast<unsigned char>(c.r * 255.0f);
            unsigned char g = static_cast<unsigned char>(c.g * 255.0f);
            unsigned char b = static_cast<unsigned char>(c.b * 255.0f);

            unsigned char color[] = {b, g, r};
            f.write(reinterpret_cast<char*>(color), 3);
        }
        f.write(bmpPadding.data(), paddingAmount);

    }
    f.close();
    std::cout << "Image exported to: " << path << std::endl;
}

int Image::get_width(){
    return m_width;
}

int Image::get_height(){
    return m_height;
}
const std::vector<Color>& Image::get_pixelArray(){
    return m_colors;
}