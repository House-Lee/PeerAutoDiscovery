# Generated by the protocol buffer compiler.  DO NOT EDIT!

from google.protobuf import descriptor
from google.protobuf import message
from google.protobuf import reflection
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)


DESCRIPTOR = descriptor.FileDescriptor(
  name='taskmsg.proto',
  package='',
  serialized_pb='\n\rtaskmsg.proto\"2\n\x0eReqExecuteTask\x12\x0f\n\x07task_id\x18\x01 \x02(\t\x12\x0f\n\x07\x63ommand\x18\x02 \x02(\t\" \n\rReqTaskCommon\x12\x0f\n\x07task_id\x18\x01 \x02(\t\"3\n\rAckTaskOutput\x12\x12\n\nreq_result\x18\x01 \x02(\x05\x12\x0e\n\x06output\x18\x02 \x02(\t\"Q\n\rAckTaskResult\x12\x12\n\nreq_result\x18\x01 \x02(\x05\x12\x13\n\x0btask_result\x18\x02 \x02(\x05\x12\x17\n\x0ftask_str_result\x18\x03 \x02(\t')




_REQEXECUTETASK = descriptor.Descriptor(
  name='ReqExecuteTask',
  full_name='ReqExecuteTask',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    descriptor.FieldDescriptor(
      name='task_id', full_name='ReqExecuteTask.task_id', index=0,
      number=1, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='command', full_name='ReqExecuteTask.command', index=1,
      number=2, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  serialized_start=17,
  serialized_end=67,
)


_REQTASKCOMMON = descriptor.Descriptor(
  name='ReqTaskCommon',
  full_name='ReqTaskCommon',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    descriptor.FieldDescriptor(
      name='task_id', full_name='ReqTaskCommon.task_id', index=0,
      number=1, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  serialized_start=69,
  serialized_end=101,
)


_ACKTASKOUTPUT = descriptor.Descriptor(
  name='AckTaskOutput',
  full_name='AckTaskOutput',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    descriptor.FieldDescriptor(
      name='req_result', full_name='AckTaskOutput.req_result', index=0,
      number=1, type=5, cpp_type=1, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='output', full_name='AckTaskOutput.output', index=1,
      number=2, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  serialized_start=103,
  serialized_end=154,
)


_ACKTASKRESULT = descriptor.Descriptor(
  name='AckTaskResult',
  full_name='AckTaskResult',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    descriptor.FieldDescriptor(
      name='req_result', full_name='AckTaskResult.req_result', index=0,
      number=1, type=5, cpp_type=1, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='task_result', full_name='AckTaskResult.task_result', index=1,
      number=2, type=5, cpp_type=1, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='task_str_result', full_name='AckTaskResult.task_str_result', index=2,
      number=3, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  serialized_start=156,
  serialized_end=237,
)



class ReqExecuteTask(message.Message):
  __metaclass__ = reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _REQEXECUTETASK
  
  # @@protoc_insertion_point(class_scope:ReqExecuteTask)

class ReqTaskCommon(message.Message):
  __metaclass__ = reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _REQTASKCOMMON
  
  # @@protoc_insertion_point(class_scope:ReqTaskCommon)

class AckTaskOutput(message.Message):
  __metaclass__ = reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _ACKTASKOUTPUT
  
  # @@protoc_insertion_point(class_scope:AckTaskOutput)

class AckTaskResult(message.Message):
  __metaclass__ = reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _ACKTASKRESULT
  
  # @@protoc_insertion_point(class_scope:AckTaskResult)

# @@protoc_insertion_point(module_scope)