// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from custom_interface:action/Autodock.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_INTERFACE__ACTION__DETAIL__AUTODOCK__BUILDER_HPP_
#define CUSTOM_INTERFACE__ACTION__DETAIL__AUTODOCK__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "custom_interface/action/detail/autodock__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace custom_interface
{

namespace action
{

namespace builder
{

class Init_Autodock_Goal_is_dock
{
public:
  Init_Autodock_Goal_is_dock()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::custom_interface::action::Autodock_Goal is_dock(::custom_interface::action::Autodock_Goal::_is_dock_type arg)
  {
    msg_.is_dock = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interface::action::Autodock_Goal msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interface::action::Autodock_Goal>()
{
  return custom_interface::action::builder::Init_Autodock_Goal_is_dock();
}

}  // namespace custom_interface


namespace custom_interface
{

namespace action
{


}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interface::action::Autodock_Result>()
{
  return ::custom_interface::action::Autodock_Result(rosidl_runtime_cpp::MessageInitialization::ZERO);
}

}  // namespace custom_interface


namespace custom_interface
{

namespace action
{

namespace builder
{

class Init_Autodock_Feedback_text
{
public:
  explicit Init_Autodock_Feedback_text(::custom_interface::action::Autodock_Feedback & msg)
  : msg_(msg)
  {}
  ::custom_interface::action::Autodock_Feedback text(::custom_interface::action::Autodock_Feedback::_text_type arg)
  {
    msg_.text = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interface::action::Autodock_Feedback msg_;
};

class Init_Autodock_Feedback_step
{
public:
  Init_Autodock_Feedback_step()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Autodock_Feedback_text step(::custom_interface::action::Autodock_Feedback::_step_type arg)
  {
    msg_.step = std::move(arg);
    return Init_Autodock_Feedback_text(msg_);
  }

private:
  ::custom_interface::action::Autodock_Feedback msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interface::action::Autodock_Feedback>()
{
  return custom_interface::action::builder::Init_Autodock_Feedback_step();
}

}  // namespace custom_interface


namespace custom_interface
{

namespace action
{

namespace builder
{

class Init_Autodock_SendGoal_Request_goal
{
public:
  explicit Init_Autodock_SendGoal_Request_goal(::custom_interface::action::Autodock_SendGoal_Request & msg)
  : msg_(msg)
  {}
  ::custom_interface::action::Autodock_SendGoal_Request goal(::custom_interface::action::Autodock_SendGoal_Request::_goal_type arg)
  {
    msg_.goal = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interface::action::Autodock_SendGoal_Request msg_;
};

class Init_Autodock_SendGoal_Request_goal_id
{
public:
  Init_Autodock_SendGoal_Request_goal_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Autodock_SendGoal_Request_goal goal_id(::custom_interface::action::Autodock_SendGoal_Request::_goal_id_type arg)
  {
    msg_.goal_id = std::move(arg);
    return Init_Autodock_SendGoal_Request_goal(msg_);
  }

private:
  ::custom_interface::action::Autodock_SendGoal_Request msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interface::action::Autodock_SendGoal_Request>()
{
  return custom_interface::action::builder::Init_Autodock_SendGoal_Request_goal_id();
}

}  // namespace custom_interface


namespace custom_interface
{

namespace action
{

namespace builder
{

class Init_Autodock_SendGoal_Response_stamp
{
public:
  explicit Init_Autodock_SendGoal_Response_stamp(::custom_interface::action::Autodock_SendGoal_Response & msg)
  : msg_(msg)
  {}
  ::custom_interface::action::Autodock_SendGoal_Response stamp(::custom_interface::action::Autodock_SendGoal_Response::_stamp_type arg)
  {
    msg_.stamp = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interface::action::Autodock_SendGoal_Response msg_;
};

class Init_Autodock_SendGoal_Response_accepted
{
public:
  Init_Autodock_SendGoal_Response_accepted()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Autodock_SendGoal_Response_stamp accepted(::custom_interface::action::Autodock_SendGoal_Response::_accepted_type arg)
  {
    msg_.accepted = std::move(arg);
    return Init_Autodock_SendGoal_Response_stamp(msg_);
  }

private:
  ::custom_interface::action::Autodock_SendGoal_Response msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interface::action::Autodock_SendGoal_Response>()
{
  return custom_interface::action::builder::Init_Autodock_SendGoal_Response_accepted();
}

}  // namespace custom_interface


namespace custom_interface
{

namespace action
{

namespace builder
{

class Init_Autodock_GetResult_Request_goal_id
{
public:
  Init_Autodock_GetResult_Request_goal_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::custom_interface::action::Autodock_GetResult_Request goal_id(::custom_interface::action::Autodock_GetResult_Request::_goal_id_type arg)
  {
    msg_.goal_id = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interface::action::Autodock_GetResult_Request msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interface::action::Autodock_GetResult_Request>()
{
  return custom_interface::action::builder::Init_Autodock_GetResult_Request_goal_id();
}

}  // namespace custom_interface


namespace custom_interface
{

namespace action
{

namespace builder
{

class Init_Autodock_GetResult_Response_result
{
public:
  explicit Init_Autodock_GetResult_Response_result(::custom_interface::action::Autodock_GetResult_Response & msg)
  : msg_(msg)
  {}
  ::custom_interface::action::Autodock_GetResult_Response result(::custom_interface::action::Autodock_GetResult_Response::_result_type arg)
  {
    msg_.result = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interface::action::Autodock_GetResult_Response msg_;
};

class Init_Autodock_GetResult_Response_status
{
public:
  Init_Autodock_GetResult_Response_status()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Autodock_GetResult_Response_result status(::custom_interface::action::Autodock_GetResult_Response::_status_type arg)
  {
    msg_.status = std::move(arg);
    return Init_Autodock_GetResult_Response_result(msg_);
  }

private:
  ::custom_interface::action::Autodock_GetResult_Response msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interface::action::Autodock_GetResult_Response>()
{
  return custom_interface::action::builder::Init_Autodock_GetResult_Response_status();
}

}  // namespace custom_interface


namespace custom_interface
{

namespace action
{

namespace builder
{

class Init_Autodock_FeedbackMessage_feedback
{
public:
  explicit Init_Autodock_FeedbackMessage_feedback(::custom_interface::action::Autodock_FeedbackMessage & msg)
  : msg_(msg)
  {}
  ::custom_interface::action::Autodock_FeedbackMessage feedback(::custom_interface::action::Autodock_FeedbackMessage::_feedback_type arg)
  {
    msg_.feedback = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interface::action::Autodock_FeedbackMessage msg_;
};

class Init_Autodock_FeedbackMessage_goal_id
{
public:
  Init_Autodock_FeedbackMessage_goal_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Autodock_FeedbackMessage_feedback goal_id(::custom_interface::action::Autodock_FeedbackMessage::_goal_id_type arg)
  {
    msg_.goal_id = std::move(arg);
    return Init_Autodock_FeedbackMessage_feedback(msg_);
  }

private:
  ::custom_interface::action::Autodock_FeedbackMessage msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interface::action::Autodock_FeedbackMessage>()
{
  return custom_interface::action::builder::Init_Autodock_FeedbackMessage_goal_id();
}

}  // namespace custom_interface

#endif  // CUSTOM_INTERFACE__ACTION__DETAIL__AUTODOCK__BUILDER_HPP_
