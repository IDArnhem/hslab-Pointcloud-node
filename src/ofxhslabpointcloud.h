#ifndef OFXHSLABPOINTCLOUD_H
#define OFXHSLABPOINTCLOUD_H

#include "ofMain.h"

typedef struct PointcloudData {
    unsigned long seq; // frame sequence number
    unsigned long count; // point count
    int width;
    int height;
    unsigned char *color; // r,g,b data
    float *vertices; // x, y, z data
    float *texture; // u,v data
} tPointcloudData;

class ofxHslabPointcloud
{
public:
    ofxHslabPointcloud();
    ~ofxHslabPointcloud();

    void allocate(unsigned long dim);

    // build the pointcloud data structure from RealSense API buffers
    void fromRSBuffers();

    // reconstruct the pointcloud data structure from UDP datagram buffer
    void fromUDPBuffer();

    tPointcloudData pcdata;
};

#endif // OFXHSLABPOINTCLOUD_H
