#include <stdio.h>
#include <iostream>
#include <thread>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include "BackgroundProcessing.h"
#include "camControl.hpp"
#include "Maxim14574.h"

int maxData, height, width, softCount, data = 0;
int secondIterationData[15];

double maxScore, score = 0;

bool frameReady = true;
bool firstIterationDone, exitProg, secondIteration = false;

const int maxDataValue = 65535;
const int increment1 = maxDataValue / 25;
const int yMin = 373;
const int yMax = 723;
const int xMin = 793;
const int xMax = 1143;
const int lowDataThreshold = 2570;
const int highDataThreshold = 62966;
const int increment2 = 256;

MAX14574Driver driver("/dev/ttyUSB0");
idsCamera cam;

void imageProcessing(cv::Mat frame){

    cv::Mat sobel;
    cv::Scalar meanTemp;
    float mean;
    
    score = 0;

    if(!firstIterationDone)
        driver.setValue(data + increment1);   
        // write next value, before image processing, so the lens has enough time to re-focus
    
    frame = frame(cv::Range(yMin, yMax), cv::Range(xMin, xMax)); // crop image for faster processing (square in the middle of the frame)
    cv::Sobel(frame, sobel, CV_8UC1, 1, 0, 3, 4, 45);
    
    meanTemp = cv::mean(sobel); // calculate mean of the edge detector output
    mean = meanTemp.val[0];
    score = mean;

    // std::cout << "Image captured at focus data: " << data << "processed. Score: " << score << std::endl << std::endl;

    if(score > maxScore) {
        maxScore = score;
        maxData = data;

        // std::cout << "Saving new max focus data: " << maxData << std::endl;
    }
    
    if(!firstIterationDone)
        data += increment1;

    if(data > maxDataValue - increment1)
        firstIterationDone = true;

    if(firstIterationDone) {    
        if(!secondIteration) {  // start second iteration
            secondIteration = true;
            
            // calculate data for next steps
            if(maxData > lowDataThreshold && maxData < highDataThreshold) {
                for(int i = -5; i <= 5; i++) {
                    secondIterationData[i + 5] = maxData + (i * increment2);
                }
            }
            else if(maxData >= highDataThreshold) {
                for(int i = 0; i <= 10; i++) {
                    secondIterationData[i] = highDataThreshold + (i * increment2);
                }
            }
            else if(maxData <= lowDataThreshold) {
                for(int i = 0; i <= 10; i++) {
                    secondIterationData[i] = i * increment2;
                }
            }
        }
        
        if(softCount < 10) { // scan 10 frames, using smaller increment1
            // std::cout << "soft focus -- writing " << secondIterationData[softCount] << " to the driverboard" << std::endl;
            data = secondIterationData[softCount];
            driver.setValue(data);
            softCount++;
        }
        else {  // focusing finished
            // exitProg = true;  // exit after finishing
            data = maxData;
            driver.setValue(maxData);
        }
    }
}

int main(int , char **) {
    
	BackgroundProcessing camProcessor([](cv::Mat s){
		imageProcessing(s);
	});

    int cnt = 0;
    bool autoGain = true;

    driver.setValue(0);     // reset lens focus

    cam.setDefaultCam();
    cam.setAutoGain(&autoGain);
    cam.getResolution(&width, &height);

	cv::Mat frame(height, width, CV_8UC1);
    auto begin = std::chrono::high_resolution_clock::now();
    
    for(;;) {
        cam.getFrame(&frame);

        if(cnt >= 3)    // skip first 3 frames due to camera initialization
            camProcessor.frameReady(frame);

        cv::imshow("Live", frame);
        cv::waitKey(1);
        
        if(exitProg) {
            auto end = std::chrono::high_resolution_clock::now();
            std::cout << (std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count()) / 1000000000.0 << "s" << std::endl;
            return 0;
        }
        cnt++;
    }

    return 0;
}
