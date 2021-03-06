/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: powermeasurement.proto */

#ifndef PROTOBUF_C_powermeasurement_2eproto__INCLUDED
#define PROTOBUF_C_powermeasurement_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003003 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _PowerMeasurement PowerMeasurement;


/* --- enums --- */


/* --- messages --- */

/*
 * Structure that contains the data representing a power consumption measurement
 * of a specific device, measured by the ESP32 powermeter.
 * 
 *  - timestamp              timestamp of the measurement, given in microseconds. This timestamp is
 *NOT based on UNIX epoch time, but on the elapsed time since start-up of 
 *the ESP32 power measurement node.
 *  - energy_consumption     total power consumption at the time of measurement, given in mAs.
 *  - current                the electrical current at the time of measurement, given in mA.
 */
struct  _PowerMeasurement
{
  ProtobufCMessage base;
  uint64_t timestamp;
  double energy_consumption;
  float current;
};
#define POWER_MEASUREMENT__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&power_measurement__descriptor) \
    , 0, 0, 0 }


/* PowerMeasurement methods */
void   power_measurement__init
                     (PowerMeasurement         *message);
size_t power_measurement__get_packed_size
                     (const PowerMeasurement   *message);
size_t power_measurement__pack
                     (const PowerMeasurement   *message,
                      uint8_t             *out);
size_t power_measurement__pack_to_buffer
                     (const PowerMeasurement   *message,
                      ProtobufCBuffer     *buffer);
PowerMeasurement *
       power_measurement__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   power_measurement__free_unpacked
                     (PowerMeasurement *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*PowerMeasurement_Closure)
                 (const PowerMeasurement *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor power_measurement__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_powermeasurement_2eproto__INCLUDED */
