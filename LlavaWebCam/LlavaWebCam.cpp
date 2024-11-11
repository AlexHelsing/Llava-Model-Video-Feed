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
std::atomic<int> latestRequestID(0);
std::atomic<bool> requestInProgress(false); // Reintroduce requestInProgress



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
            "\"model\": \"llava:7b\", "
            "\"prompt\": \"What is in this picture? Answer in 10 words or less\", "
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

void processFrame(cv::Mat& frame, const std::string& responseText) {
	cv::copyMakeBorder(frame, frame, 30, 0, 0, 0, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
    cv::putText(frame, responseText, cv::Point(0, 20),
        cv::FONT_HERSHEY_COMPLEX, 0.5,
        cv::Scalar(0, 255, 0), 1);
}


void requestThreadFunction(const std::string& base64Image, int requestID) {
    std::string newResponse = sendCurlRequest(base64Image);

    // Update the response only if it's from the latest request
    if (requestID == latestRequestID.load()) {
        std::lock_guard<std::mutex> lock(responseMutex);
        responseString = newResponse;
    }

    // Set requestInProgress to false after the request completes
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

        // Every 25 frames, capture image, encode, and send new cURL request
        if (frameCounter % 35 == 0 && !requestInProgress.load()) {
            // Prepare image for sending (resizing and encoding)
            cv::Mat resizedFrame;
            cv::resize(frame, resizedFrame, cv::Size(640, 360));

            std::vector<int> compression_params = { cv::IMWRITE_JPEG_QUALITY, 50 };
            std::vector<uchar> buffer;
            cv::imencode(".jpg", resizedFrame, buffer, compression_params);

            std::string base64Image = base64Encode(buffer);

            // Set requestInProgress to true before starting the request
            requestInProgress = true;

            // Increment and get the current request ID
            int currentRequestID = ++latestRequestID;

            // Launch the request thread with the current request ID
            std::thread(requestThreadFunction, base64Image, currentRequestID).detach();
        }

        // Process and display frame
        {
            std::lock_guard<std::mutex> lock(responseMutex);
            processFrame(frame, responseString);
        }
        cv::imshow("LlavaStream", frame);

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
