#pragma once

#include "ofMain.h"
#include "ofxFlowTools.h"
#include "ofxKinectForWindows2.h"
#include "ofxGui.h"
#include "ofxOpenCv.h"

using namespace flowTools;

enum drawModeEnum {
	DRAW_COMPOSITE = 0,
	DRAW_FLUID_DENSITY,
	DRAW_PARTICLES,
	DRAW_VELDOTS,
	DRAW_FLUID_FIELDS,
	DRAW_FLUID_VELOCITY,
	DRAW_FLUID_PRESSURE,
	DRAW_FLUID_TEMPERATURE,
	DRAW_FLUID_DIVERGENCE,
	DRAW_FLUID_VORTICITY,
	DRAW_FLUID_BUOYANCY,
	DRAW_FLUID_OBSTACLE,
	DRAW_FLOW_MASK,
	DRAW_OPTICAL_FLOW,
	DRAW_SOURCE,
	DRAW_MOUSE
};

class ofApp : public ofBaseApp {
public:
	void	setup();
	void	update();
	void	draw();


	ofxKFW2::Device kinect;
	ofShortPixels kinPix;
	ofPixels grayPix;

	int kinectWidth = 512;
	int kinectHeight = 424;


	ofImage partNone;

	ofParameter<float> partNoneRotVel;
	ofParameter<float> partNoneRotAmount;





	////////// SOUNDS \\\\\\\\\
		    
	ofSoundPlayer  background;
	ofSoundPlayer  sound;

	ofParameter<float> volFade;

	ofParameter<int> soundMoveScaleDown;
	ofParameter<float> soundMoveSpeed;


	ofParameter<float> threshMoveAudioMin;
	ofParameter<float> threshMoveAudioMax;


	ofParameter<float> soundVol;
	ofParameter<float> soundVolMove;

	ofParameter<float> backgroundVol;

	ofParameter<int> kinectDiffSkip;
	int kinectDiffCount;

	//////////////\\\\\\\\\\\\\
		
	ofParameter<bool> mirrorX;
	ofParameter<bool> mirrorY;


	ofxCvColorImage colorImg;

	ofxCvGrayscaleImage grayImageIn; // grayscale depth image

	ofxCvGrayscaleImage grayImageKinectOld; //
	ofxCvGrayscaleImage grayImageKinectDiff; //


	ofxCvGrayscaleImage grayImageThresh; // the far thresholded image


	ftFbo cameraFbo;
	bool  didCamUpdate;
	ofParameter<bool>   presence, showKinect;

	ofParameter<int>	minAreaPresIn;
	ofParameter<int>	minAreaPresOut;



	ofParameter<int>	farThreshold;

	ofParameter<float>	alphaNoneVel;
	ofParameter<float> alphaNone;

	ofParameter<int>	erode;
	ofParameter<int> dilate;


	ofParameter<int> cropLeft, cropRight;
	ofParameter<bool> showCrop;
	


	int screenshotCount;
	ofImage screenshot;

	//////---------------------------------------------------------------------------




	// Time
	float				lastTime;
	float				deltaTime;

	// FlowTools
	int					flowWidth;
	int					flowHeight;
	int					drawWidth;
	int					drawHeight;

	ftOpticalFlow		opticalFlow;
	ftVelocityMask		velocityMask;
	ftFluidSimulation	fluidSimulation;
	ftParticleFlow		particleFlow;

	ftVelocitySpheres	velocityDots;

	// MouseDraw
	ftDrawMouseForces	mouseForces;

	// Visualisations
	ofParameterGroup	visualizeParameters;
	ftDisplayScalar		displayScalar;
	ftVelocityField		velocityField;
	ftTemperatureField	temperatureField;
	ftPressureField		pressureField;
	ftVTField			velocityTemperatureField;

	ofParameter<bool>	showScalar;
	ofParameter<bool>	showField;
	ofParameter<float>	displayScalarScale;
	void				setDisplayScalarScale(float& _value) { displayScalar.setScale(_value); }
	ofParameter<float>	velocityFieldScale;
	void				setVelocityFieldScale(float& _value) { velocityField.setVelocityScale(_value); velocityTemperatureField.setVelocityScale(_value); }
	ofParameter<float>	temperatureFieldScale;
	void				setTemperatureFieldScale(float& _value) { temperatureField.setTemperatureScale(_value); velocityTemperatureField.setTemperatureScale(_value); }
	ofParameter<float>	pressureFieldScale;
	void				setPressureFieldScale(float& _value) { pressureField.setPressureScale(_value); }
	ofParameter<bool>	velocityLineSmooth;
	void				setVelocityLineSmooth(bool& _value) { velocityField.setLineSmooth(_value); velocityTemperatureField.setLineSmooth(_value); }

	// GUI
	ofxPanel			gui;

	ofxPanel			guiEffect;

	void				setupGui();
	void				keyPressed(int key);
	void				drawGui();
	ofParameter<bool>	toggleGuiDraw;
	ofParameter<bool>	toggleGuiEffectDraw;

	ofParameter<float>	guiFPS;
	ofParameter<float>	guiMinFPS;
	deque<float>		deltaTimeDeque;
	ofParameter<bool>	doFullScreen;
	void				setFullScreen(bool& _value) { ofSetFullscreen(_value); }

	// DRAW
	ofParameter<bool>	doDrawCamBackground;

	ofParameter<int>	drawMode;
	void				drawModeSetName(int& _value);
	ofParameter<string> drawName;

	void				drawComposite() { drawComposite(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawComposite(int _x, int _y, int _width, int _height);
	void				drawParticles() { drawParticles(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawParticles(int _x, int _y, int _width, int _height);
	void				drawFluidFields() { drawFluidFields(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawFluidFields(int _x, int _y, int _width, int _height);
	void				drawFluidDensity() { drawFluidDensity(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawFluidDensity(int _x, int _y, int _width, int _height);
	void				drawFluidVelocity() { drawFluidVelocity(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawFluidVelocity(int _x, int _y, int _width, int _height);
	void				drawFluidPressure() { drawFluidPressure(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawFluidPressure(int _x, int _y, int _width, int _height);
	void				drawFluidTemperature() { drawFluidTemperature(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawFluidTemperature(int _x, int _y, int _width, int _height);
	void				drawFluidDivergence() { drawFluidDivergence(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawFluidDivergence(int _x, int _y, int _width, int _height);
	void				drawFluidVorticity() { drawFluidVorticity(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawFluidVorticity(int _x, int _y, int _width, int _height);
	void				drawFluidBuoyance() { drawFluidBuoyance(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawFluidBuoyance(int _x, int _y, int _width, int _height);
	void				drawFluidObstacle() { drawFluidObstacle(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawFluidObstacle(int _x, int _y, int _width, int _height);
	void				drawMask() { drawMask(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawMask(int _x, int _y, int _width, int _height);
	void				drawOpticalFlow() { drawOpticalFlow(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawOpticalFlow(int _x, int _y, int _width, int _height);
	void				drawSource() { drawSource(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawSource(int _x, int _y, int _width, int _height);
	void				drawMouseForces() { drawMouseForces(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawMouseForces(int _x, int _y, int _width, int _height);

	void				drawVelocityDots() { drawVelocityDots(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
	void				drawVelocityDots(int _x, int _y, int _width, int _height);
};
