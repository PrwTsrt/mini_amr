// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from custom_interface:msg/Initdock.idl
// generated code does not contain a copyright notice
#include "custom_interface/msg/detail/initdock__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


bool
custom_interface__msg__Initdock__init(custom_interface__msg__Initdock * msg)
{
  if (!msg) {
    return false;
  }
  // x
  // y
  // z
  // w
  return true;
}

void
custom_interface__msg__Initdock__fini(custom_interface__msg__Initdock * msg)
{
  if (!msg) {
    return;
  }
  // x
  // y
  // z
  // w
}

bool
custom_interface__msg__Initdock__are_equal(const custom_interface__msg__Initdock * lhs, const custom_interface__msg__Initdock * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // x
  if (lhs->x != rhs->x) {
    return false;
  }
  // y
  if (lhs->y != rhs->y) {
    return false;
  }
  // z
  if (lhs->z != rhs->z) {
    return false;
  }
  // w
  if (lhs->w != rhs->w) {
    return false;
  }
  return true;
}

bool
custom_interface__msg__Initdock__copy(
  const custom_interface__msg__Initdock * input,
  custom_interface__msg__Initdock * output)
{
  if (!input || !output) {
    return false;
  }
  // x
  output->x = input->x;
  // y
  output->y = input->y;
  // z
  output->z = input->z;
  // w
  output->w = input->w;
  return true;
}

custom_interface__msg__Initdock *
custom_interface__msg__Initdock__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  custom_interface__msg__Initdock * msg = (custom_interface__msg__Initdock *)allocator.allocate(sizeof(custom_interface__msg__Initdock), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(custom_interface__msg__Initdock));
  bool success = custom_interface__msg__Initdock__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
custom_interface__msg__Initdock__destroy(custom_interface__msg__Initdock * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    custom_interface__msg__Initdock__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
custom_interface__msg__Initdock__Sequence__init(custom_interface__msg__Initdock__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  custom_interface__msg__Initdock * data = NULL;

  if (size) {
    data = (custom_interface__msg__Initdock *)allocator.zero_allocate(size, sizeof(custom_interface__msg__Initdock), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = custom_interface__msg__Initdock__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        custom_interface__msg__Initdock__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
custom_interface__msg__Initdock__Sequence__fini(custom_interface__msg__Initdock__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      custom_interface__msg__Initdock__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

custom_interface__msg__Initdock__Sequence *
custom_interface__msg__Initdock__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  custom_interface__msg__Initdock__Sequence * array = (custom_interface__msg__Initdock__Sequence *)allocator.allocate(sizeof(custom_interface__msg__Initdock__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = custom_interface__msg__Initdock__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
custom_interface__msg__Initdock__Sequence__destroy(custom_interface__msg__Initdock__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    custom_interface__msg__Initdock__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
custom_interface__msg__Initdock__Sequence__are_equal(const custom_interface__msg__Initdock__Sequence * lhs, const custom_interface__msg__Initdock__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!custom_interface__msg__Initdock__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
custom_interface__msg__Initdock__Sequence__copy(
  const custom_interface__msg__Initdock__Sequence * input,
  custom_interface__msg__Initdock__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(custom_interface__msg__Initdock);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    custom_interface__msg__Initdock * data =
      (custom_interface__msg__Initdock *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!custom_interface__msg__Initdock__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          custom_interface__msg__Initdock__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!custom_interface__msg__Initdock__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
