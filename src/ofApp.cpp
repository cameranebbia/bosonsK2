#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	kinect.open();
	kinect.initDepthSource();

	grayPix.allocate(512, 424, OF_IMAGE_GRAYSCALE);

	ofSetVerticalSync(false);
	ofSetLogLevel(OF_LOG_NOTICE);

	partNone.loadImage("partNone.jpg");

	partNoneRotVel = 0.7;
	partNoneRotAmount = 10;

	drawWidth = 1280;
	drawHeight = 720;
	// process all but the density on 16th resolution
	flowWidth = drawWidth / 4;
	flowHeight = drawHeight / 4;


	// FLOW & MASK
	opticalFlow.setup(flowWidth, flowHeight);
	velocityMask.setup(drawWidth, drawHeight);

	// FLUID & PARTICLES
	fluidSimulation.setup(flowWidth, flowHeight, drawWidth, drawHeight);
	particleFlow.setup(flowWidth, flowHeight, drawWidth, drawHeight);



	velocityDots.setup(flowWidth / 4, flowHeight / 4);

	// VISUALIZATION
	displayScalar.setup(flowWidth, flowHeight);
	velocityField.setup(flowWidth / 4, flowHeight / 4);
	temperatureField.setup(flowWidth / 4, flowHeight / 4);
	pressureField.setup(flowWidth / 4, flowHeight / 4);
	velocityTemperatureField.setup(flowWidth / 4, flowHeight / 4);

	// MOUSE DRAW
	mouseForces.setup(flowWidth, flowHeight, drawWidth, drawHeight);


	didCamUpdate = false;
	cameraFbo.allocate(kinectWidth, kinectHeight);
	cameraFbo.black();




	mirrorH = false;
	mirrorV = false;

	showMask = false;

	showInfo = false;

	showKinect = false;

	grayImageIn.allocate(kinectWidth, kinectHeight);
	grayImageThresh.allocate(kinectWidth, kinectHeight);
	grayImageKinectOld.allocate(kinectWidth, kinectHeight);
	grayImageKinectDiff.allocate(kinectWidth, kinectHeight);

	// GUI
	setupGui();

	lastTime = ofGetElapsedTimef();

	background.loadSound("audio/bg.wav");
	sound.loadSound("audio/stein6.wav");

	background.setVolume(0.3);
	background.setLoop(true);
	sound.setVolume(0);
	sound.setLoop(true);

	background.play();
	sound.play();


}

