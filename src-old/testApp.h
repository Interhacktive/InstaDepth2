#ifndef TEST_APP_GUARD
#define TEST_APP_GUARD

#include <vector>
#include "ofMain.h"
#include "ofxUI.h"
#include "pxcupipeline.h"

class testApp : public ofBaseApp {
public:

	void setup();
	void update();
	void draw();
	void exit();

	void preview();
	void drawGrid(float x, float y);
	void savePointCloud();
	void loadPointCloud();
	
	//void gallery();
	//void enhance();
	//void pointsMenu();
//	void debug();
	void keyPressed(int key);	
	

private:

	bool previewMode, debugOn, showInterface, save, mColor, rotateCircle, focusAngleLocked;

	int mDW, mDH, mCW, mCH, mSkip, mTotal;
	int mode, pointMode, enhanceMode, environmentMode;

	float mScale, mXOffset, mYOffset;
	float objectSize, objectSizeFalloff, focus, focusFalloff,blurDist, blurFalloff, fogAmount, fogFalloff, cutoffNear, cutoffFar;

	ofVec3f targetPos, cameraPos;

	ofColor objectColor, environmentColor; 
	short *mDepthMap;
	ofImage *colorSelector;

	float *mUVMap;
	unsigned char *mRGBMap;
	vector<ofVec3f> mVerts;
	vector<ofFloatColor> mColors;
	vector<string> mSteps;

	ofEasyCam cam;
	ofVbo mVBO;
	ofxUICanvas *mGUI;
	ofImage depthImage;
	ofImage colorImage;

	void setupGUI();
	void guiEvent(ofxUIEventArgs &e);

	PXCUPipeline_Instance mSession;
	
};

#endif