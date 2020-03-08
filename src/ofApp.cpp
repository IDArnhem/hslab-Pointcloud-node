#include "ofApp.h"

// ////////////////////////////////////////////////////////////////
void ofApp::setup() {
    ofSetFrameRate(30);
    ofSetVerticalSync(true);
    ofSetLogLevel(OF_LOG_NOTICE);

    initPlots();

    acquiring = false;

    // initialize realsense camera stuff
    rs2::context ctx;
    std::vector<rs2::pipeline>  pipelines;

    // enumerate available devices
    for(auto &&dev : ctx.query_devices()) {
        rs2::config cfg;
        ofLogNotice() << "RealSense device found with ID " << dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
        cfg.enable_device( dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) );
        acquiring = true; // seems like we have a data source
        pipe.start( cfg ); // open it
        pipelines.push_back(pipe); // keep track of this pipeline
    }

    // show result of camera scan
    if(acquiring) {
        ofLogNotice() << "Number of camera pipelines " << pipelines.size();
    } else {
        ofLogWarning() << "No RealSense cameras were found! make sure you have one plugged in";
    }
}

void ofApp::initPlots() {
    plotCooking = new ofxHistoryPlot( NULL, "cooking time", 200, false);
    plotCooking->setRange(0, 500);
    plotCooking->setColor( ofColor(255,255,0) );
    plotCooking->setShowNumericalInfo(true);
    plotCooking->setRespectBorders(true);
    plotCooking->setLineWidth(2);
    plotCooking->setBackgroundColor(ofColor(0,0,0,128));
}

// ////////////////////////////////////////////////////////////////
void ofApp::update() {
    if(!acquiring) {
        // do nothing
        return;
    } else {
        pcAcquire();
    }
}

void ofApp::pcDispatch() {
    cout << "Sending" << endl;
    cout << "=======" << endl;
    cout << endl;
    //send a DataGram (i.e. a message)
    {
        //make the DataGram
        auto dataGram = make_shared<ofxAsio::UDP::DataGram>();
        dataGram->setEndPoint(ofxAsio::UDP::EndPoint("127.0.0.1", 4444)); //--set where the message will be sent to

        auto & message = dataGram->getMessage();
        message.set("I am the message."); //--set the message content

        //send the DataGram
        this->sender.send(dataGram);

        auto & m = dataGram->getMessage();
        cout << m.getMessageString() << endl;
        cout << "Sent to : " << dataGram->getEndPoint() << endl;
    }
    cout << endl;
}

void ofApp::buildMesh(ofxHslabPointcloud &cloud) {
    mesh.clear();
    if(cloud.pcdata.count !=0 ){
        const float *vs = cloud.pcdata.vertices;
        for(int i = 0; i < cloud.pcdata.count; i++){
            if(vs[i+2]){
                int x = static_cast<int>(cloud.pcdata.texture[i] * cloud.pcdata.width);
                int y = static_cast<int>(cloud.pcdata.texture[i+1] * cloud.pcdata.height);

                if (y < 0 || y >= cloud.pcdata.height) { continue; }
                if (x < 0 || x >= cloud.pcdata.width) { continue; }
                int cloc = (y * cloud.pcdata.width + x) * 3;

                //const rs2::vertex v = vs[i];
                glm::vec3 v3(vs[i*3],vs[(i*3)+1],vs[(i*3)+2]);
                mesh.addVertex(v3);

                unsigned char r = cloud.pcdata.color[i*3];
                unsigned char g = cloud.pcdata.color[(i*3) + 1];
                unsigned char b = cloud.pcdata.color[(i*3) + 2];

                mesh.addColor(ofColor(r, g, b));
            } // if
        } // for
    } // if
}

void ofApp::pcAcquire() {
    ofResetElapsedTimeCounter();

    // Get depth data from camera
    auto frames = pipe.wait_for_frames();
    auto color = frames.get_color_frame();
    auto depth = frames.get_depth_frame();

    // tell pointcloud object to map colors from video frame
    pc.map_to(color);
    points = pc.calculate(depth);

    auto pcver = points.get_vertices();              // get vertices
    auto pctex = points.get_texture_coordinates();
    const unsigned char* pccolor = static_cast<const unsigned char*>(color.get_data());
    unsigned long n = points.size();

    // ----------------------------------------------------------------
    // create intermediate data structure
    pcloud.pcdata.width = color.get_width();
    pcloud.pcdata.height = color.get_height();

    // allocate intermediary data structure
    pcloud.allocate(n);

    std::memcpy(pcloud.pcdata.color, pccolor, 3*n*sizeof(unsigned char));
    std::memcpy(pcloud.pcdata.vertices, pcver, 3*n*sizeof(float));
    std::memcpy(pcloud.pcdata.texture, pctex, 2*n*sizeof(float));
    // ----------------------------------------------------------------

    buildMesh( pcloud );
/*
    int width = color.get_width();
    int height = color.get_height();

    // Create oF mesh
    mesh.clear();
    if(n!=0){
        const rs2::vertex * vs = points.get_vertices();
        for(int i=0; i<n; i++){
            if(vs[i].z){
                int x = static_cast<int>(pctex[i].u * width);
                int y = static_cast<int>(pctex[i].v * height);

                if (y < 0 || y >= height) { continue; }
                if (x < 0 || x >= width) { continue; }
                int cloc = (y * width + x) * 3;

                const rs2::vertex v = vs[i];
                glm::vec3 v3(v.x,v.y,v.z);
                mesh.addVertex(v3);

                unsigned char r = pccolor[cloc];
                unsigned char g = pccolor[cloc + 1];
                unsigned char b = pccolor[cloc + 2];

                mesh.addColor(ofColor(r, g, b));
                            //ofFloatColor(0,0,ofMap(v.z, 2, 6, 1, 0), 0.8));
            } // if
        } // for
    } // if
*/

    long ns = ofGetElapsedTimeMillis();
    //ofLogNotice() << "Cooking time in millis " << ns;
    plotCooking->update( ns );
    /*
     * With buffer copies and mesh conversion:
     * Cooking time in millis 65
     *
     * Using the RS data structures straight-up, no copies:
     * Cooking time in millis 50
     */
}

// ////////////////////////////////////////////////////////////////
void ofApp::draw() {
    float scale = 0.5f;

    ofBackground(200);
    cam.begin();
    float s = 200;
    ofScale(s,-s,-s);
    ofDrawAxis(1);

    ofPushMatrix();
    ofTranslate(0, 1, 0);
    ofRotateZDeg(90);
    ofSetColor(0,200);
    ofDrawGridPlane(1, 5, true);
    ofPopMatrix();

    mesh.drawVertices();

    cam.end();

    plotCooking->draw(10, 10, 240, 100);
//    plotBufferSize->draw(10, 110, 240, 100);
}

