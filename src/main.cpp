#include "testApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 768, 768, OF_FULLSCREEN);
	ofRunApp(new testApp());
}
