#include <iostream>
#include <thread>
#include <chrono>
#include "ImageUtils.hpp"
#include "GameSolver.hpp"
#include "DeviceController.hpp"


// Define constants for configuration
const char* SCREENSHOT_PATH = "images/screenshot.bmp";

int main(){
    std::cout << "--- GroupIt Auto-Solver Started ---" << std::endl;
    
    int groupsFound = 0;
    while (1) {
        // 1. Capture the game state
        take_screenshot();

        // 2. Load the image into memory
        Image img(0, 0);
        img.readImage(SCREENSHOT_PATH);

        // Validation: Check if image loaded correctly
        if (img.get_width() == 0 || img.get_height() == 0) {
            std::cerr << "[Fatal Error] Failed to load image data. Check ImageMagick and ADB." << std::endl;
            exit(1);
        }

        // 3. Computer Vision: Detect cards
        std::vector<groupitCard> cards = getGroupItInfo(img.get_pixelArray(), img.get_width(), img.get_height());
        // for (const auto& card: cards) {
        //     card.print();
        // }
        if (cards.empty()) {
            // Check if the game has ended by checking the color of a specific pixel
            if (gameHasEnded(img.get_pixelArray(), img.get_width(), img.get_height())) {
                break;
            }

            std::cerr << "[Warning] No cards detected. Is the game screen open?" << std::endl;
            return 0; 
        }

        // 4. Algorithm: Find a valid group
        std::vector<groupitCard> group = findGroup(cards);

        if (group.empty()) {

            // Click the "Add Cards" button
            add_new_cards(img.get_width(), img.get_height());
        } 
        else {
            click_on_cards(group);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(700));
    }

    std::cout << "--- Done ---" << std::endl;
    return 0;
}