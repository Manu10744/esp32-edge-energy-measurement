# Generated by the protocol buffer compiler.  DO NOT EDIT!
# sources: powermeasurement.proto
# plugin: python-betterproto
from dataclasses import dataclass

import betterproto


@dataclass
class PowerMeasurement(betterproto.Message):
    """
    Structure that contains the data representing a power consumption
    measurement of a specific device, measured by the ESP32 powermeter.   -
    timestamp              timestamp of the measurement, given in microseconds.
    This timestamp isNOT based on UNIX epoch time, but on the elapsed time
    since start-up of the ESP32 power measurement node.  - energy_consumption
    total power consumption at the time of measurement, given in mAs.  -
    current                the electrical current at the time of measurement,
    given in mA.
    """

    timestamp: int = betterproto.uint64_field(1)
    energy_consumption: float = betterproto.double_field(2)
    current: float = betterproto.float_field(3)