//--------------------------------------------------------------
void ofApp::setupGui() {

	gui.setup("settings");
	gui.setDefaultBackgroundColor(ofColor(0, 0, 0, 127));
	gui.setDefaultFillColor(ofColor(160, 160, 160, 160));
	gui.add(guiFPS.set("average FPS", 0, 0, 60));
	gui.add(guiMinFPS.set("minimum FPS", 0, 0, 60));
	gui.add(doFullScreen.set("fullscreen (F)", false));
	doFullScreen.addListener(this, &ofApp::setFullScreen);
	gui.add(toggleGuiDraw.set("show gui (G)", false));
	gui.add(doDrawCamBackground.set("DRAW SOURCE (C)", true));
	gui.add(showKinect.set("SHOW KINECT", true));
	gui.add(presence.set("presence", true));
	gui.add(minAreaPresIn.set("minAreaPresIn", 1000, 0, 5000));
	gui.add(minAreaPresOut.set("minAreaPresOut", 500, 0, 5000));

	gui.add(farThreshold.set("farThreshold", 2000, 0, 7000));
	gui.add(erode.set("erode", 0, 0, 40));
	gui.add(dilate.set("dilate", 0, 0, 40));
	gui.add(drawMode.set("draw mode", DRAW_COMPOSITE, DRAW_COMPOSITE, DRAW_MOUSE));
	drawMode.addListener(this, &ofApp::drawModeSetName);
	gui.add(drawName.set("MODE", "draw name"));

	gui.add(alphaNoneVel.set("alphaNoneVel", 0, 0, 5000));
	gui.add(alphaNone.set("alphaNone", 0, 0, 255));

	presence = false;
	alphaNone = 0;

	int guiColorSwitch = 0;
	ofColor guiHeaderColor[2];
	guiHeaderColor[0].set(160, 160, 80, 200);
	guiHeaderColor[1].set(80, 160, 160, 200);
	ofColor guiFillColor[2];
	guiFillColor[0].set(160, 160, 80, 200);
	guiFillColor[1].set(80, 160, 160, 200);

	gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
	gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
	guiColorSwitch = 1 - guiColorSwitch;
	gui.add(opticalFlow.parameters);

	gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
	gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
	guiColorSwitch = 1 - guiColorSwitch;
	gui.add(velocityMask.parameters);

	gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
	gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
	guiColorSwitch = 1 - guiColorSwitch;
	gui.add(fluidSimulation.parameters);

	gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
	gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
	guiColorSwitch = 1 - guiColorSwitch;
	gui.add(particleFlow.parameters);

	gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
	gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
	guiColorSwitch = 1 - guiColorSwitch;
	gui.add(mouseForces.leftButtonParameters);

	gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
	gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
	guiColorSwitch = 1 - guiColorSwitch;
	gui.add(mouseForces.rightButtonParameters);

	visualizeParameters.setName("visualizers");
	visualizeParameters.add(showScalar.set("show scalar", true));
	visualizeParameters.add(showField.set("show field", true));
	visualizeParameters.add(displayScalarScale.set("scalar scale", 0.15, 0.05, 1.0));
	displayScalarScale.addListener(this, &ofApp::setDisplayScalarScale);
	visualizeParameters.add(velocityFieldScale.set("velocity scale", 0.1, 0.0, 0.5));
	velocityFieldScale.addListener(this, &ofApp::setVelocityFieldScale);
	visualizeParameters.add(temperatureFieldScale.set("temperature scale", 0.1, 0.0, 0.5));
	temperatureFieldScale.addListener(this, &ofApp::setTemperatureFieldScale);
	visualizeParameters.add(pressureFieldScale.set("pressure scale", 0.02, 0.0, 0.5));
	pressureFieldScale.addListener(this, &ofApp::setPressureFieldScale);
	visualizeParameters.add(velocityLineSmooth.set("line smooth", false));
	velocityLineSmooth.addListener(this, &ofApp::setVelocityLineSmooth);

	gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
	gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
	guiColorSwitch = 1 - guiColorSwitch;
	gui.add(visualizeParameters);

	gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
	gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
	guiColorSwitch = 1 - guiColorSwitch;
	gui.add(velocityDots.parameters);

	// if the settings file is not present the parameters will not be set during this setup
	if (!ofFile("settings.xml"))
		gui.saveToFile("settings.xml");

	gui.loadFromFile("settings.xml");

	gui.minimizeAll();
	toggleGuiDraw = false;




}

