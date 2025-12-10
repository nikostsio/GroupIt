#include "GameSolver.hpp"
#include <iostream>
#include <set>
#include <algorithm>

// HELPERS

template <typename T>
bool isValidSetFeature(const T& a, const T& b, const T& c) {
    return (a == b && b == c) || (a != b && b != c && c != a);
}

bool isGroup(const std::vector<groupitCard>& g){
    if (g.size() != 3) return false;

    return isValidSetFeature(g[0].count, g[1].count, g[2].count) &&
            isValidSetFeature(g[0].color, g[1].color, g[2].color) &&
            isValidSetFeature(g[0].fill, g[1].fill, g[2].fill) &&
            isValidSetFeature(g[0].shape, g[1].shape, g[2].shape);
}

// VISION / PARSING LOGIC

std::vector<groupitCard> getGroupItInfo(const std::vector<Color>& pixelArray, int width, int height){
    auto getPixel = [&](int x, int y) -> Color {
        if (x < 0 || x >= width || y < 0 || y >= height) return Color(0, 0, 0);
        return pixelArray[(height - 1 - y) * width + x];
    };

    // Approximation based on resolution
    const int CARD_CURVE_SIZE = width / 36;

    std::vector<int> x_positions;
    std::vector<int> y_positions;

    // We always have 4 cards horizontally so the card width is approx. {width / 4}
    // In order to be at a vertical line on top of the top left card in our grid we set
    // srch_x to be {width / 8} and srch_y equal to 0
    int srch_x = width / 8;
    int srch_y = height / 22;

    // Find the 'top' pixel of the grid
    while (!getPixel(srch_x, srch_y).isWhite() && srch_y < height) {
        srch_y++;
    }
    int top = srch_y;

    // Find the left most pixel of the card grid
    int left = width;
    while (srch_x >= 0) {
        // Account for the card curve size
        if (getPixel(srch_x, srch_y + CARD_CURVE_SIZE).isWhite() && srch_x < left) {
            left = srch_x;
        }
        srch_x--;
    }

    // Calculate the cards width and height and the positions of the cards

    // x-positions and card width
    int card_width = width;
    srch_x = left;
    srch_y = top + CARD_CURVE_SIZE;
    while (srch_x < width) {
        int curr_width = 0;
        x_positions.push_back(srch_x);
        // printf("%d\n", srch_x);

        // Traverse the card horizontally
        while(getPixel(srch_x, srch_y).isWhite() && srch_x < width) {
            srch_x++;
            curr_width++;
        }

        // Skip the non-white pixels (gap beetween cards)
        while(!getPixel(srch_x, srch_y).isWhite() && srch_x < width) {
            srch_x++;
        }

        // Update the card width
        if (curr_width > 0 && curr_width < card_width) {
            card_width = curr_width;
        }
    }

    // y-positions and card height
    int card_height = height;
    srch_x = left + CARD_CURVE_SIZE;
    srch_y = top;
    while (srch_y < (0.64 * height)) {
        int curr_height = 0;
        y_positions.push_back(srch_y);
        // printf("%d\n", srch_y);

        // Traverse the card vertically
        while(getPixel(srch_x, srch_y).isWhite() && srch_y < height) {
            srch_y++;
            curr_height++;
        }

        // Skip the non-white pixels (gap beetween cards)
        while(!getPixel(srch_x, srch_y).isWhite() && srch_y < height) {
            srch_y++;
        }

        // Update the card height
        if (curr_height > 0 && curr_height < card_height) {
            card_height = curr_height;
        }
    }

    // printf("%d %d\n", card_width, card_height);

    const int CLICK_OFFSET_X = card_width / 10;
    const int CLICK_OFFSET_Y = card_height/ 15;

    std::vector<groupitCard> cards;
    if (x_positions.empty() || y_positions.empty()) return cards;

    for (int y_pos : y_positions) {
        for (int x_pos : x_positions) {
            // If we have added three new cards then there is a total of 15 cards and not 16
            // so we need to skip that fake last card that our algorithm finds
            if (y_positions.size() == 4 && x_pos == x_positions[x_positions.size() - 1] && y_pos == y_positions[y_positions.size() - 1]) {
                continue;
            }

            groupitCard card;
            card.count = 0;

            // 1. Determine the count (Number of shapes)
            
            // We will search inside 3 rectangles evenly spaced vertically for non white pixels
            // If we find one in the rectangle region we will increment the count by 1
            int srch_rect_w = (int)(card_width * 0.71);
            int srch_rect_h = (int)(card_height * 0.18);

            for (int i = 1; i <= 3; i++) {
                // Go to the top left of the rectangle search region
                int start_x = (x_pos + card_width / 2 - srch_rect_w / 2);
                int start_y = (y_pos + card_height * i / 4 - srch_rect_h / 2);
                
                // Search inside the rectangle region
                for (srch_x = start_x; srch_x < start_x + srch_rect_w; srch_x++) {
                    bool foundShape = false;
                    for (srch_y = start_y; srch_y < start_y + srch_rect_h; srch_y++) {
                        if (!getPixel(srch_x, srch_y).isWhite()){
                            card.count++;
                            foundShape = true;
                            break;
                        }
                    }
                    if (foundShape) break;
                }
            }

            // If i dont count any shapes that means we probably have reached the end of the game 
            // so return an empty list
            if (card.count == 0) {
                return cards;
            }
            // printf("%d\n", card.count);

            // 2. Determine Color, Fill and Shape
            int center_x = x_pos + card_width / 2;
            bool foundAttributes = false;

            for (srch_y = y_pos; srch_y < y_pos + card_height; srch_y++) {
                // Find the first non white pixel in the vertical strip in the middle of the card
                Color c = getPixel(center_x, srch_y);
                if(!c.isWhite()) {
                    // --- COLOR ---
                    card.color = c.get_color();

                    // --- FILL ---
                    bool hasWhite = false;
                    bool hasColor = false;

                    // Scan a small vertical strip to detect pattern
                    for (int j = 10; j <= 20; j++) {
                        Color sample = getPixel(center_x, srch_y + j);
                        if (sample.isWhite()) hasWhite = true;
                        else if (sample.get_color() == card.color) hasColor = true;
                    }
                    if (!hasWhite && hasColor) card.fill = "solid";
                    else if (hasWhite && !hasColor) card.fill = "empty";
                    else card.fill = "striped";

                    // --- SHAPE ---
                    // Check Rectangle vs (Triangle and Diamond)
                    if (getPixel(center_x + 20, srch_y).get_color() == card.color) {
                        card.shape = "rect";
                    }
                    else {
                        if (getPixel(center_x + 20, srch_y + 8).isWhite()) {
                            card.shape = "tri";
                        }
                        else {
                            card.shape = "diam";
                        }
                    }

                    foundAttributes = true;
                    break;
                } 
            }
            // Default if detection failed (prevents crashes)
            if(!foundAttributes) {
                card.color = "unknown";
                card.fill = "unknown";
                card.shape = "unknown";
            }

            // Save the card 'click' position
            card.x_position = x_pos + CLICK_OFFSET_X;
            card.y_position = y_pos + CLICK_OFFSET_Y;
            
            // card.print();
            cards.push_back(card);
        }
    }

    return cards;
    
}

std::vector<groupitCard> findGroup(std::vector<groupitCard> cards){
    int n = (int)cards.size();
    if (n < 3) return {};

    std::vector<groupitCard> group;
    for (int i = 0; i < n - 2; i++) {
        for (int j = i + 1; j < n - 1; j++) {
            for (int k = j + 1; k < n; k++) {
                group = {cards[i], cards[j], cards[k]};
                if (isGroup(group)) {
                    return group;
                }
            }
        }
    }
    return {};
}

bool gameHasEnded(const std::vector<Color>& pixelArray, int width, int height) {
    auto getPixel = [&](int x, int y) -> Color {
        if (x < 0 || x >= width || y < 0 || y >= height) return Color(0, 0, 0);
        return pixelArray[(height - 1 - y) * width + x];
    };

    // std::cout << getPixel(width / 21, height / 15).get_color() << std::endl;
    if (!getPixel(width / 21, height / 15).isWhite()) {

        return true;
    }
    return false;

}