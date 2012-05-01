#include "ros/roslush.h"
#include "sensor_msgs/Image.h"
#include "geometry_msgs/Twist.h"
#include "geometry_msgs/PoseWithCovarianceStamped.h"
#include "nav_msgs/Odometry.h"
#include "sensor_msgs/PointCloud2.h"
#include "sensor_msgs/Imu.h"
#include <cstring>

/// <summary> OpenNI camera depth raw subscriber. </summary>
struct RosLushCameraDepthImageRaw
{
  enum { queue_size = 1, };
  typedef sensor_msgs::Image msg_type;
  typedef int_least16_t lush_convert_type;
  inline static const char* TopicStr()
  {
    return "camera/depth/image_raw";
  }
  inline static void CopyMsgData(const msg_type::ConstPtr& msg,
                                 lush_convert_type* data)
  {
    const int32_t dataSize = static_cast<int32_t>(msg->data.size());
    memcpy(data, &msg->data[0], dataSize);
  }
};
typedef RosSubscriber<RosLushCameraDepthImageRaw> CameraDepthImageRawSubscriber;

template <typename Vector3>
inline void CopyVector3(const Vector3& src, double* dst)
{
  dst[0] = src.x; dst[1] = src.y; dst[2] = src.z;
}

template <typename Vector4>
inline void CopyVector4(const Vector4& src, double* dst)
{
  dst[0] = src.x; dst[1] = src.y; dst[2] = src.z; dst[3] = src.w;
}

/// <summary> Turtlebot odometry subscriber. </summary>
struct RosLushOdom
{
  struct OdomData
  {
    OdomData(double* p_pos, double* p_orien, double* p_cov,
             double* t_lin, double* t_ang, double* t_cov)
      : pose_position(p_pos), pose_orientation(p_orien), pose_covariance(p_cov),
        twist_linear(t_lin), twist_angular(t_ang), twist_covariance(t_cov)
    {}
    double *pose_position, *pose_orientation, *pose_covariance;
    double *twist_linear, *twist_angular, *twist_covariance;
  };
  enum { queue_size = 5, };
  typedef nav_msgs::Odometry msg_type;
  typedef OdomData lush_convert_type;
  inline static const char* TopicStr()
  {
    return "/odom";
  }
  inline static void CopyMsgData(const msg_type::ConstPtr& msg,
                                 lush_convert_type* data)
  {
    CopyVector3(msg->twist.twist.linear, data->twist_linear);
    CopyVector3(msg->twist.twist.angular, data->twist_angular);
    memcpy(data->twist_covariance, &msg->twist.covariance[0],
           msg->twist.covariance.size() * sizeof(msg->twist.covariance[0]));
    CopyVector3(msg->pose.pose.position, data->pose_position);
    CopyVector4(msg->pose.pose.orientation, data->pose_orientation);
    memcpy(data->pose_covariance, &msg->pose.covariance[0],
           msg->pose.covariance.size() * sizeof(msg->pose.covariance[0]));
  }
};
typedef RosSubscriber<RosLushOdom> OdomSubscriber;

/// <summary> Turtlebot robot_pose_ekf subscriber. </summary>
struct RosLushRobotPoseEkf
{
  struct RobotPoseEkfData
  {
    RobotPoseEkfData(double* p_pos, double* p_orien, double* p_cov)
      : pose_position(p_pos), pose_orientation(p_orien), pose_covariance(p_cov)
    {}
    double *pose_position, *pose_orientation, *pose_covariance;
  };
  enum { queue_size = 5, };
  typedef geometry_msgs::PoseWithCovarianceStamped msg_type;
  typedef RobotPoseEkfData lush_convert_type;
  inline static const char* TopicStr()
  {
    return "/robot_pose_ekf/odom_combined";
  }
  inline static void CopyMsgData(const msg_type::ConstPtr& msg,
                                 lush_convert_type* data)
  {
    CopyVector3(msg->pose.pose.position, data->pose_position);
    CopyVector4(msg->pose.pose.orientation, data->pose_orientation);
    memcpy(data->pose_covariance, &msg->pose.covariance[0],
           msg->pose.covariance.size() * sizeof(msg->pose.covariance[0]));
  }
};
typedef RosSubscriber<RosLushRobotPoseEkf> RobotPoseEkfSubscriber;

/// <summary> Point cloud subscriber. </summary>
struct RosLushCameraDepthPoints
{
  struct CloudData
  {
    CloudData(float* data_, int* numPoints_)
      : data(data_),
        numPoints(numPoints_)
    {}
    float* data;
    int* numPoints;
  };
  enum { queue_size = 1, };
  typedef sensor_msgs::PointCloud2 msg_type;
  typedef CloudData lush_convert_type;
  inline static const char* TopicStr()
  {
    return "camera/depth/points";
  }
  inline static void CopyMsgData(const msg_type::ConstPtr& msg,
                                 lush_convert_type* data)
  {
    enum { Float32XYZPointSize = sizeof(float) * 3, };
    // Collect parameters.
    uint8_t* dataDst = reinterpret_cast<uint8_t*>(data->data);
    int& numPoints = *data->numPoints;
    numPoints = 0;
    // Copy all of the points (XYZ) to the data array.
    const int pointStep = msg->point_step;
    const int dataLength = msg->height * msg->row_step;
    const uint8_t* dataSrc = &msg->data[0];
    const uint8_t* const dataSrcEnd = dataSrc + dataLength;
    for (; dataSrc < dataSrcEnd; dataSrc += pointStep)
    {
      // Ignore bad points.
      if (!std::isnan(*reinterpret_cast<const float*>(dataSrc)))
      {
        memcpy(dataDst, dataSrc, Float32XYZPointSize);
        dataDst += Float32XYZPointSize;
        ++numPoints;
      }
    }
  }
};
typedef RosSubscriber<RosLushCameraDepthPoints> CameraDepthPointsSubscriber;

/// <summary> Twist cmd_vel message publisher. </summary>
struct RosLushCmdVel
{
  struct TwistData
  {
    TwistData(double* linear_, double* angular_)
     : linear(linear_),
       angular(angular_)
    {}
    double* linear;
    double* angular;
  };
  enum { queue_size = 5, };
  typedef geometry_msgs::Twist msg_type;
  typedef TwistData lush_convert_type;
  inline static const char* TopicStr()
  {
    return "/cmd_vel";
  }
  inline static void CopyMsgData(const lush_convert_type& data,
                                 msg_type* msg)
  {
    msg->linear.x = data.linear[0];
    msg->linear.y = data.linear[1];
    msg->linear.z = data.linear[2];
    msg->angular.x = data.angular[0];
    msg->angular.y = data.angular[1];
    msg->angular.z = data.angular[2];
  }
};
typedef RosPublisher<RosLushCmdVel> CmdVelPublisher;

/// <summary> ROS streams for turtlebot. </summary>
class RosStreams
  : // Subscribers.
    public CameraDepthImageRawSubscriber,
    public OdomSubscriber,
    public RobotPoseEkfSubscriber,
    public CameraDepthPointsSubscriber,
    // Publishers
    public CmdVelPublisher
{};
/// <summary> The ROS client for turtlebot. </summary>
typedef RosClientBase<RosStreams> RosClient;

