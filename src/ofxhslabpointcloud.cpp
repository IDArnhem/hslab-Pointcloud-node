#include "ofxhslabpointcloud.h"

/*
typedef struct PointcloudData {
    unsigned long seq; // frame sequence number
    unsigned long count; // point count
    unsigned char *color; // r,g,b data
    float *vertices; // x, y, z data
    float *texture; // u,v data
} tPointcloudData;
*/

ofxHslabPointcloud::ofxHslabPointcloud()
{
    pcdata.count = 0;
    pcdata.width = 0;
    pcdata.height = 0;
}

ofxHslabPointcloud::~ofxHslabPointcloud() {
}

void ofxHslabPointcloud::allocate(unsigned long dim) {
    if(dim == pcdata.count ) { return; }
    // reallocate only if dimensions are different
    pcdata.count = dim;
    pcdata.color = new unsigned char[dim*3];
    pcdata.vertices = new float[dim*3];
    pcdata.texture = new float[dim*2];

    ofLogNotice() << "Allocating " << pcdata.count << " points in pointcloud";
    ofLogNotice() << 3*dim*sizeof(unsigned char) << " size of color buffer";
    ofLogNotice() << 3*dim*sizeof(float) << " size of vertex bufffer";
    ofLogNotice() << 2*dim*sizeof(float) << " size of texture bufffer";
}

// build the pointcloud data structure from RealSense API buffers
void ofxHslabPointcloud::fromRSBuffers() {
}

// reconstruct the pointcloud data structure from UDP datagram buffer
void ofxHslabPointcloud::fromUDPBuffer() {
}

//ofMesh &ofxHslabPointcloud::toOFMesh() {
//}
