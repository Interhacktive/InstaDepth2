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
		if(name=="FOCUS DISTANCE")
	{
		ofxUISlider *sender = (ofxUISlider *)e.widget;
		focus = sender->getScaledValue();
			cam.setScale(focus);
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


	if(name=="OBJECT SIZE")
	{
		ofxUISlider *sender = (ofxUISlider *)e.widget;
		cutoffNear = sender->getScaledValue();
	}
		
	if(name=="CUTOFF FAR")
	{
		ofxUISlider *sender = (ofxUISlider *)e.widget;
		cutoffFar = sender->getScaledValue();
	}

	if(name=="SAMPLER")
	{
		ofxUIImageSampler *sender = (ofxUIImageSampler *)e.widget;
		environmentColor = sender->getColor();
	}
		if(name=="OBJECT COLOR")
	{
		ofxUIImageSampler *sender = (ofxUIImageSampler *)e.widget;
		objectColor = sender->getColor();
	}
}

void testApp::setupGUI()
{
	mGUI = new ofxUICanvas(0,0,colorSelector->getWidth()+10,800);
	mGUI->addFPS();
	mGUI->addSpacer(150,2);
	mGUI->setTheme(4);
	mGUI->setColorBack(ofColor(50,50,50,200)); 
	mGUI->addSlider("SCALE",0.1f,20,mScale,150,10);
	mGUI->addSlider("OBJECT SIZE",0.1f,2,objectSize,150,10);
	mGUI->addSlider("OBJECT SIZE FALLOFF",0.1f,2,objectSizeFalloff,150,10);
	mGUI->addSpacer(150,2);
	//mGUI->addSlider("OBJECT SCALE",0.1f,30,mScale,150,10);
	mGUI->addSlider("FOCUS DISTANCE",0.0f,3.0f,focus,150,10);
	//mGUI->addSpacer(150,2);

	mGUI->addSlider("CUTOFF NEAR",0.0f,200,cutoffNear,150,10);
	mGUI->addSlider("CUTOFF FAR",0.0f,1500,cutoffFar,150,10);
	mGUI->addSpacer(150,2);

	mGUI->addWidgetDown(new ofxUIImageSampler(colorSelector->getWidth(), colorSelector->getHeight(), colorSelector, "SAMPLER"));
	mGUI->addSpacer(150,2);
	mGUI->addWidgetDown(new ofxUIImageSampler(colorSelector->getWidth(), colorSelector->getHeight(), colorSelector, "OBJECT COLOR"));


	mGUI->addLabelToggle("COLOR", false, 150,10);
	mGUI->addDropDownList("RES", mSteps,150);
	ofAddListener(mGUI->newGUIEvent,this,&testApp::guiEvent);	
	//mGUI->toggleVisible();
}

void testApp::setup()
{
	ofSetWindowShape(768,768);
	
	ofSetFrameRate(60);
	mode = 0; //modes = 0-preview, 1-pointmenu, 2-enhancement, 3-saved, 4-gallery
	pointMode = 0; //0-boxes, 1-spheres, 2-points, 3-lines, 4-mesh
	objectSize = 0.25;
	objectSizeFalloff = 0.25;
	mScale = 20; //was 6
	camScale = 1;
	mSkip = 1;
	camDist = 1;
	showInterface = false;
	focusAngleLocked = false;
	snapCounter = 0;
	cutoffNear = 200;
	cutoffFar = 32000;
	mColor = true;
	mBlur = 0.6f;
	previewMode = true;
	ofSetSphereResolution(6);
	mSteps.push_back("1");mSteps.push_back("2");mSteps.push_back("4");mSteps.push_back("8");mSteps.push_back("16");mSteps.push_back("20");
	
	

	  // Setup post-processing chain
    post.init(ofGetScreenWidth(), ofGetScreenHeight());
    post.createPass<FxaaPass>()->setEnabled(true);
    post.createPass<BloomPass>()->setEnabled(false);
    post.createPass<DofPass>()->setEnabled(true);
	//printf("stuff and %f",post.createPass<DofPass>()->getFocus());
	//post.createPass<DofPass>()->setMaxBlur(0.1);
    post.createPass<KaleidoscopePass>()->setEnabled(false);
    post.createPass<NoiseWarpPass>()->setEnabled(false);
    post.createPass<PixelatePass>()->setEnabled(false);
    post.createPass<EdgePass>()->setEnabled(false);
    

	mSession = PXCUPipeline_Create();
	if(!PXCUPipeline_Init(mSession, (PXCUPipeline)(PXCU_PIPELINE_COLOR_WXGA|PXCU_PIPELINE_DEPTH_QVGA)))
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
	rgbTexture.allocate(mCW, mCH, GL_RGB, true); //OF_IMAGE_COLOR);

	bgImage.loadImage("bg0.jpg");

	depthImage.allocate(mDW, mDH, OF_IMAGE_COLOR_ALPHA);
	colorImage.allocate(mDW, mDH, OF_IMAGE_COLOR_ALPHA);
	colorSelector = new ofImage(); 
	colorSelector->loadImage("colorSelector.png");
	setupGUI();

	//Image filters
		dir.allowExt("cube");
	dir.listDir("LUTs/");
	dir.sort();
	if (dir.size()>0) {
		dirLoadIndex=0;
		loadLUT(dir.getPath(dirLoadIndex));
		doLUT = true;
	}else{
		doLUT = false;
	}
	
	lutImg.allocate(mCW, mCH, OF_IMAGE_COLOR);
	
	
	thumbPos.set(lutImg.getWidth()*0.5f-80, -lutImg.getHeight()*0.5f - 60, 0);
	lutPos.set(-lutImg.getWidth()*0.5f, -lutImg.getHeight()*0.5f, 0);

		
}

