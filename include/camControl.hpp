#ifndef CAMCONTROL_H
#define CAMCONTROL_H

//Defined resolution of camera
//IF one value is 0, then maximal resoution
#define RES_WIDTH 0
#define RES_HEIGHT 0


#include <iostream>
#include <ueye.h>

#include <opencv2/opencv.hpp>


/* Function defined in library ueye.h
// IDSEXP is_GetNumberOfCameras (INT* pnNumCams);


*/


/* Class for one cammera to work with
 */
class idsCamera{

	private:
		int displWidth = RES_WIDTH;
		int displHeight = RES_HEIGHT;


		int memID = 0;
		char* camMem;

		CAMINFO camInfo;
		SENSORINFO senInfo;

		void initResolution();


	public:
		//Get any camera
		idsCamera();
		//Get one camera
		idsCamera(int id);

		~idsCamera();

		
		void getResolution(int* wid, int* hei);
		bool setDefaultCam();
        void setAutoGain(bool* enable);

		void getFrame(cv::Mat *frame);

		HIDS camId = 0;

		double fps;

};





#endif
