#include "MildredControl.h"

namespace Mildred {

    MildredControl::MildredControl() :
        nodeHandle(),
        body(std::make_unique<Mildred::Body>()) {

        controlSubscriber            = nodeHandle.subscribe("control", 1, &MildredControl::controlMessageCallback, this);
        jointStatesSubscriber        = nodeHandle.subscribe("joint_states", 1, &MildredControl::jointsStatesCallback, this);
        targetJointPositionPublisher = nodeHandle.advertise<std_msgs::Float64MultiArray>("joint_position_controller/command", 1);

        #ifdef VIZ_DEBUG
        targetMarkersPublisher = nodeHandle.advertise<visualization_msgs::Marker>("visualization_marker", 1);
        #endif
    }

    bool MildredControl::init() {
        auto model = std::make_shared<urdf::Model>();
        if (!model->initParam("robot_description")) {
            ROS_ERROR("Could not generate robot model");
            return false;
        }

        ROS_INFO("Loaded %s model", model->getName().c_str());

        if (!body->setup(model, LEG_COUNT, "foot_frame")) {
            ROS_ERROR("Failed to setup body");
            return false;
        }

        targetJointPositionMessage.data.resize(LEG_COUNT * DOF);

        return true;
    }

    void MildredControl::loop() {
        body->tick();

        #ifdef VIZ_DEBUG
        visualization_msgs::Marker marker;
        marker.header.frame_id = "base_link";
        marker.header.stamp    = ros::Time::now();
        marker.ns              = nodeHandle.getNamespace();
        marker.id              = 0;
        marker.type            = visualization_msgs::Marker::POINTS;
        marker.action          = visualization_msgs::Marker::ADD;
        marker.scale.x         = 0.01;
        marker.scale.y         = 0.01;
        marker.scale.z         = 0.01;
        marker.color.a         = 1.0;
        marker.color.r         = 1.0;
        marker.color.g         = 0.5;
        marker.color.b         = 0.0;
        #endif

        uint8_t count = 0;

        for (auto const &leg:body->legs) {
            for (auto const &joint:leg->joints) {
                targetJointPositionMessage.data[count] = joint.targetPosition;
                count++;
            }

            #ifdef VIZ_DEBUG
            geometry_msgs::Point point;
            point.x = leg->targetPosition.x();
            point.y = leg->targetPosition.y();
            point.z = leg->targetPosition.z();
            marker.points.push_back(point);
            #endif
            //break;
        }

        targetJointPositionPublisher.publish(targetJointPositionMessage);

        #ifdef VIZ_DEBUG
        targetMarkersPublisher.publish(marker);
        #endif
    }

    void MildredControl::controlMessageCallback(const mildred_core::RemoteControlMessage::ConstPtr &controlMessage) {
        body->velocity = KDL::Vector2(controlMessage->velocity.x, controlMessage->velocity.y);
        //body->frame    = KDL::Frame(
        //    KDL::Rotation::RPY(
        //        controlMessage->rotation.y * (M_PI / 8), //TODO: Dynamically adjust maximum rotation
        //        controlMessage->rotation.x * (M_PI / 8), //TODO: Dynamically adjust maximum rotation
        //        controlMessage->rotation.z * (M_PI / 8)  //TODO: Dynamically adjust maximum rotation
        //    ),
        //    KDL::Vector(
        //        controlMessage->position.x / 10.00, //TODO: Dynamically adjust maximum position
        //        controlMessage->position.y / 10.00, //TODO: Dynamically adjust maximum position
        //        controlMessage->position.z / 10.00  //TODO: Dynamically adjust maximum position
        //    )
        //);
    }

    /**
     * Callback getting the joint's current position from the jointStates messages
     */
    void MildredControl::jointsStatesCallback(const sensor_msgs::JointState::ConstPtr &jointStatesMessage) {
        if (!body) {
            ROS_WARN("Received joint states before having a body");
            return;
        }
        body->setJointState(jointStatesMessage);
    }

}