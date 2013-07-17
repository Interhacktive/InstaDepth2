#ifndef TEST_APP_GUARD
#define TEST_APP_GUARD

#include <vector>
#include "ofMain.h"
#include "ofxUI.h"
#include "pxcupipeline.h"
#include "ofxPostProcessing.h"

class testApp : public ofBaseApp {
public:

	void setup();
	void update();
	void draw();
	void exit();

	void preview();
	void savePointCloud();
	void loadPointCloud();
	void loadLUT(string path);
	void applyLUT(ofPixelsRef pix);
	
	//void gallery();
	//void enhance();
	//void pointsMenu();
//	void debug();
	void keyPressed(int key);	

	
	bool doLUT;
	ofVideoGrabber 		vidGrabber;
	int dirLoadIndex;
	ofDirectory dir;
	ofPoint lutPos;
	ofPoint thumbPos;
	
	bool LUTloaded;
	ofVec3f lut[32][32][32];
	
	ofImage lutImg;
	

private:
	 ofxPostProcessing post;
	bool previewMode, debugOn, showInterface, save, mColor, rotateCircle, focusAngleLocked;

	int mDW, mDH, mCW, mCH, mSkip, mTotal;
	int mode, pointMode, enhanceMode, environmentMode;

	float mScale, mXOffset, mYOffset,mBlur, camScale;
	float objectSize, objectSizeFalloff, focus, focusFalloff,blurDist, blurFalloff, fogAmount, fogFalloff, cutoffNear, cutoffFar;
	float camDist;
	ofVec3f targetPos, cameraPos;

	ofColor objectColor, environmentColor; 
	short *mDepthMap;
	ofImage *colorSelector;
	ofImage screenShot;
	int snapCounter;
	ofLight pointLight;
	float *mUVMap;
	unsigned char *mRGBMap;
	vector<ofVec3f> mVerts;
	vector<ofFloatColor> mColors;
	vector<ofVec2f> mTexCoords;
	vector<string> mSteps;

	ofEasyCam cam;
	ofVbo mVBO;
	ofxUICanvas *mGUI;
	ofImage depthImage;
	ofImage colorImage;
	ofImage bgImage;
	ofTexture rgbTexture;
	void setupGUI();
	void guiEvent(ofxUIEventArgs &e);

	PXCUPipeline_Instance mSession;
	
};

#endif