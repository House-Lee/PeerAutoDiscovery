/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

#ifndef PROTOBUF_C_common_5fmsg_2eproto__INCLUDED
#define PROTOBUF_C_common_5fmsg_2eproto__INCLUDED

#include <google/protobuf-c/protobuf-c.h>

PROTOBUF_C_BEGIN_DECLS


typedef struct _Pluse Pluse;
typedef struct _ReqAuth ReqAuth;
typedef struct _AckCommon AckCommon;
typedef struct _TaskDesc TaskDesc;
typedef struct _ReqSetConfig ReqSetConfig;
typedef struct _ReqGetConfig ReqGetConfig;


/* --- enums --- */


/* --- messages --- */

struct  _Pluse
{
  ProtobufCMessage base;
  int32_t timestamp;
};
#define PLUSE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&pluse__descriptor) \
    , 0 }


struct  _ReqAuth
{
  ProtobufCMessage base;
  int32_t timestamp;
  char *auth_key;
  char *sign;
};
#define REQ_AUTH__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&req_auth__descriptor) \
    , 0, NULL, NULL }


struct  _AckCommon
{
  ProtobufCMessage base;
  int32_t code;
  char *msg;
};
#define ACK_COMMON__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ack_common__descriptor) \
    , 0, NULL }


struct  _TaskDesc
{
  ProtobufCMessage base;
  char *task_id;
  int32_t retcode;
  int32_t result;
  char *str_result;
  char *output;
};
#define TASK__DESC__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&task__desc__descriptor) \
    , NULL, 0, 0, NULL, NULL }


struct  _ReqSetConfig
{
  ProtobufCMessage base;
  char *key;
  char *value;
};
#define REQ_SET_CONFIG__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&req_set_config__descriptor) \
    , NULL, NULL }


struct  _ReqGetConfig
{
  ProtobufCMessage base;
  char *key;
};
#define REQ_GET_CONFIG__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&req_get_config__descriptor) \
    , NULL }


/* Pluse methods */
void   pluse__init
                     (Pluse         *message);
size_t pluse__get_packed_size
                     (const Pluse   *message);
size_t pluse__pack
                     (const Pluse   *message,
                      uint8_t             *out);
size_t pluse__pack_to_buffer
                     (const Pluse   *message,
                      ProtobufCBuffer     *buffer);
Pluse *
       pluse__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   pluse__free_unpacked
                     (Pluse *message,
                      ProtobufCAllocator *allocator);
/* ReqAuth methods */
void   req_auth__init
                     (ReqAuth         *message);
size_t req_auth__get_packed_size
                     (const ReqAuth   *message);
size_t req_auth__pack
                     (const ReqAuth   *message,
                      uint8_t             *out);
size_t req_auth__pack_to_buffer
                     (const ReqAuth   *message,
                      ProtobufCBuffer     *buffer);
ReqAuth *
       req_auth__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   req_auth__free_unpacked
                     (ReqAuth *message,
                      ProtobufCAllocator *allocator);
/* AckCommon methods */
void   ack_common__init
                     (AckCommon         *message);
size_t ack_common__get_packed_size
                     (const AckCommon   *message);
size_t ack_common__pack
                     (const AckCommon   *message,
                      uint8_t             *out);
size_t ack_common__pack_to_buffer
                     (const AckCommon   *message,
                      ProtobufCBuffer     *buffer);
AckCommon *
       ack_common__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ack_common__free_unpacked
                     (AckCommon *message,
                      ProtobufCAllocator *allocator);
/* TaskDesc methods */
void   task__desc__init
                     (TaskDesc         *message);
size_t task__desc__get_packed_size
                     (const TaskDesc   *message);
size_t task__desc__pack
                     (const TaskDesc   *message,
                      uint8_t             *out);
size_t task__desc__pack_to_buffer
                     (const TaskDesc   *message,
                      ProtobufCBuffer     *buffer);
TaskDesc *
       task__desc__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   task__desc__free_unpacked
                     (TaskDesc *message,
                      ProtobufCAllocator *allocator);
/* ReqSetConfig methods */
void   req_set_config__init
                     (ReqSetConfig         *message);
size_t req_set_config__get_packed_size
                     (const ReqSetConfig   *message);
size_t req_set_config__pack
                     (const ReqSetConfig   *message,
                      uint8_t             *out);
size_t req_set_config__pack_to_buffer
                     (const ReqSetConfig   *message,
                      ProtobufCBuffer     *buffer);
ReqSetConfig *
       req_set_config__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   req_set_config__free_unpacked
                     (ReqSetConfig *message,
                      ProtobufCAllocator *allocator);
/* ReqGetConfig methods */
void   req_get_config__init
                     (ReqGetConfig         *message);
size_t req_get_config__get_packed_size
                     (const ReqGetConfig   *message);
size_t req_get_config__pack
                     (const ReqGetConfig   *message,
                      uint8_t             *out);
size_t req_get_config__pack_to_buffer
                     (const ReqGetConfig   *message,
                      ProtobufCBuffer     *buffer);
ReqGetConfig *
       req_get_config__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   req_get_config__free_unpacked
                     (ReqGetConfig *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Pluse_Closure)
                 (const Pluse *message,
                  void *closure_data);
typedef void (*ReqAuth_Closure)
                 (const ReqAuth *message,
                  void *closure_data);
typedef void (*AckCommon_Closure)
                 (const AckCommon *message,
                  void *closure_data);
typedef void (*TaskDesc_Closure)
                 (const TaskDesc *message,
                  void *closure_data);
typedef void (*ReqSetConfig_Closure)
                 (const ReqSetConfig *message,
                  void *closure_data);
typedef void (*ReqGetConfig_Closure)
                 (const ReqGetConfig *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor pluse__descriptor;
extern const ProtobufCMessageDescriptor req_auth__descriptor;
extern const ProtobufCMessageDescriptor ack_common__descriptor;
extern const ProtobufCMessageDescriptor task__desc__descriptor;
extern const ProtobufCMessageDescriptor req_set_config__descriptor;
extern const ProtobufCMessageDescriptor req_get_config__descriptor;

PROTOBUF_C_END_DECLS


#endif  /* PROTOBUF_common_5fmsg_2eproto__INCLUDED */
