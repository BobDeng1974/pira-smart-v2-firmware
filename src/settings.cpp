#include "settings.h"

settingsPacket_t settings_packet = { settings_packet.data.status_time = TIME_INIT_VALUE,
                                     settings_packet.data.status_battery = 0,
                                     settings_packet.data.safety_power_period = ON_PERIOD_INIT_VALUE_s,
                                     settings_packet.data.safety_sleep_period = OFF_PERIOD_INIT_VALUE_s,
                                     settings_packet.data.operational_wakeup = OFF_PERIOD_INIT_VALUE_s,
                                     settings_packet.data.safety_reboot = REBOOT_TIMEOUT_s,
                                     settings_packet.data.turnOnRpi = 0};

