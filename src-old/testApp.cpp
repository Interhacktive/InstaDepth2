#include <sstream>
#include "testApp.h"

void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	if(name=="SCALE")
	{
		ofxUISlider *sender = (ofxUISlider *)e.widget;
		mScale = sender->getScaledValue();
	}
	if(name=="RES")
	{
		ofxUIDropDownList *sender = (ofxUIDropDownList *)e.widget;
		vector<ofxUIWidget *> &dlist = sender->getSelected();
		if(dlist.size()>0)
		{
			stringstream convert(dlist[0]->getName());
			convert >> mSkip;
		}
	}
	
	if(name=="OBJECT SIZE")
	{
		ofxUISlider *sender = (ofxUISlider *)e.widget;
		objectSize = sender->getScaledValue();
	}

	if(name=="OBJECT SIZE FALLOFF")
	{
		ofxUISlider *sender = (ofxUISlider *)e.widget;
		objectSizeFalloff = sender->getScaledValue();
	}

	
	if(name=="COLOR")
	{
		ofxUILabelToggle *sender = (ofxUILabelToggle *)e.widget;
		mColor = sender->getValue();
	}

	if(name=="SAMPLER")
	{
		ofxUIImageSampler *sender = (ofxUIImageSampler *)e.widget;
		environmentColor = sender->getColor();
	}
}

void testApp::setup()
{
	ofSetWindowShape(768,768);
	ofSetFrameRate(60);
	mode = 0; //modes = 0-preview, 1-pointmenu, 2-enhancement, 3-saved, 4-gallery
	pointMode = 0; //0-boxes, 1-spheres, 2-points, 3-lines, 4-mesh
	objectSize = 3;
	objectSizeFalloff = 0.5;
	mScale = 2;
	mSkip = 1;
	showInterface = false;
	focusAngleLocked = false;
	mColor = true;
	previewMode = true;
	ofSetSphereResolution(6);
	mSteps.push_back("1");mSteps.push_back("2");mSteps.push_back("4");mSteps.push_back("8");mSteps.push_back("16");mSteps.push_back("20");
	

	mSession = PXCUPipeline_Create();
	if(!PXCUPipeline_Init(mSession, (PXCUPipeline)(PXCU_PIPELINE_COLOR_VGA|PXCU_PIPELINE_DEPTH_QVGA)))
		ofLogNotice() << "Unable to initialize session" << endl;
	if(PXCUPipeline_QueryDepthMapSize(mSession, &mDW, &mDH))
	{
		mDepthMap = new short[mDW*mDH];
		mUVMap = new float[mDW*mDH*2];
		mXOffset = mDW*0.5f;
		mYOffset = mDH*0.5f;
	}
	if(PXCUPipeline_QueryRGBSize(mSession, &mCW, &mCH))
		mRGBMap = new unsigned char[mCW*mCH*4];

	depthImage.allocate(mDW, mDH, OF_IMAGE_COLOR_ALPHA);
	colorImage.allocate(mDW, mDH, OF_IMAGE_COLOR_ALPHA);
	colorSelector = new ofImage(); 
	colorSelector->loadImage("colorSelector.png");
	setupGUI();
		
}

void testApp::update()
{
	if(mode == 0){
		preview(); //update live preview
	}

}

