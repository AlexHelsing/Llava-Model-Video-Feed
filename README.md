# Llava Webcam AI Integration

## Overview
This project connects a live webcam feed to a locally hosted Llava AI model (Llava:7b). The application captures an image from the webcam, sends it as a `cURL` request to the Llava model, and displays the AI's response directly on the video feed. The program leverages multithreading to ensure smooth video playback while interacting with the AI model asynchronously.

## Features
- **Real-time Webcam Feed**: Captures live video from your webcam.
- **AI-Powered Insights**: Sends webcam frames to a locally hosted Llava AI model for analysis.
- **Dynamic Response Display**: Overlays the Llava model's response on the video feed in real-time.
- **Efficient Asynchronous Design**: Multithreaded implementation ensures the video feed remains smooth while communicating with the AI model.

## Motivation
The inspiration for this project came from my desire to experiment with locally hosted AI models in a more interactive and visually engaging way. Instead of limiting the model to static images or text prompts, I thought it would be exciting to integrate it into a video program that responds dynamically to what the camera sees.

This project showcases:
- The potential of AI when integrated with real-time systems like video feeds.
- Practical use of cURL for sending requests to an AI model.
- Handling asynchronous tasks in a multithreaded application.
- Combining computer vision (OpenCV) with AI-powered backend analysis.

## Key Technologies
- **OpenCV**: Used for real-time webcam video capture and processing.
- **cURL**: For sending HTTP POST requests to the Llava model API.
- **C++ Multithreading**: Ensures smooth video performance while awaiting responses from the AI model.
- **Base64 Encoding**: Converts captured frames into a format suitable for API transmission.

## How It Works
1. Captures the live webcam feed using OpenCV.
2. Every 100th frame:
   - Encodes the frame in JPEG format.
   - Converts the encoded image to a Base64 string.
   - Sends the string as part of a JSON payload to the Llava API.
3. Displays the AI's response on the video feed using OpenCV's `putText`.
4. Uses multithreading to ensure that sending requests and receiving responses doesn’t interrupt the video feed.

## Use Cases
- **AI Experimentation**: Test and visualize the capabilities of your locally hosted AI model in real-time.
- **Interactive AI Applications**: Build engaging applications that combine live video and AI insights.
- **AI Showcase**: Demonstrate the potential of locally hosted AI for interactive and responsive systems.

## Why This Project?
This project bridges the gap between static AI use cases and real-time interactivity. It demonstrates how locally hosted AI models can be integrated into dynamic applications, showcasing the possibilities of personalized AI experiences without relying on cloud-based solutions.

By working on this, I learned:
- How to efficiently handle asynchronous tasks in video-based applications.
- The nuances of sending image data to APIs in Base64 format.
- Real-time visualization of AI responses and integrating them with computer vision tools like OpenCV.

## Future Improvements
- **Performance Tuning**: Improve the performance by tweaking the image size, compress etc. 
- **Enhanced UI**: Add overlays, menus, or interaction options to make the application more user-friendly.

## Youtube Video
[![Llava Webcam AI Integration](https://alexanderhelsing.com/assets/me.webp)](https://www.youtube.com/watch?v=IamS8hGH2mE)

## How to Run
1. Clone this repository.
2. Ensure your Llava model is hosted locally and accessible via the specified API endpoint.
3. Install the necessary dependencies: OpenCV, cURL, and C++ multithreading libraries. (vcpkg recommended for CMake).
4. Build the application using your preferred C++ compiler (e.g., g++, Visual Studio, or CMake).
5. Run the application and watch as the AI interacts with your webcam feed in real time.

---

This project reflects the exciting possibilities of real-time AI applications. Feel free to experiment with the code or extend it for your own use cases! 🚀