//--------------------------------------------------------------
void ofApp::update() {
	
	didCamUpdate = false;

	deltaTime = ofGetElapsedTimef() - lastTime;
	lastTime = ofGetElapsedTimef();

	kinect.update();

	if (kinect.isFrameNew()) {
		didCamUpdate = true;


		kinPix = kinect.getDepthSource()->getPixels();
		//cout << " kinPix[200*500]" << int(kinPix[200 * 500]) << endl;
		for (int i = 0; i< grayPix.size(); i++) {
			grayPix[i] = 0;
			if (kinPix[i] < farThreshold &&  kinPix[i] != 0) {
				grayPix[i] = 255;
			}

		}
		
		grayImageIn.setFromPixels(grayPix);

		grayImageIn.mirror(mirrorV, mirrorH);

		grayImageThresh = grayImageIn;
		//int type = CV_THRESH_TOZERO;

		//cvThreshold(grayImageIn.getCvImage(), grayImageThresh.getCvImage(), farThreshold, 255, type);

		grayImageThresh.flagImageChanged();

		for (int i = 0; i < erode; i++) {
			grayImageThresh.erode();
		}
		for (int i = 0; i < dilate; i++) {
			grayImageThresh.dilate();
		}
		/////////////// SOUNDS \\\\\\\\\\\\\\\\

		grayImageKinectDiff = grayImageThresh;
		grayImageKinectDiff.absDiff(grayImageKinectOld);
		grayImageKinectOld = grayImageThresh;

		grayImageKinectDiff.threshold(100);

		kinectMovementSpeed = grayImageKinectDiff.countNonZeroInRegion(0, 0, grayImageKinectDiff.width, grayImageKinectDiff.height);




	}


	if (kinectMovementSpeed >0) {
		volume += (kinectMovementSpeed - volume) * volFade;
	}
	else if (kinectMovementSpeed <= 0 && prevMovSpd <= 0) {
		volume += (kinectMovementSpeed - volume) * volFadeOut;
	}


	int areaPres = grayImageThresh.countNonZeroInRegion(0, 0, grayImageThresh.width, grayImageThresh.height);
	if (areaPres < minAreaPresOut) {
		presence = false;
	}
	else if (areaPres > minAreaPresIn)
	{
		presence = true;
	}


	prevMovSpd = kinectMovementSpeed;


	sound.setVolume(volume * volScale);

	fluidSimulation.addVelocity(opticalFlow.getOpticalFlowDecay());
	fluidSimulation.addDensity(velocityMask.getColorMask());
	fluidSimulation.addTemperature(velocityMask.getLuminanceMask());

	mouseForces.update(deltaTime);

	for (int i = 0; i<mouseForces.getNumForces(); i++) {
		if (mouseForces.didChange(i)) {
			switch (mouseForces.getType(i)) {
			case FT_DENSITY:
				fluidSimulation.addDensity(mouseForces.getTextureReference(i), mouseForces.getStrength(i));
				break;
			case FT_VELOCITY:
				fluidSimulation.addVelocity(mouseForces.getTextureReference(i), mouseForces.getStrength(i));
				particleFlow.addFlowVelocity(mouseForces.getTextureReference(i), mouseForces.getStrength(i));
				break;
			case FT_TEMPERATURE:
				fluidSimulation.addTemperature(mouseForces.getTextureReference(i), mouseForces.getStrength(i));
				break;
			case FT_PRESSURE:
				fluidSimulation.addPressure(mouseForces.getTextureReference(i), mouseForces.getStrength(i));
				break;
			case FT_OBSTACLE:
				fluidSimulation.addTempObstacle(mouseForces.getTextureReference(i));
			default:
				break;
			}
		}
	}

	fluidSimulation.update();

	if (particleFlow.isActive()) {
		particleFlow.setSpeed(fluidSimulation.getSpeed());
		particleFlow.setCellSize(fluidSimulation.getCellSize());
		particleFlow.addFlowVelocity(opticalFlow.getOpticalFlow());
		particleFlow.addFluidVelocity(fluidSimulation.getVelocity());
		//		particleFlow.addDensity(fluidSimulation.getDensity());
		particleFlow.setObstacle(fluidSimulation.getObstacle());
	}
	particleFlow.update();

	if (didCamUpdate) {

		ofSetColor(255);


		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		cameraFbo.begin();

		if (presence) {
			if (alphaNone >0) {
				alphaNone -= deltaTime * alphaNoneVel;
			}
			if (alphaNone < 0) {
				alphaNone = 0;
			}
		}
		else {
			if (alphaNone < 255) {
				alphaNone += deltaTime * alphaNoneVel;
			}
			if (alphaNone > 255) {
				alphaNone = 255;
			}
		}

		if (alphaNone <255) {
			grayImageThresh.draw(0, 0);
		}

		if (alphaNone > 0) {
			ofEnableAlphaBlending();
			//ofPushMatrix();
			ofTranslate(-cos(lastTime * partNoneRotVel) * partNoneRotAmount, sin(lastTime * partNoneRotVel) * partNoneRotAmount);
			//ofTranslate(cameraFbo.getWidth() / 2, cameraFbo.getHeight() / 2);
			ofSetColor(255, alphaNone);

			//			partNone.draw(-1920 / 2, -1200 / 2, 1920, 1200);
			partNone.draw(0, 0, cameraFbo.getWidth(), cameraFbo.getHeight());
			ofSetColor(255);
			//	ofPopMatrix();
			ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		}
		cameraFbo.end();
		ofPopStyle();

		opticalFlow.setSource(cameraFbo.getTexture());

		// opticalFlow.update(deltaTime);
		// use internal deltatime instead
		opticalFlow.update();

		velocityMask.setDensity(cameraFbo.getTexture());
		velocityMask.setVelocity(opticalFlow.getOpticalFlow());
		velocityMask.update();

	}

	
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case 'l': gui.loadFromFile("settings.xml"); break;
	case 's': gui.saveToFile("settings.xml"); break;
	case 'G':
	case 'g': toggleGuiDraw = !toggleGuiDraw; break;
	case 'f':
	case 'F': doFullScreen.set(!doFullScreen.get()); break;
	case 'c':
	case 'C': doDrawCamBackground.set(!doDrawCamBackground.get()); break;

	case '1': drawMode.set(DRAW_COMPOSITE); break;
	case '2': drawMode.set(DRAW_PARTICLES); break;
		//case '9': drawMode.set(DRAW_FLUID_FIELDS); break;
	case '3': drawMode.set(DRAW_SOURCE); break;
	case '4': drawMode.set(DRAW_FLUID_VELOCITY); break;
		//case '5': drawMode.set(DRAW_FLUID_TEMPERATURE); break;
		//case '6': drawMode.set(DRAW_OPTICAL_FLOW); break;
		//case '7': drawMode.set(DRAW_FLOW_MASK); break;
		//case '8': drawMode.set(DRAW_MOUSE); break;

	case 'r':
	case 'R':
		fluidSimulation.reset();
		mouseForces.reset();
		break;


	default: break;
	}
}

