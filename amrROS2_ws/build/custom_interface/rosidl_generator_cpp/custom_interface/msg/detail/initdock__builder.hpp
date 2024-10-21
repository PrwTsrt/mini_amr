// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from custom_interface:msg/Initdock.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_INTERFACE__MSG__DETAIL__INITDOCK__BUILDER_HPP_
#define CUSTOM_INTERFACE__MSG__DETAIL__INITDOCK__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "custom_interface/msg/detail/initdock__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace custom_interface
{

namespace msg
{

namespace builder
{

class Init_Initdock_w
{
public:
  explicit Init_Initdock_w(::custom_interface::msg::Initdock & msg)
  : msg_(msg)
  {}
  ::custom_interface::msg::Initdock w(::custom_interface::msg::Initdock::_w_type arg)
  {
    msg_.w = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interface::msg::Initdock msg_;
};

class Init_Initdock_z
{
public:
  explicit Init_Initdock_z(::custom_interface::msg::Initdock & msg)
  : msg_(msg)
  {}
  Init_Initdock_w z(::custom_interface::msg::Initdock::_z_type arg)
  {
    msg_.z = std::move(arg);
    return Init_Initdock_w(msg_);
  }

private:
  ::custom_interface::msg::Initdock msg_;
};

class Init_Initdock_y
{
public:
  explicit Init_Initdock_y(::custom_interface::msg::Initdock & msg)
  : msg_(msg)
  {}
  Init_Initdock_z y(::custom_interface::msg::Initdock::_y_type arg)
  {
    msg_.y = std::move(arg);
    return Init_Initdock_z(msg_);
  }

private:
  ::custom_interface::msg::Initdock msg_;
};

class Init_Initdock_x
{
public:
  Init_Initdock_x()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Initdock_y x(::custom_interface::msg::Initdock::_x_type arg)
  {
    msg_.x = std::move(arg);
    return Init_Initdock_y(msg_);
  }

private:
  ::custom_interface::msg::Initdock msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interface::msg::Initdock>()
{
  return custom_interface::msg::builder::Init_Initdock_x();
}

}  // namespace custom_interface

#endif  // CUSTOM_INTERFACE__MSG__DETAIL__INITDOCK__BUILDER_HPP_
