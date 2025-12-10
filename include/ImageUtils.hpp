// ImageUtils.hpp

#pragma once
#include <string>
#include <vector>

class Color{
    public:
        float r, g, b;

        Color();
        Color(float r, float g, float b);
		void print();
		bool operator ==(const Color &other);
		bool operator !=(const Color &other);
		bool isWhite();
		std::string get_color();
};

class Image{
    public:
        Image(int width, int height);
        Color getColor(int x, int y) const;
        void setColor(const Color &color, int x, int y);
        void readImage(const char* path);
        void exportImage(const char* path) const;
        int get_width();
        int get_height();
        const std::vector<Color>& get_pixelArray();
    private:
        int m_width;
        int m_height;
        std::vector<Color> m_colors;
};