void testApp::draw()
{
	ofBackground(environmentColor);
	ofSetColor(255, 255);
	//if(debugOn){
	//	debug();
	//}


	//Draw circle to indicate rotation areas
	ofPushStyle();
	ofNoFill();
	ofSetCircleResolution(40);
	ofSetColor(255,150);
	ofEllipse(ofGetWidth()/2, ofGetHeight()/2, ofGetHeight(),ofGetHeight());

	ofPopStyle();


	cam.begin();
	ofScale(mScale, -mScale, -mScale); // make y point down

	glEnable(GL_DEPTH_TEST);//required to render properly
		drawGrid(100,100);
	if(mode == 0){
		
		//draw points
		if(mTotal>0){
			mVBO.draw(GL_POINTS, 0, mTotal);
		}

	}else if(mode == 1){//Points Menu

		
			
			for(int i = 0;i<mColors.size()-mSkip;i+=mSkip){
				ofPushMatrix();		
				ofSetColor(mColors[i]);
				ofTranslate(mVerts[i]);

				ofVec3f pointPos = (ofVec3f)mVerts[i];
					if(!focusAngleLocked){
						cameraPos = cam.getPosition();
					}
					float distance = pointPos.distance(cameraPos); // distance is 5.8310	
				if(pointMode == 0){//Draw box cloud
					ofBox(ofMap(distance, 0, 1000, objectSize, objectSizeFalloff, false)); //magic numbers beware
				} else if(pointMode == 1){
					ofSphere(ofMap(distance, 0, 1000, objectSize, objectSizeFalloff, false)); //magic numbers beware
				} else if(pointMode == 2){
					glPointSize(10);
					
					
					if(mTotal>0)mVBO.draw(GL_POINTS, 0, mTotal);
				} else if(pointMode == 3){
					//if(mTotal>0)mVBO.draw(GL_LINES, 0, mTotal);
					
				}

				ofPopMatrix();
			}
			
	
	}
	cam.end();

	
}

void testApp::exit()
{
	PXCUPipeline_Close(mSession);
}

void testApp::setupGUI()
{
	mGUI = new ofxUICanvas(0,0,colorSelector->getWidth()+10,800);
	mGUI->addFPS();
	mGUI->addSpacer(150,2);
	mGUI->setColorBack(ofColor(50,50,50,200)); 
	//	mGUI->addSlider("SCALE",0.1f,2,mScale,150,10);
	mGUI->addSlider("OBJECT SIZE",0.1f,2,objectSize,150,10);
	mGUI->addSlider("OBJECT SIZE FALLOFF",0.1f,2,objectSizeFalloff,150,10);
	mGUI->addSpacer(150,2);

	mGUI->addSlider("CUTOFF NEAR",0.0f,200,cutoffNear,150,10);
	mGUI->addSlider("CUTOFF FAR",0.0f,200,cutoffFar,150,10);
	mGUI->addSpacer(150,2);

	mGUI->addWidgetDown(new ofxUIImageSampler(colorSelector->getWidth(), colorSelector->getHeight(), colorSelector, "SAMPLER"));


	mGUI->addLabelToggle("COLOR", false, 150,10);
	mGUI->addDropDownList("RES", mSteps,150);
	ofAddListener(mGUI->newGUIEvent,this,&testApp::guiEvent);	
	//mGUI->toggleVisible();
}

void testApp::drawGrid(float x, float y)
{
	int camW = 320;
	int camH = 240;
	ofPushMatrix();
	ofScale(0.5,0.5,1);
	ofTranslate(camW/-2, camH/-2, 0);
    float w = camW; 
    float h = camH; 
    
    for(int i = 0; i < h; i+=y)
    {
        ofLine(0,i,w,i); 
    }
    
    for(int j = 0; j < w; j+=x)
    {
        ofLine(j,0,j,h); 
    }    
	ofPopMatrix();
}

