#ifndef _LUSH_ROSLUSH_H_
#define _LUSH_ROSLUSH_H_
#include "ros/exceptions.h"
#include "ros/ros.h"
#include "boost/scoped_ptr.hpp"
#include <exception>
#include <string>
#include <iostream>
#include <stdint.h>

// Flags used when client components receive message updates.
enum { DATA_COPY_RELEASE, DATA_COPY_HOLD_REF, };

/// <summary> Generic ROS subscriber logic. </summary>
template <typename RosLushSubscriberTraits>
class RosSubscriber
{
public:
  typedef typename RosLushSubscriberTraits::msg_type msg_type;
  typedef typename RosLushSubscriberTraits::lush_convert_type lush_convert_type;

  RosSubscriber()
  : m_subscriber(),
    m_frId(-1),
    m_lastFrIdUpdated(-1),
    m_mostRecent(),
    m_heldRef()
  {}

  virtual ~RosSubscriber()
  {}

  /// <summary> Callback for subscription. </summary>
  void Callback(const typename msg_type::ConstPtr& msg)
  {
    m_mostRecent = msg;
    ++m_frId;
  }

  /// <summary> Start subscriber. </summary>
  /// <returns> Zero on success, otherwise failure.. </returns>
  int StartSubscriber(ros::NodeHandle* nodeHandle)
  {
    assert(nodeHandle);
    try
    {
      m_subscriber = nodeHandle->subscribe(RosLushSubscriberTraits::TopicStr(),
                                           RosLushSubscriberTraits::queue_size,
                                           &RosSubscriber::Callback,
                                           this);
      std::cout << "StartSubscriber() : subscribed to "
                   "'" << RosLushSubscriberTraits::TopicStr() << "'"
                << std::endl;
      return 0;
    }
    catch (std::exception& e)
    {
      std::cerr << "StartSubscriber() : error subscribing to "
                   "'" << RosLushSubscriberTraits::TopicStr() << "'"
                << std::endl;
      return 1;
    }
  }

  /// <summary> Update subscriber. </summary>
  /// <returns> Non-negative frame id on success. </returns>
  int Update(lush_convert_type* data)
  {
    ros::spinOnce();
    if (m_lastFrIdUpdated < m_frId)
    {
      // Copy data then release message.
      const int res = RosLushSubscriberTraits::CopyMsgData(m_mostRecent, data);
      if (DATA_COPY_RELEASE == res)
      {
        m_mostRecent.reset();
      }
      else
      {
        m_heldRef.reset();
        m_heldRef.swap(m_mostRecent);
      }
      m_lastFrIdUpdated = m_frId;
      return m_lastFrIdUpdated;
    }
    else
    {
      return -1;
    }
  }

private:
  ros::Subscriber m_subscriber;
  int32_t m_frId;
  int32_t m_lastFrIdUpdated;
  typename msg_type::ConstPtr m_mostRecent;
  typename msg_type::ConstPtr m_heldRef;
};

/// <summary> Generic ROS publisher logic. </summary>
template <typename RosLushPublisherTraits>
class RosPublisher
{
public:
  typedef typename RosLushPublisherTraits::msg_type msg_type;
  typedef typename RosLushPublisherTraits::lush_convert_type lush_convert_type;

  RosPublisher()
  : m_publisher(),
    m_msg()
  {}

  virtual ~RosPublisher()
  {}

  /// <summary> Start publishing. </summary>
  /// <returns> Zero on success, otherwise failure.. </returns>
  int StartPublisher(ros::NodeHandle* nodeHandle)
  {
    assert(nodeHandle);
    try
    {
      const std::string topicStr(RosLushPublisherTraits::TopicStr());
      const uint32_t queueSize = RosLushPublisherTraits::queue_size;
      m_publisher = nodeHandle->advertise<msg_type>(topicStr, queueSize);
      std::cout << "StartPublisher() : publishing to "
                   "'" << RosLushPublisherTraits::TopicStr() << "'"
                << std::endl;
      return 0;
    }
    catch (std::exception& e)
    {
      std::cerr << "StartPublisher() : error publishing to "
                   "'" << RosLushPublisherTraits::TopicStr() << "'"
                << std::endl;
      return 1;
    }
  }

  /// <summary> Publish a message. </summary>
  void Publish(const lush_convert_type& data)
  {
    // Copy data then publish.
    // TODO(reissb) -- 20120306 -- To use the shared_ptr API of publish(),
    //   we will require a memory pool of messages. This only affects
    //   intra-process operation between nodes.
    RosLushPublisherTraits::CopyMsgData(data, &m_msg);
    m_publisher.publish(m_msg);
  }

private:
  ros::Publisher m_publisher;
  msg_type m_msg;
};

/// <summary> A generic ROS client. </summary>
/// <remarks>
///   <para> Inherit from subscriber/publisher types to add
///     stream services.
///   </para>
/// </remarks>
template <typename Streams>
class RosClientBase
  : public Streams
{
public:
  RosClientBase()
  : m_nodeHandle(NULL)
  {
    // Init ROS.
    {
      ros::VP_string remappings;
      ros::init(remappings, std::string("listener"),
                ros::init_options::AnonymousName);
    }
    std::cout << "RosClient() : initialized ros" << std::endl;
    // Start the node.
    m_nodeHandle.reset(new ros::NodeHandle());
    std::cout << "RosClient() : started node " << m_nodeHandle
              << std::endl;
  }

  virtual ~RosClientBase() {}

  /// <summary> Start a ROS subscriber. </summary>
  template <typename SubscriberType>
  inline int StartSubscriber()
  {
    SubscriberType* subscriber = static_cast<SubscriberType*>(this);
    return subscriber->StartSubscriber(m_nodeHandle.get());
  }

  /// <summary> Update a ROS subscriber. </summary>
  template <typename SubscriberType>
  inline int Update(typename SubscriberType::lush_convert_type* data)
  {
    SubscriberType* subscriber = static_cast<SubscriberType*>(this);
    return subscriber->Update(data);
  }

  /// <summary> Start a ROS publisher. </summary>
  template <typename PublisherType>
  inline int StartPublisher()
  {
    PublisherType* publisher = static_cast<PublisherType*>(this);
    return publisher->StartPublisher(m_nodeHandle.get());
  }

  /// <summary> Publish to a ROS publisher. </summary>
  template <typename PublisherType>
  inline void Publish(const typename PublisherType::lush_convert_type& data)
  {
    PublisherType* publisher = static_cast<PublisherType*>(this);
    publisher->Publish(data);
  }

private:
  // Disable copy and assign.
  RosClientBase(const RosClientBase&);
  RosClientBase& operator=(const RosClientBase&);

  /// <summary> The ROS node maintains the connections. </summary>
  boost::scoped_ptr<ros::NodeHandle> m_nodeHandle;
};

#endif // _LUSH_ROSLUSH_H_