//--------------------------------------------------------------
void ofApp::drawModeSetName(int &_value) {
	switch (_value) {
	case DRAW_COMPOSITE:		drawName.set("Composite      (1)"); break;
	case DRAW_PARTICLES:		drawName.set("Particles      (2)"); break;
		//case DRAW_FLUID_FIELDS:		drawName.set("Fluid Fields   (2)"); break;
		//case DRAW_FLUID_DENSITY:	drawName.set("Fluid Density  "); break;
	case DRAW_FLUID_VELOCITY:	drawName.set("Fluid Velocity (3)"); break;
		//case DRAW_FLUID_PRESSURE:	drawName.set("Fluid Pressure (4)"); break;
		//case DRAW_FLUID_TEMPERATURE:drawName.set("Fld Temperature(5)"); break;
		//case DRAW_FLUID_DIVERGENCE: drawName.set("Fld Divergence "); break;
		//case DRAW_FLUID_VORTICITY:	drawName.set("Fluid Vorticity"); break;
		//case DRAW_FLUID_BUOYANCY:	drawName.set("Fluid Buoyancy "); break;
		//case DRAW_FLUID_OBSTACLE:	drawName.set("Fluid Obstacle "); break;
		//case DRAW_OPTICAL_FLOW:		drawName.set("Optical Flow   (6)"); break;
		//case DRAW_FLOW_MASK:		drawName.set("Flow Mask      (7)"); break;
	case DRAW_SOURCE:			drawName.set("Source         (4)"); break;
		//case DRAW_MOUSE:			drawName.set("Left Mouse     (8)"); break;
		//case DRAW_VELDOTS:			drawName.set("VelDots        (0)"); break;
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofClear(0, 0);
	if (doDrawCamBackground.get())
		drawSource();


	if (!toggleGuiDraw) {
		ofHideCursor();
		drawParticles();
	}
	else {
		ofShowCursor();
		switch (drawMode.get()) {
		case DRAW_COMPOSITE: drawComposite(); break;
		case DRAW_PARTICLES: drawParticles(); break;
			//case DRAW_FLUID_FIELDS: drawFluidFields(); break;
			//case DRAW_FLUID_DENSITY: drawFluidDensity(); break;
		case DRAW_FLUID_VELOCITY: drawFluidVelocity(); break;
		case DRAW_SOURCE: drawSource(); break;
			//case DRAW_FLUID_PRESSURE: drawFluidPressure(); break;
			//case DRAW_FLUID_TEMPERATURE: drawFluidTemperature(); break;
			//case DRAW_FLUID_DIVERGENCE: drawFluidDivergence(); break;
			//case DRAW_FLUID_VORTICITY: drawFluidVorticity(); break;
			//case DRAW_FLUID_BUOYANCY: drawFluidBuoyance(); break;
			//case DRAW_FLUID_OBSTACLE: drawFluidObstacle(); break;
			//case DRAW_FLOW_MASK: drawMask(); break;
			//case DRAW_OPTICAL_FLOW: drawOpticalFlow(); break;

			//case DRAW_MOUSE: drawMouseForces(); break;
			//case DRAW_VELDOTS: drawVelocityDots(); break;
		}
		drawGui();
	}
	if (showKinect) {
		ofDrawRectangle(300 - 5, 200 - 5, 320 + 10, 240 + 10);
		kinect.getDepthSource()->draw(300, 200, 320, 240);
		ofDrawRectangle(700 - 5, 200 - 5, 320 + 10, 240 + 10);
		grayImageThresh.draw(700, 200, 320, 240);
		ofDrawRectangle(1100 - 5, 200 - 5, 320 + 10, 240 + 10);
		cameraFbo.draw(1100, 200, 320, 240);
		ofDrawRectangle(1500 - 5, 200 - 5, 320 + 10, 240 + 10);
		grayImageKinectDiff.draw(1500, 200, 320, 240);
		ofDrawBitmapStringHighlight("KINECT DEPTH", 300, 470);
		ofDrawBitmapStringHighlight("KINECT THRESHOLD", 700, 470);
		ofDrawBitmapStringHighlight("FBO FOR FLOW", 1100, 470);
		ofDrawBitmapStringHighlight("KINECT DIFFERENCE", 1500, 470);

	}
	//grayImageIn.draw(620, 200, 320, 240);
	//grayImageKinectDiff.draw(940, 200, 320, 240);
	//grayImageThresh.draw(1260, 200, 320, 240);
}

//--------------------------------------------------------------
void ofApp::drawComposite(int _x, int _y, int _width, int _height) {
	ofPushStyle();

	ofEnableBlendMode(OF_BLENDMODE_ADD);
	fluidSimulation.draw(_x, _y, _width, _height);

	ofEnableBlendMode(OF_BLENDMODE_ADD);
	if (particleFlow.isActive())
		particleFlow.draw(_x, _y, _width, _height);


	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawParticles(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);

	if (particleFlow.isActive())
		particleFlow.draw(_x, _y, _width, _height);


	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidFields(int _x, int _y, int _width, int _height) {
	ofPushStyle();

	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	pressureField.setPressure(fluidSimulation.getPressure());
	pressureField.draw(_x, _y, _width, _height);
	velocityTemperatureField.setVelocity(fluidSimulation.getVelocity());
	velocityTemperatureField.setTemperature(fluidSimulation.getTemperature());
	velocityTemperatureField.draw(_x, _y, _width, _height);
	temperatureField.setTemperature(fluidSimulation.getTemperature());

	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidDensity(int _x, int _y, int _width, int _height) {
	ofPushStyle();

	fluidSimulation.draw(_x, _y, _width, _height);

	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidVelocity(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	if (showScalar.get()) {
		ofClear(0, 0);
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		//	ofEnableBlendMode(OF_BLENDMODE_DISABLED); // altenate mode
		displayScalar.setSource(fluidSimulation.getVelocity());
		displayScalar.draw(_x, _y, _width, _height);
	}
	if (showField.get()) {
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		velocityField.setVelocity(fluidSimulation.getVelocity());
		velocityField.draw(_x, _y, _width, _height);
	}
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidPressure(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	ofClear(128);
	if (showScalar.get()) {
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		displayScalar.setSource(fluidSimulation.getPressure());
		displayScalar.draw(_x, _y, _width, _height);
	}
	if (showField.get()) {
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		pressureField.setPressure(fluidSimulation.getPressure());
		pressureField.draw(_x, _y, _width, _height);
	}
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidTemperature(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	if (showScalar.get()) {
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		displayScalar.setSource(fluidSimulation.getTemperature());
		displayScalar.draw(_x, _y, _width, _height);
	}
	if (showField.get()) {
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		temperatureField.setTemperature(fluidSimulation.getTemperature());
		temperatureField.draw(_x, _y, _width, _height);
	}
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidDivergence(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	if (showScalar.get()) {
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		displayScalar.setSource(fluidSimulation.getDivergence());
		displayScalar.draw(_x, _y, _width, _height);
	}
	if (showField.get()) {
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		temperatureField.setTemperature(fluidSimulation.getDivergence());
		temperatureField.draw(_x, _y, _width, _height);
	}
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidVorticity(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	if (showScalar.get()) {
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		displayScalar.setSource(fluidSimulation.getConfinement());
		displayScalar.draw(_x, _y, _width, _height);
	}
	if (showField.get()) {
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		ofSetColor(255, 255, 255, 255);
		velocityField.setVelocity(fluidSimulation.getConfinement());
		velocityField.draw(_x, _y, _width, _height);
	}
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidBuoyance(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	if (showScalar.get()) {
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		displayScalar.setSource(fluidSimulation.getSmokeBuoyancy());
		displayScalar.draw(_x, _y, _width, _height);
	}
	if (showField.get()) {
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		velocityField.setVelocity(fluidSimulation.getSmokeBuoyancy());
		velocityField.draw(_x, _y, _width, _height);
	}
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidObstacle(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	ofEnableBlendMode(OF_BLENDMODE_DISABLED);
	fluidSimulation.getObstacle().draw(_x, _y, _width, _height);
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawMask(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	ofEnableBlendMode(OF_BLENDMODE_DISABLED);
	velocityMask.draw(_x, _y, _width, _height);
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawOpticalFlow(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	if (showScalar.get()) {
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		displayScalar.setSource(opticalFlow.getOpticalFlowDecay());
		displayScalar.draw(0, 0, _width, _height);
	}
	if (showField.get()) {
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		velocityField.setVelocity(opticalFlow.getOpticalFlowDecay());
		velocityField.draw(0, 0, _width, _height);
	}
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawSource(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	ofEnableBlendMode(OF_BLENDMODE_DISABLED);
	cameraFbo.draw(_x, _y, _width, _height);
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawMouseForces(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	ofClear(0, 0);

	for (int i = 0; i<mouseForces.getNumForces(); i++) {
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		if (mouseForces.didChange(i)) {
			ftDrawForceType dfType = mouseForces.getType(i);
			if (dfType == FT_DENSITY)
				mouseForces.getTextureReference(i).draw(_x, _y, _width, _height);
		}
	}

	for (int i = 0; i<mouseForces.getNumForces(); i++) {
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		if (mouseForces.didChange(i)) {
			switch (mouseForces.getType(i)) {
			case FT_VELOCITY:
				velocityField.setVelocity(mouseForces.getTextureReference(i));
				velocityField.draw(_x, _y, _width, _height);
				break;
			case FT_TEMPERATURE:
				temperatureField.setTemperature(mouseForces.getTextureReference(i));
				temperatureField.draw(_x, _y, _width, _height);
				break;
			case FT_PRESSURE:
				pressureField.setPressure(mouseForces.getTextureReference(i));
				pressureField.draw(_x, _y, _width, _height);
				break;
			default:
				break;
			}
		}
	}

	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawVelocityDots(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	velocityDots.setVelocity(fluidSimulation.getVelocity());
	velocityDots.draw(_x, _y, _width, _height);
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawGui() {
	guiFPS = (int)(ofGetFrameRate() + 0.5);

	// calculate minimum fps
	deltaTimeDeque.push_back(deltaTime);

	while (deltaTimeDeque.size() > guiFPS.get())
		deltaTimeDeque.pop_front();

	float longestTime = 0;
	for (int i = 0; i<deltaTimeDeque.size(); i++) {
		if (deltaTimeDeque[i] > longestTime)
			longestTime = deltaTimeDeque[i];
	}

	guiMinFPS.set(1.0 / longestTime);


	ofPushStyle();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	gui.draw();

	// HACK TO COMPENSATE FOR DISSAPEARING MOUSE
	ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
	ofDrawCircle(ofGetMouseX(), ofGetMouseY(), ofGetWindowWidth() / 300.0);
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofDrawCircle(ofGetMouseX(), ofGetMouseY(), ofGetWindowWidth() / 600.0);
	ofPopStyle();
}
