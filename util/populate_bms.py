#!/bin/env ./.venv/bin/python3

# This utility generates fake BMS data for a battery pack with configurable
# period, lifetime, baud_rate, cell count, module count, type of connections,
# and battery cell specification.

import serial
import numpy as np
from time import sleep

# BMS Information
lifetime = 1000                 # BMS works for this many seconds.
period = 0.005                  # BMS sends data with this period.
device = "./ttyVBMS"
baud_rate = 115200

# Battery Pack Information
connection = {
    "module": {
        "parallel": 1,          # all cells in the module are in series
        "series": 40
    },
    "pack": {
        "parallel": 4,          # all modules in the pack are in parallel
        "series": 1
    }
}
modules = 4
cells = 40
temps = 12
pcb_temps = 4                   # BMS PCB temperature sensors 

# Battery Cell Information: LiFePO4 18650-3.2V-1600mAh
nominal_cell_voltage = 3.2      # V
charge_temperature = 40         # [0-55] degrees Celcius
discharge_temperature = 35      # [-20-60] degrees Celcius
storage_temperature = 25        # Room temperature, [-20-55] degrees Celcius
normal_pcb_temperature = 30     # ambient temperature + pcb heat

# Calculate the expected `nominal` voltage of the battery pack that consists
# of [modules] and [cells] connected as specified in [connection]
def connected_cell_voltage():
    module_voltage = connection["module"]["series"] * nominal_cell_voltage
    pack_voltage = connection["pack"]["series"] * module_voltage
    return pack_voltage

# Battery pack output
battery_low_voltage  = 12.0                     # to power low-voltage devices
battery_high_voltage = connected_cell_voltage() # to power e.g. the motor
battery_low_current = 5.0
battery_high_current = 120

# Environment information
humidity = 65.00

# Generates a fake BMS data chunk: one such chunk is a string of comma-separated
# floating point values formatted using %.2f and where each symbol is encoded
# in ASCII. The order of data is as follows:
#   - cell voltage (modules * cells)
#   - cell temperatures (modules * temps)
#   - Battery Average Temp
#   - PCB Temp 0, 1, 2, 3
#   - Humidity
#   - voltageLow
#   - voltagePack
#   - currentLow
#   - currentPack
#   - maxTemp
#   - minTemp
#   - maxBMSTemp
#   - minBMSTemp
#   - avgTemp
#   - maxCellVolt
#   - minCellVolt
#   - SoC
# [mode] indicates the mode of operation of the BMS: charging, discharging, idle.
def fake_bms_msg():
    cell_voltage = np.random.normal(nominal_cell_voltage, 0.15, modules*cells)
    cell_temp_count = modules * temps
    cell_temp = [storage_temperature]
    battery_avg_temp = np.mean(cell_temp) # Spatial mean of cell temps
    pcb_temp = np.random.normal(normal_pcb_temperature, 5.0, pcb_temps)
    voltage_low = np.random.normal(battery_low_voltage, 0.5, 1)[0]
    voltage_pack = np.random.normal(connected_cell_voltage(), 0.5, 1)[0]
    current_low = battery_low_current + np.random.uniform(-0.1, 0.1)
    current_pack = np.random.normal(battery_high_current, 0.3, 1)[0]
    max_temp = np.max(cell_temp)
    min_temp = np.min(cell_temp)
    BMS_temp = np.random.normal(np.mean(pcb_temp), 0.2, 10)
    max_BMS_temp = np.max(BMS_temp)
    min_BMS_temp = np.min(BMS_temp)
    avg_temp = np.mean(np.concatenate([pcb_temp, cell_temp, BMS_temp]))
    max_cell_voltage = np.max(cell_voltage)
    min_cell_voltage = np.min(cell_voltage)
    soc = 85.0                  # TODO: @hardcoded

    result = []
    result.extend(cell_voltage)     # 160
    result.extend(cell_temp)        # 48
    result.append(battery_avg_temp) # 1
    result.extend(pcb_temp)         # 4
    result.append(humidity)         # 1
    result.append(voltage_low)      # 1
    result.append(voltage_pack)     # 1
    result.append(current_low)      # 1
    result.append(current_pack)     # 1
    result.append(max_temp)         # 1
    result.append(min_temp)         # 1
    result.append(max_BMS_temp)     # 1
    result.append(min_BMS_temp)     # 1
    result.append(avg_temp)         # 1
    result.append(max_cell_voltage) # 1
    result.append(min_cell_voltage) # 1
    result.append(soc)              # 1

    return ",".join([f"{val:.2f}" for val in result]) + ","

# Open the serial communication port to the BMS virtual device
try:
    bms = serial.Serial(f"{device}", baud_rate, timeout=1)
except serial.serialutil.SerialException as err:
    print(err)
    print("NOTE: Try running ./start_comm.sh")
    exit(1)

# Populate the BMS with data
print("Begin populating the BMS with data...")

for count in np.arange(0, lifetime, period):
    msg = fake_bms_msg()
    bms.write(msg.encode('ascii'))
    sleep(period)

print("End populating the BMS with data.")