void testApp::update()
{
	if(mode == 0){
		preview(); //update live preview
	}

}

void testApp::preview(){
	

if(PXCUPipeline_AcquireFrame(mSession,false))
	{

		mVerts.clear();
		mColors.clear();
		//mTexCoords.clear();
		if(PXCUPipeline_QueryDepthMap(mSession, mDepthMap))                        
		{
			PXCUPipeline_QueryUVMap(mSession, mUVMap);
			if(mColor)
				PXCUPipeline_QueryRGB(mSession, mRGBMap);
#if 1
			//rgbTexture.loadData(mRGBMap,mCW, mCH, GL_RGB);
			//ofPixelsRef pix = (ofPixelsRef)mRGBMap;
			
			

			for(int y1=1; y1<mDH-mSkip; y1+=mSkip)
			{
				int y0=y1-1;
				for(int x1=1; x1<mDW-mSkip; x1+=mSkip)
				{
					int x0=x1-1;

					const struct { int x,y,z; } vert[] = {
						{ x0, y0, mDepthMap[y0*mDW+x0] },
						{ x1, y0, mDepthMap[y0*mDW+x1] },
						{ x1, y1, mDepthMap[y1*mDW+x1] },
						{ x0, y1, mDepthMap[y1*mDW+x0] }
					};
					bool bad=false;
					ofVec2f uv[4];
					for(int i=0; i<4; ++i)
					{
						uv[i] = ofVec2f(mUVMap[(vert[i].y*mDW+vert[i].x)*2+0], mUVMap[(vert[i].y*mDW+vert[i].x)*2+1]);
						if(vert[i].z > cutoffFar|| uv[i].x < 0 || uv[i].x >= 1 || uv[i].y < 0 || uv[i].y >= 1)
						{
							bad = true;
							break;
						}
					};
					if(bad) continue;

					for(int i=0; i<4; ++i)
					{
						//mTexCoords.push_back(uv[i]);
						float vx = ofMap((float)vert[i].x,0,mDW,-mXOffset,mXOffset);
						float vy = ofMap((float)vert[i].y,0,mDH,-mYOffset,mYOffset);
						int sx = uv[i].x*mCW;
						int sy = uv[i].y*mCH;
						float _r = mRGBMap[(sy*mCW+sx)*4]/255.0f;
						float _g = mRGBMap[(sy*mCW+sx)*4+1]/255.0f;
						float _b = mRGBMap[(sy*mCW+sx)*4+2]/255.0f;
						mColors.push_back(ofFloatColor(_r,_g,_b,1.0f));
						float maxRange = 100; //was 240
						float oldDist = ofMap((float)vert[i].z,0,1800,-maxRange,maxRange, true);
						float newDist = ofMap((float)vert[i].z, 0, 1800, 0, 1);
						mVerts.push_back(ofVec3f(vx*newDist,vy*newDist,oldDist));
					}
				}
			}			
#else
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
							int sx=(int)(mUVMap[(y*mDW+x)*2+0]*mCW);//mDW+0.5)*2
							int sy=(int)(mUVMap[(y*mDW+x)*2+1]*mCH);//mDH+0.5)*2
							//everything is between 320x240
							if(sx>0&&sx<mCW&&sy>0&&sy<mCH)
							{
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
							mColors.push_back(objectColor);
							float maxRange = 100; //was 240
								float oldDist = ofMap((float)mDepthMap[di],0,1800,-maxRange,maxRange, true);
								float newDist = ofMap((float)mDepthMap[di], 0, 1800, 0, 1);
								mVerts.push_back(ofVec3f(vx*newDist,vy*newDist,oldDist));
						}
					}
				}
			}

