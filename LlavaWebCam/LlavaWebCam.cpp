#include "LlavaWebCam.h"
#include <opencv2/opencv.hpp>
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

std::mutex responseMutex;
std::string responseString;
std::atomic<bool> requestInProgress(false);

// Callback to handle cURL response and extract the "response" field
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    std::string data((char*)contents, totalSize);

    // Find the "response" field and extract its value
    size_t responsePos = data.find("\"response\":\"");
    if (responsePos != std::string::npos) {
        size_t start = responsePos + 12;
        size_t end = data.find("\"", start);
        if (end != std::string::npos) {
            *response += data.substr(start, end - start);
        }
    }
    return totalSize;
}

// Function to send cURL request with base64 image and retrieve response
std::string sendCurlRequest(const std::string& base64Image) {
    CURL* curl;
    CURLcode res;
    std::string responseString;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/generate");

        std::string jsonData = "{"
            "\"model\": \"llava:13b\", "
            "\"prompt\": \"What is in this picture? Answer in 15 words or less\", "
            "\"images\": [\"" + base64Image + "\"]"
            "}";

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    curl_global_cleanup();

    return responseString;
}

// Function to encode image as base64
std::string base64Encode(const std::vector<uchar>& buffer) {
    static const char encodeLookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encodedString;
    encodedString.reserve(((buffer.size() / 3) + (buffer.size() % 3 > 0)) * 4);
    uint32_t temp{};
    int bitsCollected = 0;

    for (const auto& byte : buffer) {
        temp <<= 8;
        temp += byte;
        bitsCollected += 8;
        while (bitsCollected >= 6) {
            bitsCollected -= 6;
            encodedString += encodeLookup[(temp >> bitsCollected) & 0x3F];
        }
    }
    if (bitsCollected > 0) {
        temp <<= 6 - bitsCollected;
        encodedString += encodeLookup[temp & 0x3F];
    }
    while (encodedString.size() % 4) encodedString += '=';
    return encodedString;
}

// Function to process frame and add text
void processFrame(cv::Mat& frame, const std::string& responseText) {
    cv::putText(frame, responseText, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 2);
}

void requestThreadFunction(const std::string& base64Image) {
    std::string newResponse = sendCurlRequest(base64Image);
	std::lock_guard<std::mutex> lock(responseMutex);
    responseString = newResponse;
	requestInProgress = false;
}

int main() {
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
        return -1;
    }

    cv::namedWindow("Webcam Test", cv::WINDOW_AUTOSIZE);
    int frameCounter = 0;

    while (true) {
        cv::Mat frame;
        cap >> frame;

        if (frame.empty()) {
            std::cerr << "Error: Blank frame grabbed" << std::endl;
            break;
        }

        // Every 100 frames, capture image, encode, and send new cURL request
        if (frameCounter % 100 == 0 && !requestInProgress) {
            std::vector<uchar> buffer;
            cv::imencode(".jpg", frame, buffer);
            std::string base64Image = base64Encode(buffer);

			requestInProgress = true;
            // Get new response
			std::thread(requestThreadFunction, base64Image).detach();
        }

        // Process and display frame
        processFrame(frame, responseString);
        cv::imshow("Webcam Test", frame);

        // Break the loop if 'q' key is pressed
        char key = (char)cv::waitKey(1);
        if (key == 'q' || key == 27 /* ESC */) {
            break;
        }

        frameCounter++;
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}
