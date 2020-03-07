#include "ofApp.h"

// ////////////////////////////////////////////////////////////////
void ofApp::setup() {
    ofSetFrameRate(30);
    ofSetVerticalSync(true);
    ofSetLogLevel(OF_LOG_NOTICE);

    acquiring = false;

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

void ofApp::pcAcquire() {
    // Get depth data from camera
    auto frames = pipe.wait_for_frames();
    auto color = frames.get_color_frame();
    auto depth = frames.get_depth_frame();

    // tell pointcloud object to map colors from video frame
    pc.map_to(color);
    points = pc.calculate(depth);

    int width = color.get_width();
    int height = color.get_height();

    auto pcver = points.get_vertices();              // get vertices
    auto pctex = points.get_texture_coordinates();
    const unsigned char* pccolor = static_cast<const unsigned char*>(color.get_data());

    // int colorLocation = (y * width + x) * 3;
    // unsigned char color[3] = {colorData[colorLocation], colorData[colorLocation +1], colorData[colorLocation +2]}; // RGB
/*
    for (int i = 0; i < points.size(); i++) {
        int x = static_cast<int>(pctex[i].u * width);
        int y = static_cast<int>(pctex[i].v * height);

        if (y < 0 || y >= height) { continue; }
        if (x < 0 || x >= width) { continue; }

//        if (pcver[i].z <= 0 || pcver[i].z > maxAcceptDistance) {
//            // filter out values further than this value
//            continue;
//        }

        int cloc = (y * width + x) * 3;
        unsigned char color[3] = {color[cloc], color[cloc+1], color[cloc+2]};

        float coordinates[3] = { pcver[i].x, pcver[i].y, pcver[i].z };
    }
*/

    /*
    auto pcver = points.get_vertices();              // get vertices
    auto pctex = points.get_texture_coordinates();
    const unsigned char* pccolor = static_cast<const unsigned char*>(color.get_data());
    for (int i = 0; i < rs_points.size(); i++) {
        int x = static_cast<int>(textureCoordinates[i].u * width);
        int y = static_cast<int>(textureCoordinates[i].v * height);
        int colorLocation = y * color_intrinsic.width + x;
        float px = rs_vertices[i].x;
        float py = rs_vertices[i].y;
        float pz = rs_vertices[i].z;
        unsigned char r = colorData[colorLoation];
        unsigned char g = colorData[colorLocation + 1];
        unsigned char b = colorData[colorLocation + 2];
        save(px, py, pz, r, g, b);
    }
    */

    /*
    auto vertices = points.get_vertices();              // get vertices
    auto tex_coords = points.get_texture_coordinates(); // and texture coordinates
    for (int i = 0; i < points.size(); i++)
    {
        if (vertices[i].z)
        {
            // upload the point and texture coordinates only for points we have depth data for
            glVertex3fv(vertices[i]);
            glTexCoord2fv(tex_coords[i]);
        }
    }
    */
    unsigned long n = points.size();

    pcloud.pcdata.width = color.get_width();
    pcloud.pcdata.height = color.get_height();

    // allocate intermediary data structure
    pcloud.allocate(n);

    std::memcpy(pcloud.pcdata.color, pccolor, 3*n*sizeof(unsigned char));
    std::memcpy(pcloud.pcdata.vertices, pcver, 3*n*sizeof(float));
    std::memcpy(pcloud.pcdata.texture, pctex, 2*n*sizeof(float));

    mesh.clear();
    if(pcloud.pcdata.count !=0 ){
        const float *vs = pcloud.pcdata.vertices;
        for(int i = 0; i < pcloud.pcdata.count; i++){
            if(vs[i+2]){
                int x = static_cast<int>(pcloud.pcdata.texture[i] * pcloud.pcdata.width);
                int y = static_cast<int>(pcloud.pcdata.texture[i+1] * pcloud.pcdata.height);

                if (y < 0 || y >= pcloud.pcdata.height) { continue; }
                if (x < 0 || x >= pcloud.pcdata.width) { continue; }
                int cloc = (y * pcloud.pcdata.width + x) * 3;

                //const rs2::vertex v = vs[i];
                glm::vec3 v3(vs[i*3],vs[(i*3)+1],vs[(i*3)+2]);
                mesh.addVertex(v3);

                unsigned char r = pcloud.pcdata.color[i*3];
                unsigned char g = pcloud.pcdata.color[(i*3) + 1];
                unsigned char b = pcloud.pcdata.color[(i*3) + 2];

                mesh.addColor(ofColor(r, g, b));
                            //ofFloatColor(0,0,ofMap(v.z, 2, 6, 1, 0), 0.8));
            } // if
        } // for
    } // if

    /*
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
}

