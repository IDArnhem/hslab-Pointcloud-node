#pragma once

#include "ofMain.h"
#include "ofxAsio.h"
#include "ofxHistoryPlot.h"
#include "ofxhslabpointcloud.h"
#include "ofxRealsense.h"
#include "ofxAvahiClient.h"
#include "snappy.h"

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void exit();

    string getHostname();

    void initPlots();

    // point cloud functionalities
    void pcAcquire();
    void pcDispatch();
    void pcCompress();

    void buildMesh(ofxHslabPointcloud &cloud);

    //void populateMesh(ofxHslabPointcloud &c);

private:
    rs2::pipeline pipe;
    rs2::device device;

    rs2::points points;
    rs2::pointcloud pc;

    bool acquiring;

    ofVboMesh mesh;
    ofEasyCam cam;

    ofxHslabPointcloud pcloud;

    ofxAsio::UDP::Socket sender;
    shared_ptr<ofxAsio::UDP::Server> receiver;

    ofxHistoryPlot *plotCooking;
    ofxHistoryPlot *plotRatio;

    ofxAvahiClientService mdns;

    ofMutex lock;
};