#endif
		}
		PXCUPipeline_ReleaseFrame(mSession);
		mTotal = mColors.size();
		mVBO.setColorData(&mColors[0],mTotal, GL_STREAM_DRAW);
		//mVBO.setTexCoordData(&mTexCoords[0], (int)mTexCoords.size(), GL_STREAM_DRAW); // HERE
		mTotal = mVerts.size();
		if(mTotal>0)
			mVBO.setVertexData(&mVerts[0], mTotal, GL_STREAM_DRAW);
	}


}

void testApp::draw()
{
	ofBackground(environmentColor);

	ofSetColor(255, 255);
	
	//if(debugOn){
	//	debug();
	//}
	/*
	for (unsigned i = 0; i < post.size(); ++i)
    {
        if (post[i]->getEnabled()) ofSetColor(0, 255, 255);
        else ofSetColor(255, 0, 0);
        ostringstream oss;
        oss << i << ": " << post[i]->getName() << (post[i]->getEnabled()?" (on)":" (off)");
        ofDrawBitmapString(oss.str(), 10, 20 * (i + 2));
   
		
	}
	*/
	
	//vector<RenderPass::Ptr>& myPtr = post.getPasses();
	//itg::DofPass.setFocus(mouseX);
	//post[2].
	
		

	//Draw circle to indicate rotation areas
	ofPushStyle();
	ofNoFill();
	ofSetCircleResolution(40);
	ofSetColor(255,150);
	ofEllipse(ofGetWidth()/2, ofGetHeight()/2, ofGetHeight(),ofGetHeight());

	ofPopStyle();

	post.begin(cam);

   //cam.begin();
	ofScale(mScale, -mScale, -mScale); // make y point down
		//cam.setDistance(mouseX);
	
	float imageScale = 0.1;
	bgImage.draw( -(bgImage.width*imageScale)/2,-(bgImage.height*imageScale)/2, bgImage.width*imageScale, bgImage.height*imageScale);
	//lutImg.draw(lutPos.x, lutPos.y);
	glEnable(GL_DEPTH_TEST);//required to render properly
		//drawGrid(100,100);
	if(mode == 0){
		
		//draw points
		if(mTotal>0){
			glPointSize(7.5);
			//mVBO.draw(GL_POINTS, 0, mTotal);
			//rgbTexture.bind();
			mVBO.draw(GL_LINE_STRIP, 0, mTotal);//GL_QUADS
			//rgbTexture.unbind();
		}

	}else if(mode == 1){//Points Menu
	
		
			
			for(int i = 0;i<mColors.size()-mSkip+3;i+=mSkip+3){
				ofPushMatrix();	
				if(mColor){
					ofSetColor(mColors[i]);
				}else{
					ofSetColor(objectColor);
				}
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
				}

				ofPopMatrix();
			}

			if(pointMode ==2){
				
					glPointSize(10);
					
					
					if(mTotal>0)mVBO.draw(GL_POINTS, 0, mTotal);

				} else if(pointMode == 3){
					
					//if(mTotal>0)mVBO.draw(GL_LINES, 0, mTotal);
					//if(mTotal>0)mVBO.draw(GL_LINE_STRIP, 0, mTotal);
					mVBO.draw(GL_QUADS, 0, mTotal);
					//if(mTotal>0)mVBO.draw(GL_POLYGON, 0, mTotal);
				}
			
			
	
	}

	 post.end();
    
	//cam.end();
}

void testApp::exit()
{
	PXCUPipeline_Close(mSession);
}

void testApp::loadLUT(string path){
	LUTloaded=false;
	
	ofFile file(path);
	string line;
	for(int i = 0; i < 5; i++) {
		getline(file, line);
		ofLog() << "Skipped line: " << line;	
	}
	for(int z=0; z<32; z++){
		for(int y=0; y<32; y++){
			for(int x=0; x<32; x++){
				ofVec3f cur;
				file >> cur.x >> cur.y >> cur.z;
				lut[x][y][z] = cur;
			}
		}
	}
	
	LUTloaded = true;
}