void testApp::savePointCloud(){
	
	if(PXCUPipeline_AcquireFrame(mSession,false))
	{

		mVerts.clear();
		mColors.clear();
		if(PXCUPipeline_QueryDepthMap(mSession, mDepthMap))                        
		{
			PXCUPipeline_QueryUVMap(mSession, mUVMap);
			if(mColor)
				PXCUPipeline_QueryRGB(mSession, mRGBMap);

			
			for(int y=0;y<mDH-mSkip;y+=mSkip)
			{
				for(int x=0;x<mDW-mSkip;x+=mSkip)
				{
					if(save){
					colorImage.setColor(x, y, ofFloatColor(0,0,0, 0.0f));
					depthImage.setColor(x, y, ofFloatColor(0,0,0, 0.0f));
					}

					int di = y*mDW+x;
					float d = (float)mDepthMap[di];
					if(d<32000) //artificial cutoff was 32000
					{
						float vx = ofMap(x,0,mDW,-mXOffset,mXOffset);
						float vy = ofMap(y,0,mDH,-mYOffset,mYOffset);
						
						if(mColor)
						{
							int sx=(int)(mUVMap[(y*mDW+x)*2+0]*mDW+0.5) * 2;
							int sy=(int)(mUVMap[(y*mDW+x)*2+1]*mDH+0.5) * 2;
							//everything is between 320x240
							if(sx>0&&sx<mCW&&sy>0&&sy<mCH)
							{
								
								//crack out individual color vals and scale
								float _r = mRGBMap[(sy*mCW+sx)*4]/255.0f;
								float _g = mRGBMap[(sy*mCW+sx)*4+1]/255.0f;
								float _b = mRGBMap[(sy*mCW+sx)*4+2]/255.0f;
								mColors.push_back(ofFloatColor(_r,_g,_b,1.0f));
								mVerts.push_back(ofVec3f(vx,vy,ofMap((float)mDepthMap[di],0,1800,-240,240, true)));
								
								
									depthImage.setColor(x, y, ofFloatColor(ofMap((float)mDepthMap[di],0,600,0,1, true), ofMap((float)mDepthMap[di],600,1200,0,1, true), ofMap((float)mDepthMap[di],1200,1800,0,1, true),1.0f));
									if(d < 32000) colorImage.setColor(x, y, ofFloatColor(_r, _g, _b, 1.0f));
								
							}
						}
						else
						{
							mColors.push_back(ofFloatColor(1,1,1,1));
							mVerts.push_back(ofVec3f(vx,vy,ofMap((float)mDepthMap[di],0,1800,-240,240, true)));
						}
					}
				}
			}
		}
		PXCUPipeline_ReleaseFrame(mSession);
		mTotal = mColors.size();
		mVBO.setColorData(&mColors[0],mTotal, GL_STREAM_DRAW);

		mTotal = mVerts.size();
		if(mTotal>0)
			mVBO.setVertexData(&mVerts[0], mTotal, GL_STREAM_DRAW);
		
			depthImage.update();
			colorImage.update();
	

	}

	depthImage.saveImage("depthImage.png", OF_IMAGE_QUALITY_BEST);     
	colorImage.saveImage("colorImage.png", OF_IMAGE_QUALITY_BEST);
}

void testApp::preview(){
	

if(PXCUPipeline_AcquireFrame(mSession,false))
	{

		mVerts.clear();
		mColors.clear();
		if(PXCUPipeline_QueryDepthMap(mSession, mDepthMap))                        
		{
			PXCUPipeline_QueryUVMap(mSession, mUVMap);
			if(mColor)
				PXCUPipeline_QueryRGB(mSession, mRGBMap);

			
			for(int y=0;y<mDH-mSkip;y+=mSkip)
			{
				for(int x=0;x<mDW-mSkip;x+=mSkip)
				{
					

					int di = y*mDW+x;
					float d = (float)mDepthMap[di];
					if(d<32000) //artificial cutoff was 32000
					{
						float vx = ofMap(x,0,mDW,-mXOffset,mXOffset);
						float vy = ofMap(y,0,mDH,-mYOffset,mYOffset);
						
						if(mColor)
						{
							int sx=(int)(mUVMap[(y*mDW+x)*2+0]*mDW+0.5) * 2;
							int sy=(int)(mUVMap[(y*mDW+x)*2+1]*mDH+0.5) * 2;
							//everything is between 320x240
							if(sx>0&&sx<mCW&&sy>0&&sy<mCH)
							{
								
								//crack out individual color vals and scale
								float _r = mRGBMap[(sy*mCW+sx)*4]/255.0f;
								float _g = mRGBMap[(sy*mCW+sx)*4+1]/255.0f;
								float _b = mRGBMap[(sy*mCW+sx)*4+2]/255.0f;
								mColors.push_back(ofFloatColor(_r,_g,_b,1.0f));
								float maxRange = 100; //was 240
								float oldDist = ofMap((float)mDepthMap[di],0,1800,-maxRange,maxRange, true);
								float newDist = ofMap((float)mDepthMap[di], 0, 1800, 0, 1);
								mVerts.push_back(ofVec3f(vx*newDist,vy*newDist,oldDist));
								
								
							}
						}
						else
						{
							mColors.push_back(ofFloatColor(1,1,1,1));
							mVerts.push_back(ofVec3f(vx,vy,ofMap((float)mDepthMap[di],0,1800,-240,240, true)));
						}
					}
				}
			}
		}
		PXCUPipeline_ReleaseFrame(mSession);
		mTotal = mColors.size();
		mVBO.setColorData(&mColors[0],mTotal, GL_STREAM_DRAW);

		mTotal = mVerts.size();
		if(mTotal>0)
			mVBO.setVertexData(&mVerts[0], mTotal, GL_STREAM_DRAW);
	}


}

