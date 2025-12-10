#include "DeviceController.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
#include <thread> // Required for sleep
#include <chrono>

void take_screenshot(){
    // Capture screenshot to the phone's storage
    const char* cmdCapture = "adb shell screencap -p /sdcard/temp_groupit.png";
    if (system(cmdCapture) != 0) {
        std::cerr << "Failed to capture screenshot on device" << std::endl;
        return;
    }

    // Pull the screenshot to my laptop
    const char* cmdPull = "adb pull /sdcard/temp_groupit.png images/screenshot.png > NUL 2>&1";
    if (system(cmdPull) != 0) {
        std::cerr << "Failed to pull screenshot to laptop" << std::endl;
    }

    // Clean up the file on the phone
    system("adb shell rm /sdcard/temp_groupit.png");

    // Convert PNG to BMP using ImageMagick
    const char* cmdConvert = "magick images/screenshot.png -type TrueColor images/screenshot.bmp";
    if (system(cmdConvert) != 0) {
        std::cerr << "Image conversion failed. Is Magick installed?" << std::endl;
    }
    else {
        std::cout << "Screenshot processed successfully!" << std::endl;
    }
}

void click_on_cards(const std::vector<groupitCard>& group){
    if (group.empty()) {
        std::cout << "No group found to click" << std::endl;
    }

    for (const groupitCard &card : group){
        std::string com = "adb shell input tap " + std::to_string(card.x_position) + " " + std::to_string(card.y_position);

        const char* command = com.c_str();

        if (system(command) != 0) {
            std::cerr << "Failed to click at: (" << card.x_position << ", " << card.y_position << ")" << std::endl;
        }

        // Small delay between clicks to ensure phone registers them
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void add_new_cards(int width, int height){
    std::string cmd = "adb shell input tap " + std::to_string(width * 0.76) + " " + std::to_string(height * 0.80);
    system(cmd.c_str());
}