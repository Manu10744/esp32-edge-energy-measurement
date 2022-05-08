import logging

from powermeasurement import PowerMeasurement
from prometheus_client import Counter, Gauge

logger = logging.getLogger(__name__)


class CounterWrapper:
    """ Wrapper for a prometheus counter in order to keep track of its value. """
    def __init__(self, prom_counter: Counter):
        self.counter = prom_counter
        self.counter_value = 0

    def set_value(self, new_value: float):
        self.counter_value = new_value

    def get_value(self):
        return self.counter_value


METRIC_NAME_TOTAL_CONSUMPTION = "powerexporter_power_consumption_ampere_seconds_total"
METRIC_DESC_TOTAL_CONSUMPTION = "The total power consumption given in ampere-seconds."

METRIC_NAME_CURRENT = "powerexporter_current_ampere"
METRIC_DESC_CURRENT = "The electric current given in ampere."

ENERGY_CONSUMPTION_COUNTER = CounterWrapper(
    Counter(METRIC_NAME_TOTAL_CONSUMPTION, METRIC_DESC_TOTAL_CONSUMPTION)
)

ELECTRICAL_CURRENT_GAUGE = Gauge(METRIC_NAME_CURRENT, METRIC_DESC_CURRENT)


def update_metrics(new_data: PowerMeasurement) -> None:
    """ Updates the power-related metrics based on the new data. """
    increment_counter_to(ENERGY_CONSUMPTION_COUNTER, new_data.energy_consumption / 1000)  # mAs -> As
    ELECTRICAL_CURRENT_GAUGE.set(new_data.current / 1000)  # mA -> A


def increment_counter_to(counter_wrapper: CounterWrapper, new_value: float) -> None:
    """
    Increments a prometheus counter up to the given `new_value`

    This is a workaround because the counter API does not support incrementing a counter up to a fixed value, but only
    increment by a certain value. This is a problem as we are only receiving the most recent value of the total power
    consumption and not its difference compared to the previous measurement.

    :param counter_wrapper: the prometheus counter to increment.
    :param new_value: the desired new value of the counter.
    """
    if new_value < 0 or new_value < counter_wrapper.get_value():
        logger.error("Illegal operation - Cannot increment counter to a negative or lower value!")
        return

    difference: float = new_value - counter_wrapper.get_value()
    counter_wrapper.counter.inc(difference)
    counter_wrapper.set_value(new_value)