void testApp::loadPointCloud(){
		depthImage.loadImage("depthImage.png");
		depthImage.update();

		colorImage.loadImage("colorImage.png");
		colorImage.update();


		mVerts.clear();
		mColors.clear();
	
			for(int y=0;y<mDH-mSkip;y+=mSkip)
			{
				for(int x=0;x<mDW-mSkip;x+=mSkip)
				{


					int di = y*mDW+x;
					float d = (float)mDepthMap[di];
					if(d<32000) //artificial cutoff was 32000
					{
						float vx = ofMap(x,0,mDW,-mXOffset,mXOffset);
						float vy = ofMap(y,0,mDH,-mYOffset,mYOffset);
						
						if(mColor)
						{
							int sx=(int)(mUVMap[(y*mDW+x)*2+0]*mDW+0.5) * 2;
							int sy=(int)(mUVMap[(y*mDW+x)*2+1]*mDH+0.5) * 2;
							//everything is between 320x240
							if(sx>0&&sx<mCW&&sy>0&&sy<mCH)
							{
								
								//crack out individual color vals and scale
								
								mColors.push_back(colorImage.getColor(x, y));
								//map from -240, 240. Originally 0-1800.
								int depthValue = (int)ofMap(depthImage.getColor(x,y).r, 0,255,0,600, true)+(int)ofMap(depthImage.getColor(x,y).g, 0,255,0,600, true)+(int)ofMap(depthImage.getColor(x,y).b, 0,255,0,600, true);
								mVerts.push_back(ofVec3f(vx,vy,ofMap((float)depthValue,0,1800,-240,240, true)));

							}
						}
						else
						{
							mColors.push_back(ofFloatColor(1,1,1,1));
							mVerts.push_back(ofVec3f(vx,vy,ofMap((float)mDepthMap[di],0,1800,-240,240, true)));
						}
					}
				}
			}
		
	
		mTotal = mColors.size();
		mVBO.setColorData(&mColors[0],mTotal, GL_STREAM_DRAW);

		mTotal = mVerts.size();
		if(mTotal>0){
			mVBO.setVertexData(&mVerts[0], mTotal, GL_STREAM_DRAW);
		}

}
/*
void testApp::debug(){
depthImage.draw(200,200);
colorImage.draw(200,500);
}
*/


void testApp::keyPressed(int key){
 
    if (key == 's'){
		savePointCloud();
	}

	else if (key == 'd'){
		//debugOn = !debugOn;
	}
	else if (key == 'c'){
		focusAngleLocked = !focusAngleLocked;
	}
	else if (key == 'h'){

		mGUI->toggleVisible();
	}else if (key == 'l'){		
		loadPointCloud();
	}else if(key == ' '){
		if(mode < 1){
			mode++;
		} else{
			mode = 0;
		}

	}else if(key == 'm'){
		if(pointMode < 3){
			pointMode++;
		} else{
			pointMode = 0;
		}

	}
}
	