void testApp::applyLUT(ofPixelsRef pix){
	if (LUTloaded) {
		
		for(int y = 0; y < pix.getHeight(); y++){
			for(int x = 0; x < pix.getWidth(); x++){
				
				ofColor color = pix.getColor(x, y);
				
				int lutPos [3];
				for (int m=0; m<3; m++) {
					lutPos[m] = color[m] / 8;
					if (lutPos[m]==31) {
						lutPos[m]=30;
					}
				}
				
				ofVec3f start = lut[lutPos[0]][lutPos[1]][lutPos[2]];
				ofVec3f end = lut[lutPos[0]+1][lutPos[1]+1][lutPos[2]+1]; 
				
				for (int k=0; k<3; k++) {
					float amount = (color[k] % 8) / 8.0f;
					color[k]= (start[k] + amount * (end[k] - start[k])) * 255;
				}
				
				lutImg.setColor(x, y, color);
				
			}			
		}
		
		lutImg.update();
	}
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
								
								depthImage.setColor(x, y, ofFloatColor(ofMap((float)mDepthMap[di],0,600,0,1, true), ofMap((float)mDepthMap[di],600,1200,0,1, true), ofMap((float)mDepthMap[di],1200,1800,0,1, true),1.0f));
									if(d < 32000) colorImage.setColor(x, y, ofFloatColor(_r, _g, _b, 1.0f));
								
							}
						}
					

					/*
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

						*/

					//}
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
						
					}
				}
			
		
	
		mTotal = mColors.size();
		mVBO.setColorData(&mColors[0],mTotal, GL_STREAM_DRAW);

		mTotal = mVerts.size();
		if(mTotal>0){
			mVBO.setVertexData(&mVerts[0], mTotal, GL_STREAM_DRAW);
		}

}

void testApp::keyPressed(int key){
 
    if (key == 's'){
		savePointCloud();
	}  else if (key == 'p'){
		screenShot.grabScreen(0,0,ofGetScreenWidth(),ofGetScreenHeight());

		string fileName = "snapshot_"+ofToString(100+snapCounter)+".png";
		screenShot.saveImage(fileName);
		snapCounter++;
	}

	else if (key == 'd'){
		//debugOn = !debugOn;
	}
	else if (key == '['){
		camScale +=0.1;
		//cam.setPosition(0,0,camDist);
	}
	else if (key == ']'){
		camScale -= 0.1;
		
	//	cam.setPosition(0,0,camDist);
	}
		else if (key == '-'){
		mBlur +=0.1;
		//cam.setPosition(0,0,camDist);
		//post.getPasses()
		//post.createPass<DofPass>()->setEnabled(false);
			
		post.createPass<DofPass>()->setFocus(mBlur);
	}
	else if (key == '='){
		mBlur -= 0.1;
		//itg::DofPass.setFocus(mBlur);
	//itg::
		
		post.createPass<DofPass>()->setFocus(mBlur);
	//	cam.setPosition(0,0,camDist);
	}

	
	else if (key == 'c'){
		focusAngleLocked = !focusAngleLocked;
	}
	else if (key == '1'){
		bgImage.loadImage("bg1.jpg");
	}
	else if (key == '2'){
		bgImage.loadImage("bg2.jpg");
	}
	else if (key == '3'){
		bgImage.loadImage("bg3.jpg");
	}
	else if (key == '4'){
		bgImage.loadImage("bg4.jpg");
	}
	else if (key == '5'){
		bgImage.loadImage("bg5.jpg");
	}
	else if (key == '6'){
		bgImage.loadImage("bg6.jpg");
	}
	else if (key == '7'){
		bgImage.loadImage("bg7.jpg");
	}
	else if (key == '8'){
		bgImage.loadImage("bg8.jpg");
	}
	else if (key == '9'){
		bgImage.loadImage("bg9.jpg");
	}	
	else if (key == '0'){
		bgImage.loadImage("bg0.jpg");
	}
	else if (key == 'h'){

		mGUI->toggleVisible();
	}else if (key == 'l'){		
		loadPointCloud();
	}else if(key == ' '){
		if(mode < 1){
			mode++;
			//applyLUT((ofPixelsRef)mColors);
			//applyLUT(
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
	