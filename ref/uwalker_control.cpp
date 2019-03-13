#include "uwalker_control.h"

int main( int argc, char **argv )
{
    ros::init( argc, argv, "uwalker_control" );
    ros::NodeHandle n;
    ros::Rate loop_rate( 150 );

    //Control subscriber, topic is publisher from the uwalker_remote_{remote_type} node
    controlSubscriber = n.subscribe( "/uwalker/control", 1, controlMessageCallback );

    //TODO: Gait change subscriber ??? What the fuck did I mean?

    //Temporary debug dispatcher and message setup
    legsJointsPositionsPublisher = n.advertise<uwalker::LegsJointsPositions>( "legs_joints_positions", 1 );

    //Visualize target in viz
    targetMarkersPublisher = n.advertise<visualization_msgs::Marker>( "visualization_marker", 1 );

    //Setup Body
    body = new UWalker::Body();

    //Get the robot description from the parameter server
    std::string robotDescriptionXML;
    if ( !n.getParam( "robot_description", robotDescriptionXML ) )
    {
        ROS_ERROR( "UWalkerControl::main() Failed to load the robot description XML from the parameter server" );
        return false;
    }

    //Initialize the URDF model from the XML
    urdf::Model model;
    if (!model.initString( robotDescriptionXML ) ) {
        ROS_ERROR( "UWalkerControl::main() Failed to initialize robot model" );
        return false;
    }

    //Setup body
    if ( !body->setup( model, "thorax", "foot_frame" ) )
    {
        ROS_ERROR( "UWalkerControl::main() Failed to setup body->" );
        return false;
    }

    //TODO: Select default gait
    //TODO: Wake up, starting pose

    int count = 0;
    while ( ros::ok() )
    {
        // TODO Stuff unrelated to the actual walk

        // NOTE Manual leg control

        // NOTE Gait switching

        /**
         * Tick the body loop
         * Gait calculations and body/leg IK all happens happens in this call.
         */
        body->tick();

#ifdef VIZ_DEBUG
        visualization_msgs::Marker marker;
        marker.header.frame_id = "base_link";
        marker.header.stamp = ros::Time::now();
        marker.ns = "uwalker";
        marker.id = 0;
        marker.type = visualization_msgs::Marker::POINTS;
        marker.action = visualization_msgs::Marker::ADD;
        marker.scale.x = 0.005;
        marker.scale.y = 0.005;
        marker.scale.z = 0.005;
        marker.color.a = 1.0;
        marker.color.r = 1.0;
        marker.color.g = 0.5;
        marker.color.b = 0.0;
#endif

        //Send the joint positions values to the actuators
        uwalker::LegsJointsPositions legsJointsPositionsMsg;
        for ( unsigned int i = 0; i < body->LEG_COUNT; i++ )
        {
            for ( unsigned int j = 0; j < body->leg[i]->JOINT_COUNT; j++ )
            {
                legsJointsPositionsMsg.leg[i].joint[j] = body->leg[i]->joint[j].targetPosition;
            }

#ifdef VIZ_DEBUG
            //Add point to marker points
            geometry_msgs::Point point;
            point.x = body->leg[i]->targetPosition.x();
            point.y = body->leg[i]->targetPosition.y();
            point.z = body->leg[i]->targetPosition.z();
            marker.points.push_back( point );
#endif
        }

        //TODO Joint limits (not necessarily in here)

        //Publish final joints positions
        legsJointsPositionsPublisher.publish( legsJointsPositionsMsg );

#ifdef VIZ_DEBUG
        targetMarkersPublisher.publish( marker );
#endif

        ros::spinOnce();
        loop_rate.sleep();
        ++count;
    }

    return 0;
}

void controlMessageCallback( uwalker::RemoteControl controlMessage )
{
    body->velocity = KDL::Vector2( controlMessage.velocity.x, controlMessage.velocity.y );
    body->frame = KDL::Frame(
            KDL::Rotation::RPY(
                    controlMessage.rotation.y * ( M_PI / 8 ), //TODO: Dynamically adjust maximum rotation
                    controlMessage.rotation.x * ( M_PI / 8 ), //TODO: Dynamically adjust maximum rotation
                    controlMessage.rotation.z * ( M_PI / 8 )  //TODO: Dynamically adjust maximum rotation
            ),
            KDL::Vector(
                    controlMessage.position.x / 10.00, //TODO: Dynamically adjust maximum position
                    controlMessage.position.y / 10.00, //TODO: Dynamically adjust maximum position
                    controlMessage.position.z / 10.00  //TODO: Dynamically adjust maximum position
            )
    );
}