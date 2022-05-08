set(PROTOBUFFERS_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../protobuffers)

idf_component_register(SRCS "power_measurement.c" "powermeasurement.pb-c.c"
                       INCLUDE_DIRS "include"
                       REQUIRES ina3221 protobuf-c)
