// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from custom_interface:action/Autodock.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_INTERFACE__ACTION__DETAIL__AUTODOCK__STRUCT_H_
#define CUSTOM_INTERFACE__ACTION__DETAIL__AUTODOCK__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in action/Autodock in the package custom_interface.
typedef struct custom_interface__action__Autodock_Goal
{
  bool is_dock;
} custom_interface__action__Autodock_Goal;

// Struct for a sequence of custom_interface__action__Autodock_Goal.
typedef struct custom_interface__action__Autodock_Goal__Sequence
{
  custom_interface__action__Autodock_Goal * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interface__action__Autodock_Goal__Sequence;


// Constants defined in the message

/// Struct defined in action/Autodock in the package custom_interface.
typedef struct custom_interface__action__Autodock_Result
{
  uint8_t structure_needs_at_least_one_member;
} custom_interface__action__Autodock_Result;

// Struct for a sequence of custom_interface__action__Autodock_Result.
typedef struct custom_interface__action__Autodock_Result__Sequence
{
  custom_interface__action__Autodock_Result * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interface__action__Autodock_Result__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'text'
#include "std_msgs/msg/detail/string__struct.h"

/// Struct defined in action/Autodock in the package custom_interface.
typedef struct custom_interface__action__Autodock_Feedback
{
  int32_t step;
  std_msgs__msg__String text;
} custom_interface__action__Autodock_Feedback;

// Struct for a sequence of custom_interface__action__Autodock_Feedback.
typedef struct custom_interface__action__Autodock_Feedback__Sequence
{
  custom_interface__action__Autodock_Feedback * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interface__action__Autodock_Feedback__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
#include "unique_identifier_msgs/msg/detail/uuid__struct.h"
// Member 'goal'
#include "custom_interface/action/detail/autodock__struct.h"

/// Struct defined in action/Autodock in the package custom_interface.
typedef struct custom_interface__action__Autodock_SendGoal_Request
{
  unique_identifier_msgs__msg__UUID goal_id;
  custom_interface__action__Autodock_Goal goal;
} custom_interface__action__Autodock_SendGoal_Request;

// Struct for a sequence of custom_interface__action__Autodock_SendGoal_Request.
typedef struct custom_interface__action__Autodock_SendGoal_Request__Sequence
{
  custom_interface__action__Autodock_SendGoal_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interface__action__Autodock_SendGoal_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__struct.h"

/// Struct defined in action/Autodock in the package custom_interface.
typedef struct custom_interface__action__Autodock_SendGoal_Response
{
  bool accepted;
  builtin_interfaces__msg__Time stamp;
} custom_interface__action__Autodock_SendGoal_Response;

// Struct for a sequence of custom_interface__action__Autodock_SendGoal_Response.
typedef struct custom_interface__action__Autodock_SendGoal_Response__Sequence
{
  custom_interface__action__Autodock_SendGoal_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interface__action__Autodock_SendGoal_Response__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.h"

/// Struct defined in action/Autodock in the package custom_interface.
typedef struct custom_interface__action__Autodock_GetResult_Request
{
  unique_identifier_msgs__msg__UUID goal_id;
} custom_interface__action__Autodock_GetResult_Request;

// Struct for a sequence of custom_interface__action__Autodock_GetResult_Request.
typedef struct custom_interface__action__Autodock_GetResult_Request__Sequence
{
  custom_interface__action__Autodock_GetResult_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interface__action__Autodock_GetResult_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'result'
// already included above
// #include "custom_interface/action/detail/autodock__struct.h"

/// Struct defined in action/Autodock in the package custom_interface.
typedef struct custom_interface__action__Autodock_GetResult_Response
{
  int8_t status;
  custom_interface__action__Autodock_Result result;
} custom_interface__action__Autodock_GetResult_Response;

// Struct for a sequence of custom_interface__action__Autodock_GetResult_Response.
typedef struct custom_interface__action__Autodock_GetResult_Response__Sequence
{
  custom_interface__action__Autodock_GetResult_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interface__action__Autodock_GetResult_Response__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.h"
// Member 'feedback'
// already included above
// #include "custom_interface/action/detail/autodock__struct.h"

/// Struct defined in action/Autodock in the package custom_interface.
typedef struct custom_interface__action__Autodock_FeedbackMessage
{
  unique_identifier_msgs__msg__UUID goal_id;
  custom_interface__action__Autodock_Feedback feedback;
} custom_interface__action__Autodock_FeedbackMessage;

// Struct for a sequence of custom_interface__action__Autodock_FeedbackMessage.
typedef struct custom_interface__action__Autodock_FeedbackMessage__Sequence
{
  custom_interface__action__Autodock_FeedbackMessage * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interface__action__Autodock_FeedbackMessage__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // CUSTOM_INTERFACE__ACTION__DETAIL__AUTODOCK__STRUCT_H_
