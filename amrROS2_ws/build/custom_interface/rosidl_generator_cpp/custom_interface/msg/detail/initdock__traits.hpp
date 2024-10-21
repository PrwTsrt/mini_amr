// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from custom_interface:msg/Initdock.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_INTERFACE__MSG__DETAIL__INITDOCK__TRAITS_HPP_
#define CUSTOM_INTERFACE__MSG__DETAIL__INITDOCK__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "custom_interface/msg/detail/initdock__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace custom_interface
{

namespace msg
{

inline void to_flow_style_yaml(
  const Initdock & msg,
  std::ostream & out)
{
  out << "{";
  // member: x
  {
    out << "x: ";
    rosidl_generator_traits::value_to_yaml(msg.x, out);
    out << ", ";
  }

  // member: y
  {
    out << "y: ";
    rosidl_generator_traits::value_to_yaml(msg.y, out);
    out << ", ";
  }

  // member: z
  {
    out << "z: ";
    rosidl_generator_traits::value_to_yaml(msg.z, out);
    out << ", ";
  }

  // member: w
  {
    out << "w: ";
    rosidl_generator_traits::value_to_yaml(msg.w, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const Initdock & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: x
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "x: ";
    rosidl_generator_traits::value_to_yaml(msg.x, out);
    out << "\n";
  }

  // member: y
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "y: ";
    rosidl_generator_traits::value_to_yaml(msg.y, out);
    out << "\n";
  }

  // member: z
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "z: ";
    rosidl_generator_traits::value_to_yaml(msg.z, out);
    out << "\n";
  }

  // member: w
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "w: ";
    rosidl_generator_traits::value_to_yaml(msg.w, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const Initdock & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace custom_interface

namespace rosidl_generator_traits
{

[[deprecated("use custom_interface::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const custom_interface::msg::Initdock & msg,
  std::ostream & out, size_t indentation = 0)
{
  custom_interface::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use custom_interface::msg::to_yaml() instead")]]
inline std::string to_yaml(const custom_interface::msg::Initdock & msg)
{
  return custom_interface::msg::to_yaml(msg);
}

template<>
inline const char * data_type<custom_interface::msg::Initdock>()
{
  return "custom_interface::msg::Initdock";
}

template<>
inline const char * name<custom_interface::msg::Initdock>()
{
  return "custom_interface/msg/Initdock";
}

template<>
struct has_fixed_size<custom_interface::msg::Initdock>
  : std::integral_constant<bool, true> {};

template<>
struct has_bounded_size<custom_interface::msg::Initdock>
  : std::integral_constant<bool, true> {};

template<>
struct is_message<custom_interface::msg::Initdock>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // CUSTOM_INTERFACE__MSG__DETAIL__INITDOCK__TRAITS_HPP_
