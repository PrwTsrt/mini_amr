// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from custom_interface:action/Autodock.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_INTERFACE__ACTION__DETAIL__AUTODOCK__STRUCT_HPP_
#define CUSTOM_INTERFACE__ACTION__DETAIL__AUTODOCK__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__custom_interface__action__Autodock_Goal __attribute__((deprecated))
#else
# define DEPRECATED__custom_interface__action__Autodock_Goal __declspec(deprecated)
#endif

namespace custom_interface
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct Autodock_Goal_
{
  using Type = Autodock_Goal_<ContainerAllocator>;

  explicit Autodock_Goal_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->is_dock = false;
    }
  }

  explicit Autodock_Goal_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->is_dock = false;
    }
  }

  // field types and members
  using _is_dock_type =
    bool;
  _is_dock_type is_dock;

  // setters for named parameter idiom
  Type & set__is_dock(
    const bool & _arg)
  {
    this->is_dock = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    custom_interface::action::Autodock_Goal_<ContainerAllocator> *;
  using ConstRawPtr =
    const custom_interface::action::Autodock_Goal_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_Goal_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_Goal_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_Goal_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_Goal_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_Goal_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_Goal_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_Goal_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_Goal_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__custom_interface__action__Autodock_Goal
    std::shared_ptr<custom_interface::action::Autodock_Goal_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__custom_interface__action__Autodock_Goal
    std::shared_ptr<custom_interface::action::Autodock_Goal_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Autodock_Goal_ & other) const
  {
    if (this->is_dock != other.is_dock) {
      return false;
    }
    return true;
  }
  bool operator!=(const Autodock_Goal_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Autodock_Goal_

// alias to use template instance with default allocator
using Autodock_Goal =
  custom_interface::action::Autodock_Goal_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace custom_interface


#ifndef _WIN32
# define DEPRECATED__custom_interface__action__Autodock_Result __attribute__((deprecated))
#else
# define DEPRECATED__custom_interface__action__Autodock_Result __declspec(deprecated)
#endif

namespace custom_interface
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct Autodock_Result_
{
  using Type = Autodock_Result_<ContainerAllocator>;

  explicit Autodock_Result_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->structure_needs_at_least_one_member = 0;
    }
  }

  explicit Autodock_Result_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->structure_needs_at_least_one_member = 0;
    }
  }

  // field types and members
  using _structure_needs_at_least_one_member_type =
    uint8_t;
  _structure_needs_at_least_one_member_type structure_needs_at_least_one_member;


  // constant declarations

  // pointer types
  using RawPtr =
    custom_interface::action::Autodock_Result_<ContainerAllocator> *;
  using ConstRawPtr =
    const custom_interface::action::Autodock_Result_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_Result_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_Result_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_Result_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_Result_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_Result_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_Result_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_Result_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_Result_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__custom_interface__action__Autodock_Result
    std::shared_ptr<custom_interface::action::Autodock_Result_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__custom_interface__action__Autodock_Result
    std::shared_ptr<custom_interface::action::Autodock_Result_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Autodock_Result_ & other) const
  {
    if (this->structure_needs_at_least_one_member != other.structure_needs_at_least_one_member) {
      return false;
    }
    return true;
  }
  bool operator!=(const Autodock_Result_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Autodock_Result_

// alias to use template instance with default allocator
using Autodock_Result =
  custom_interface::action::Autodock_Result_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace custom_interface


// Include directives for member types
// Member 'text'
#include "std_msgs/msg/detail/string__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__custom_interface__action__Autodock_Feedback __attribute__((deprecated))
#else
# define DEPRECATED__custom_interface__action__Autodock_Feedback __declspec(deprecated)
#endif

namespace custom_interface
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct Autodock_Feedback_
{
  using Type = Autodock_Feedback_<ContainerAllocator>;

  explicit Autodock_Feedback_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : text(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->step = 0l;
    }
  }

  explicit Autodock_Feedback_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : text(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->step = 0l;
    }
  }

  // field types and members
  using _step_type =
    int32_t;
  _step_type step;
  using _text_type =
    std_msgs::msg::String_<ContainerAllocator>;
  _text_type text;

  // setters for named parameter idiom
  Type & set__step(
    const int32_t & _arg)
  {
    this->step = _arg;
    return *this;
  }
  Type & set__text(
    const std_msgs::msg::String_<ContainerAllocator> & _arg)
  {
    this->text = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    custom_interface::action::Autodock_Feedback_<ContainerAllocator> *;
  using ConstRawPtr =
    const custom_interface::action::Autodock_Feedback_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_Feedback_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_Feedback_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_Feedback_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_Feedback_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_Feedback_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_Feedback_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_Feedback_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_Feedback_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__custom_interface__action__Autodock_Feedback
    std::shared_ptr<custom_interface::action::Autodock_Feedback_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__custom_interface__action__Autodock_Feedback
    std::shared_ptr<custom_interface::action::Autodock_Feedback_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Autodock_Feedback_ & other) const
  {
    if (this->step != other.step) {
      return false;
    }
    if (this->text != other.text) {
      return false;
    }
    return true;
  }
  bool operator!=(const Autodock_Feedback_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Autodock_Feedback_

// alias to use template instance with default allocator
using Autodock_Feedback =
  custom_interface::action::Autodock_Feedback_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace custom_interface


// Include directives for member types
// Member 'goal_id'
#include "unique_identifier_msgs/msg/detail/uuid__struct.hpp"
// Member 'goal'
#include "custom_interface/action/detail/autodock__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__custom_interface__action__Autodock_SendGoal_Request __attribute__((deprecated))
#else
# define DEPRECATED__custom_interface__action__Autodock_SendGoal_Request __declspec(deprecated)
#endif

namespace custom_interface
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct Autodock_SendGoal_Request_
{
  using Type = Autodock_SendGoal_Request_<ContainerAllocator>;

  explicit Autodock_SendGoal_Request_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : goal_id(_init),
    goal(_init)
  {
    (void)_init;
  }

  explicit Autodock_SendGoal_Request_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : goal_id(_alloc, _init),
    goal(_alloc, _init)
  {
    (void)_init;
  }

  // field types and members
  using _goal_id_type =
    unique_identifier_msgs::msg::UUID_<ContainerAllocator>;
  _goal_id_type goal_id;
  using _goal_type =
    custom_interface::action::Autodock_Goal_<ContainerAllocator>;
  _goal_type goal;

  // setters for named parameter idiom
  Type & set__goal_id(
    const unique_identifier_msgs::msg::UUID_<ContainerAllocator> & _arg)
  {
    this->goal_id = _arg;
    return *this;
  }
  Type & set__goal(
    const custom_interface::action::Autodock_Goal_<ContainerAllocator> & _arg)
  {
    this->goal = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    custom_interface::action::Autodock_SendGoal_Request_<ContainerAllocator> *;
  using ConstRawPtr =
    const custom_interface::action::Autodock_SendGoal_Request_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_SendGoal_Request_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_SendGoal_Request_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_SendGoal_Request_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_SendGoal_Request_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_SendGoal_Request_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_SendGoal_Request_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_SendGoal_Request_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_SendGoal_Request_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__custom_interface__action__Autodock_SendGoal_Request
    std::shared_ptr<custom_interface::action::Autodock_SendGoal_Request_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__custom_interface__action__Autodock_SendGoal_Request
    std::shared_ptr<custom_interface::action::Autodock_SendGoal_Request_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Autodock_SendGoal_Request_ & other) const
  {
    if (this->goal_id != other.goal_id) {
      return false;
    }
    if (this->goal != other.goal) {
      return false;
    }
    return true;
  }
  bool operator!=(const Autodock_SendGoal_Request_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Autodock_SendGoal_Request_

// alias to use template instance with default allocator
using Autodock_SendGoal_Request =
  custom_interface::action::Autodock_SendGoal_Request_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace custom_interface


// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__custom_interface__action__Autodock_SendGoal_Response __attribute__((deprecated))
#else
# define DEPRECATED__custom_interface__action__Autodock_SendGoal_Response __declspec(deprecated)
#endif

namespace custom_interface
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct Autodock_SendGoal_Response_
{
  using Type = Autodock_SendGoal_Response_<ContainerAllocator>;

  explicit Autodock_SendGoal_Response_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : stamp(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->accepted = false;
    }
  }

  explicit Autodock_SendGoal_Response_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : stamp(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->accepted = false;
    }
  }

  // field types and members
  using _accepted_type =
    bool;
  _accepted_type accepted;
  using _stamp_type =
    builtin_interfaces::msg::Time_<ContainerAllocator>;
  _stamp_type stamp;

  // setters for named parameter idiom
  Type & set__accepted(
    const bool & _arg)
  {
    this->accepted = _arg;
    return *this;
  }
  Type & set__stamp(
    const builtin_interfaces::msg::Time_<ContainerAllocator> & _arg)
  {
    this->stamp = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    custom_interface::action::Autodock_SendGoal_Response_<ContainerAllocator> *;
  using ConstRawPtr =
    const custom_interface::action::Autodock_SendGoal_Response_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_SendGoal_Response_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_SendGoal_Response_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_SendGoal_Response_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_SendGoal_Response_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_SendGoal_Response_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_SendGoal_Response_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_SendGoal_Response_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_SendGoal_Response_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__custom_interface__action__Autodock_SendGoal_Response
    std::shared_ptr<custom_interface::action::Autodock_SendGoal_Response_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__custom_interface__action__Autodock_SendGoal_Response
    std::shared_ptr<custom_interface::action::Autodock_SendGoal_Response_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Autodock_SendGoal_Response_ & other) const
  {
    if (this->accepted != other.accepted) {
      return false;
    }
    if (this->stamp != other.stamp) {
      return false;
    }
    return true;
  }
  bool operator!=(const Autodock_SendGoal_Response_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Autodock_SendGoal_Response_

// alias to use template instance with default allocator
using Autodock_SendGoal_Response =
  custom_interface::action::Autodock_SendGoal_Response_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace custom_interface

namespace custom_interface
{

namespace action
{

struct Autodock_SendGoal
{
  using Request = custom_interface::action::Autodock_SendGoal_Request;
  using Response = custom_interface::action::Autodock_SendGoal_Response;
};

}  // namespace action

}  // namespace custom_interface


// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__custom_interface__action__Autodock_GetResult_Request __attribute__((deprecated))
#else
# define DEPRECATED__custom_interface__action__Autodock_GetResult_Request __declspec(deprecated)
#endif

namespace custom_interface
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct Autodock_GetResult_Request_
{
  using Type = Autodock_GetResult_Request_<ContainerAllocator>;

  explicit Autodock_GetResult_Request_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : goal_id(_init)
  {
    (void)_init;
  }

  explicit Autodock_GetResult_Request_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : goal_id(_alloc, _init)
  {
    (void)_init;
  }

  // field types and members
  using _goal_id_type =
    unique_identifier_msgs::msg::UUID_<ContainerAllocator>;
  _goal_id_type goal_id;

  // setters for named parameter idiom
  Type & set__goal_id(
    const unique_identifier_msgs::msg::UUID_<ContainerAllocator> & _arg)
  {
    this->goal_id = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    custom_interface::action::Autodock_GetResult_Request_<ContainerAllocator> *;
  using ConstRawPtr =
    const custom_interface::action::Autodock_GetResult_Request_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_GetResult_Request_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_GetResult_Request_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_GetResult_Request_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_GetResult_Request_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_GetResult_Request_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_GetResult_Request_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_GetResult_Request_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_GetResult_Request_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__custom_interface__action__Autodock_GetResult_Request
    std::shared_ptr<custom_interface::action::Autodock_GetResult_Request_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__custom_interface__action__Autodock_GetResult_Request
    std::shared_ptr<custom_interface::action::Autodock_GetResult_Request_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Autodock_GetResult_Request_ & other) const
  {
    if (this->goal_id != other.goal_id) {
      return false;
    }
    return true;
  }
  bool operator!=(const Autodock_GetResult_Request_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Autodock_GetResult_Request_

// alias to use template instance with default allocator
using Autodock_GetResult_Request =
  custom_interface::action::Autodock_GetResult_Request_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace custom_interface


// Include directives for member types
// Member 'result'
// already included above
// #include "custom_interface/action/detail/autodock__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__custom_interface__action__Autodock_GetResult_Response __attribute__((deprecated))
#else
# define DEPRECATED__custom_interface__action__Autodock_GetResult_Response __declspec(deprecated)
#endif

namespace custom_interface
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct Autodock_GetResult_Response_
{
  using Type = Autodock_GetResult_Response_<ContainerAllocator>;

  explicit Autodock_GetResult_Response_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : result(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->status = 0;
    }
  }

  explicit Autodock_GetResult_Response_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : result(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->status = 0;
    }
  }

  // field types and members
  using _status_type =
    int8_t;
  _status_type status;
  using _result_type =
    custom_interface::action::Autodock_Result_<ContainerAllocator>;
  _result_type result;

  // setters for named parameter idiom
  Type & set__status(
    const int8_t & _arg)
  {
    this->status = _arg;
    return *this;
  }
  Type & set__result(
    const custom_interface::action::Autodock_Result_<ContainerAllocator> & _arg)
  {
    this->result = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    custom_interface::action::Autodock_GetResult_Response_<ContainerAllocator> *;
  using ConstRawPtr =
    const custom_interface::action::Autodock_GetResult_Response_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_GetResult_Response_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_GetResult_Response_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_GetResult_Response_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_GetResult_Response_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_GetResult_Response_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_GetResult_Response_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_GetResult_Response_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_GetResult_Response_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__custom_interface__action__Autodock_GetResult_Response
    std::shared_ptr<custom_interface::action::Autodock_GetResult_Response_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__custom_interface__action__Autodock_GetResult_Response
    std::shared_ptr<custom_interface::action::Autodock_GetResult_Response_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Autodock_GetResult_Response_ & other) const
  {
    if (this->status != other.status) {
      return false;
    }
    if (this->result != other.result) {
      return false;
    }
    return true;
  }
  bool operator!=(const Autodock_GetResult_Response_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Autodock_GetResult_Response_

// alias to use template instance with default allocator
using Autodock_GetResult_Response =
  custom_interface::action::Autodock_GetResult_Response_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace custom_interface

namespace custom_interface
{

namespace action
{

struct Autodock_GetResult
{
  using Request = custom_interface::action::Autodock_GetResult_Request;
  using Response = custom_interface::action::Autodock_GetResult_Response;
};

}  // namespace action

}  // namespace custom_interface


// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.hpp"
// Member 'feedback'
// already included above
// #include "custom_interface/action/detail/autodock__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__custom_interface__action__Autodock_FeedbackMessage __attribute__((deprecated))
#else
# define DEPRECATED__custom_interface__action__Autodock_FeedbackMessage __declspec(deprecated)
#endif

namespace custom_interface
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct Autodock_FeedbackMessage_
{
  using Type = Autodock_FeedbackMessage_<ContainerAllocator>;

  explicit Autodock_FeedbackMessage_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : goal_id(_init),
    feedback(_init)
  {
    (void)_init;
  }

  explicit Autodock_FeedbackMessage_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : goal_id(_alloc, _init),
    feedback(_alloc, _init)
  {
    (void)_init;
  }

  // field types and members
  using _goal_id_type =
    unique_identifier_msgs::msg::UUID_<ContainerAllocator>;
  _goal_id_type goal_id;
  using _feedback_type =
    custom_interface::action::Autodock_Feedback_<ContainerAllocator>;
  _feedback_type feedback;

  // setters for named parameter idiom
  Type & set__goal_id(
    const unique_identifier_msgs::msg::UUID_<ContainerAllocator> & _arg)
  {
    this->goal_id = _arg;
    return *this;
  }
  Type & set__feedback(
    const custom_interface::action::Autodock_Feedback_<ContainerAllocator> & _arg)
  {
    this->feedback = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    custom_interface::action::Autodock_FeedbackMessage_<ContainerAllocator> *;
  using ConstRawPtr =
    const custom_interface::action::Autodock_FeedbackMessage_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_FeedbackMessage_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<custom_interface::action::Autodock_FeedbackMessage_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_FeedbackMessage_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_FeedbackMessage_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      custom_interface::action::Autodock_FeedbackMessage_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<custom_interface::action::Autodock_FeedbackMessage_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_FeedbackMessage_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<custom_interface::action::Autodock_FeedbackMessage_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__custom_interface__action__Autodock_FeedbackMessage
    std::shared_ptr<custom_interface::action::Autodock_FeedbackMessage_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__custom_interface__action__Autodock_FeedbackMessage
    std::shared_ptr<custom_interface::action::Autodock_FeedbackMessage_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Autodock_FeedbackMessage_ & other) const
  {
    if (this->goal_id != other.goal_id) {
      return false;
    }
    if (this->feedback != other.feedback) {
      return false;
    }
    return true;
  }
  bool operator!=(const Autodock_FeedbackMessage_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Autodock_FeedbackMessage_

// alias to use template instance with default allocator
using Autodock_FeedbackMessage =
  custom_interface::action::Autodock_FeedbackMessage_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace custom_interface

#include "action_msgs/srv/cancel_goal.hpp"
#include "action_msgs/msg/goal_info.hpp"
#include "action_msgs/msg/goal_status_array.hpp"

namespace custom_interface
{

namespace action
{

struct Autodock
{
  /// The goal message defined in the action definition.
  using Goal = custom_interface::action::Autodock_Goal;
  /// The result message defined in the action definition.
  using Result = custom_interface::action::Autodock_Result;
  /// The feedback message defined in the action definition.
  using Feedback = custom_interface::action::Autodock_Feedback;

  struct Impl
  {
    /// The send_goal service using a wrapped version of the goal message as a request.
    using SendGoalService = custom_interface::action::Autodock_SendGoal;
    /// The get_result service using a wrapped version of the result message as a response.
    using GetResultService = custom_interface::action::Autodock_GetResult;
    /// The feedback message with generic fields which wraps the feedback message.
    using FeedbackMessage = custom_interface::action::Autodock_FeedbackMessage;

    /// The generic service to cancel a goal.
    using CancelGoalService = action_msgs::srv::CancelGoal;
    /// The generic message for the status of a goal.
    using GoalStatusMessage = action_msgs::msg::GoalStatusArray;
  };
};

typedef struct Autodock Autodock;

}  // namespace action

}  // namespace custom_interface

#endif  // CUSTOM_INTERFACE__ACTION__DETAIL__AUTODOCK__STRUCT_HPP_
