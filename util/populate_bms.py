#!/bin/env ./.venv/bin/python3

import serial
import numpy as np
from time import sleep

# Generates fake BMS data chunk.
# Each value is either ASCII or float `%.2f`. There are no spaces.
# The format:
#   - Cell Voltage: 4 modules * 40 cells = 160 values    [float]
#   - Cell Temp:    4 modules * 12 temps = 48 values     [float]
#   - Battery Average Temp                               [float]
#   - PCB Temp 0, 1, 2, 3                                [float]
#   - Humidity                                           [float]
#   - voltageLow                                         [float]
#   - voltagePack                                        [float]
#   - currentLow                                         [float]
#   - currentPack                                        [float]
#   - maxTemp                                            [float]
#   - minTemp                                            [float]
#   - maxBMSTemp                                         [float]
#   - minBMSTemp                                         [float]
#   - avgTemp                                            [float]
#   - maxCellVolt                                        [float]
#   - minCellVolt                                        [float]
#   - SoC                                                [float]
# [mode] indicates the mode of operation of the BMS: charging, discharging, idle.
def fake_bms_msg(mode):
    cell_voltage = np.random.normal(3.70, 0.15, 160) # nominal cell voltage is 3.70V
    cell_temp = np.random.normal(32.0, 5.0, 48) if mode == "charging" else    \
                np.random.normal(43.0, 5.0, 48) if mode == "discharging" else \
                np.random.normal(23.0, 1.0, 48) # mode == "idle", so room temp
    battery_avg_temp = np.mean(cell_temp) # Spatial mean of cell temps
    pcb_temp = np.random.normal(30.0, 5.0, 4) # PCB Temperatures
    humidity = np.random.normal(65.0, 1.0, 1)[0]
    voltage_low = np.random.normal(12.1, 0.5, 1)[0] # after de-amplification
    voltage_pack = np.random.normal(np.sum(cell_voltage), 0.5, 1)[0] # all cell voltages
    current_low = 5.0 + np.random.uniform(-0.1, 0.1)
    current_pack = np.random.normal(120, 0.3, 1)[0] if mode == "discharging" else 0.0
    max_temp = np.max(cell_temp)
    min_temp = np.min(cell_temp)
    BMS_temp = np.random.normal(40, 0.2, 10)
    max_BMS_temp = np.max(BMS_temp)
    min_BMS_temp = np.min(BMS_temp)
    avg_temp = np.mean(np.concatenate([pcb_temp, cell_temp, BMS_temp]))
    max_cell_voltage = np.max(cell_voltage)
    min_cell_voltage = np.min(cell_voltage)
    soc = 85.0

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

lifetime  = 4                   # BMS works for [lifetime] seconds
period    = 1                   # T=1sec
device    = "./ttyVBMS"
baud_rate = 115200

# Open the serial communication port to the BMS virtual device
try:
    bms = serial.Serial(f"{device}", baud_rate, timeout=1)
except serial.serialutil.SerialException as err:
    print(err)
    print("NOTE: Try running ./start_comm.sh")
    exit(1)

# Populate the BMS with data
print("Begin populating the BMS with data...")

for count in range(0, lifetime + period, period):
    mode = "idle"
    msg = fake_bms_msg(mode)
    bms.write(msg.encode('ascii'))
    sleep(period)

print("End populating the BMS with data.")
