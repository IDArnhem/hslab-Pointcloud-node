#include "alvinutil.h"

#include <string>
#include <array>

#include <librealsense2/rs.hpp>

namespace alvin {

    void enumerate() {
        rs2::context ctx;
        std::vector<rs2::pipeline>  pipelines;

        for(auto &&dev : ctx.query_devices()) {
            rs2::pipeline pipe(ctx);
            rs2::config cfg;
            cfg.enable_device(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
            pipe.start(cfg);
            pipelines.emplace_back(pipe);
        }
    } // enumarate

};
