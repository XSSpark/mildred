#pragma once

#define VIZ_DEBUG

#include <memory>

#include <ros/ros.h>
#include <std_msgs/ColorRGBA.h>
#include <std_msgs/Float64MultiArray.h>
#include <sensor_msgs/JointState.h>
#include <geometry_msgs/Point.h>
#include <visualization_msgs/Marker.h>

#include "../../mildred_core/include/mildred_core/mildred.h"
#include "../../../devel/include/mildred_core/RemoteControlMessage.h"

#include "mildred/Body.h"

namespace Mildred {

    class MildredControl {
    public:
        MildredControl();
        ~MildredControl() = default;

        bool init();
        void loop();

    private:
        ros::NodeHandle nodeHandle;

        ros::Subscriber controlSubscriber;
        ros::Subscriber jointStatesSubscriber;
        ros::Publisher targetJointPositionPublisher;
        std_msgs::Float64MultiArray targetJointPositionMessage;

        std::unique_ptr<Mildred::Body> body{nullptr};

        void controlMessageCallback(const mildred_core::RemoteControlMessage::ConstPtr& controlMessage);
        void jointsStatesCallback(const sensor_msgs::JointState::ConstPtr& jointStatesMessage);

        #ifdef VIZ_DEBUG
        ros::Publisher targetMarkersPublisher;
        #endif
    };

}