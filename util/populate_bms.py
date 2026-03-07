#!/bin/env ./.venv/bin/python3

# This utility generates fake BMS data for a battery pack with configurable
# period, lifetime, baud_rate, cell count, module count, type of connections,
# and battery cell specification.

import serial
import threading
import numpy as np
# from itertools import product
from dataclasses import dataclass
from time import sleep

@dataclass
class Environment:
    humidity: float             # Ambient humidity (locally constant)
    T: float                    # Ambient temperature (locally constant)
    K: float                    # Cooling's law constant (ambient-battery)

@dataclass
class BatteryCell:
    """ Class for storing information and simulating a single battery cell. """
    V_nominal: float    # V, Nominal voltage value [datasheet + manufacturer err]
    Q_max: float        # mAh, Battery capacity [datasheet + manufacturer err]
    mass: float         # kg, Mass of the battery
    Z_internal: float   # Ohm, Internal impedance (assumed to be REAL)
    efficiency: float   # 1, Efficiency of the operations of the battery
    thermal_cap: float  # J/(kg*deg Celcius), Thermal capacity for battery's material (not accurate)
    V: float            # V, Current battery cell voltage
    T: float            # degrees Celcius, Current battery cell temperature
    soc: float          # %, Current battery cell state of charge

    def __update_temp(self, I: float, dt:float, ambient: Environment):
        heat = I*I*self.Z_internal              # Joule heat power
        cool = ambient.K * (self.T - ambient.T) # loss to environment power
        dT = (heat - cool) * dt / self.thermal_cap / self.mass
        self.T += dT

    # Charge the battery cell with constant DC current I for some time dt (in sec)
    def charge(self, I: float, dt: float, ambient: Environment):
        self.soc = self.soc + I*(dt/3600)*self.efficiency/(self.Q_max/1000)*100
        self.soc = min(self.soc, 100)
        self.V = self.V_nominal + self.Z_internal*I
        self.__update_temp(I, dt, ambient)

    # Discharge the battery cell with some constant load resistance R_load for some time dt.
    def discharge(self, I_load: float, dt: float, ambient: Environment):
        self.soc = self.soc - I_load*dt*self.efficiency/(self.Q_max/1000)*100
        self.soc = max(self.soc, 0)
        self.V = self.V_nominal - I_load * self.Z_internal
        self.__update_temp(I_load, dt, ambient)

    # Leave the battery cell unconnected to anything for some time dt - its idle
    # state. Here, it might lose some charge due to unforseen factors, but we
    # ignore it in the simulation.
    def idle(self, dt: float, ambient: Environment):
        self.__update_temp(0, dt, ambient)

# Generate real BatteryCells given information from a datasheet of some
# particular battery cell. In this case, the battery cell information is that of
# LiFePO4.
def generate_real_cells(number: int, ambient: Environment) -> list[BatteryCell]:
    cells = []
    for i in range(number):
        # use Gaussian distribution for manufacturing errors.
       v_nom = np.random.normal(3.2, 0.06) # 2% error
       cell = BatteryCell(
           V_nominal=v_nom,
           Q_max=np.random.normal(1600, 80),           # 5% error
           Z_internal=np.random.normal(0.040, 0.002),  # 5% error
           efficiency=np.random.normal(0.95, 0.01),    # 1% error
           mass=np.random.normal(0.041, 0.001),        # 0.1% error
           thermal_cap=1130,                           # J/(kg·°C), constant
           V=v_nom,                                    # initial voltage ~ OCV
           T=ambient.T + np.random.normal(0, 2),       # spread around ambient
           soc=np.random.uniform(70, 95),              # start partially charged
       )
       cells.append(cell)
    return cells

@dataclass
class Topology:
    number: int
    parallel: int
    series: int

@dataclass
class Connection:
    module: Topology            # Individual BatteryCells connections
    pack: Topology              # Individual modules connections

def series_voltage(emf: list[float]) -> float:
    """Returns the combined voltage of N cells (emf[i]) that are
    connected in series."""
    return np.sum(emf)

def series_impedance(imp: list[float]) -> float:
    """Returns the combined impedance of N cells (imp[i]) that are connected in
    series."""
    return np.sum(imp)

def parallel_voltage(emf: list[float], imp: list[float], R_load: float) -> float:
    r"""Returns the combined voltage of N cells (emf[i] and imp[i]) that are
    connected in parallel to some load resistance R_load. The formula was
    derived using KCL, KVL, and Sherman-Morrison formula. The LaTeX version for
    the formula is:

    $$V = \sum_{k=1}^N \left( Z_k^{-1} \left( R_L - \frac{R_L}{\alpha +
    \frac{1}{R_L}} Z_k^{-1} \right) \mathcal{E}_k - \frac{R}{\alpha +
    \frac{1}{R}} \sum_{\substack{i=1 \ i \neq k}}^N Z_i^{-1} Z_k^{-1}
    \mathcal{E}_i \right), \alpha = Z_1^{-1} + \dots + Z_N^{-1}$$
    """
    if len(emf) != len(imp):
        raise ValueError("len(emf) must equal len(imp)")
    if np.abs(R_load) < 0.001:
        raise ValueError("load resistance must not be 0")
    N = len(emf)

    result = 0

    alpha = np.sum([1/Z for Z in imp])
    coef  = R_load / (alpha + 1 / R_load)
    for k in range(N):
        result += 1/imp[k]*(R_load - coef * 1/imp[k])*emf[k]

        temp = 0
        for i in range(N):
            if i == k:
                continue
            temp += 1/imp[i]*1/imp[k]*emf[i]

        result -= coef * temp

    return result


@dataclass
class BatteryPack:
    connection: Connection
    temps: int                  # per module
    pcb_temps: int              # overall, in the entire PCB

    def cell_count(self) -> int:
        return self.connection.pack.number * self.connection.module.number

    def cell_current(self, current: float) -> float:
        """ Returns the current that charges the individual cell given the total
        current used to charge the entire battery pack. """
        return current / self.connection.pack.parallel                         \
                       / self.connection.module.parallel

    def cell_index(self, module_idx, parallel_idx, series_idx) -> int:
        """Returns the linearized cell index given its module index, parallel
        index, and series index."""
        inner_idx = self.connection.module.series * parallel_idx + series_idx
        outer_idx = self.connection.module.number * module_idx + inner_idx
        return outer_idx

    def module_thevenin(self, cells: list[BatteryCell], module_idx: int) -> (float, float):
        """Calculate the Thevenin's equivalent to the battery pack module"""
        # collect all the voltages that the series-cells create
        voltages = []
        impedances = []
        for parallel_idx in range(self.connection.module.parallel):
            emf = [cells[self.cell_index(module_idx, parallel_idx, series_idx)].V
                   for series_idx in range(self.connection.module.series)]
            imp = [cells[self.cell_index(module_idx, parallel_idx, series_idx)].Z_internal
                   for series_idx in range(self.connection.module.series)]
            voltages.append(series_voltage(emf))
            impedances.append(series_impedance(imp))

        # Use Millman's theorem
        impedance = 1 / np.sum([1 / z for z in impedances]) 
        voltage = impedance * np.sum([e / z for (e, z) in zip(voltages, impedances)])
        return voltage, impedance

    def pack_voltage(self, cells: list[BatteryCell], R_load: float) -> float:
        """Calculates the total pack voltage given the configuration of [cells]"""
        module_emfs = []
        module_imps = []

        for parallel_idx in range(self.connection.pack.parallel):
            branch_voltage, branch_impedance = 0.0, 0.0
            for series_idx in range(self.connection.pack.series):
                module_idx = parallel_idx * self.connection.pack.series + series_idx
                m_v, m_z = self.module_thevenin(cells, module_idx)
                branch_voltage += m_v
                branch_impedance += m_z
            module_emfs.append(branch_voltage)
            module_imps.append(branch_impedance)

        return parallel_voltage(module_emfs, module_imps, R_load)

@dataclass
class RCD:
    """ Holds information about the charging residual current device. """
    current: float              # A, DC charge current

class BMS:
    def __init__(self, lifetime, period, device_name, baud_rate):
        self.lifetime = lifetime       # BMS works for this many seconds.
        self.period = period           # BMS sends data at frequency 1/period Hz
        self.device_name = device_name # BMS file device path
        self.baud_rate = baud_rate     # UART baud rating (i.e., bps)
        try:
            self.device = serial.Serial(f"{self.device_name}", self.baud_rate,
                                        timeout=1)
        except serial.serialutil.SerialException as err:
            print(err)
            print("NOTE: Try running ./start_comm.sh")
            exit(1)

# Simulator variable initializations
ambient = Environment(65.0, 19.0, 0.06)
bms = BMS(1000, 1, "./ttyVBMS", 115200)
battery = BatteryPack(Connection(Topology(40, 4, 10), Topology(4, 1, 4)), 12, 4)
cells = generate_real_cells(battery.cell_count(), ambient)
rcd = RCD(10)                   # A, we charge the battery using this current
R_load = 30000                  # Ohm, load resistance
current_mode = 'idle'
running = True

def generate_bms_msg(battery, cells, load, ambient):
    """ Constructs a comma-separated ASCII string of floats %.2f BMS data.
    Order: cell voltages, cell temps, battery average temperature, pcb temps,
    humidity, voltage low, voltage pack, current low, current pack, max temp,
    min temp, max bms temp, min bms temp, avg temp, max cell volt, min cell
    volt, state of charge. """
    cell_voltages = [c.V for c in cells]

    # NOTE: because I don't know the exact placement of temperature sensors, I
    # take the temperature information from the first couple of cells in each
    # module.
    module_size = battery.connection.module.number
    selected_indices = []
    for m in range(battery.connection.pack.number):
        start = m * module_size
        selected_indices.extend(range(start, start + battery.temps))
    cell_temps = [cells[i].T for i in selected_indices]

    avg_cell_temp = np.mean(cell_temps)
    total_series = battery.connection.pack.series * battery.connection.module.series
    voltage_pack = battery.pack_voltage(cells, R_load)
    pcb_temps = np.random.normal(ambient.T + 5.0, 0.5, battery.pcb_temps).tolist()
    voltage_low = 12.1
    current_low = 1.2

    # Compute pack current based on mode
    parallel_total = battery.connection.pack.parallel * battery.connection.module.parallel
    if current_mode == "charge":
        pack_current = rcd.current
    elif current_mode == "discharge":
        pack_current = voltage_pack / load
    else:
        pack_current = 0.0

    max_cell_v = np.max(cell_voltages)
    min_cell_v = np.min(cell_voltages)
    max_temp = np.max(cell_temps)
    min_temp = np.min(cell_temps)
    max_bms_temp = np.max(pcb_temps)
    min_bms_temp = np.min(pcb_temps)
    avg_total_temp = np.mean(np.concatenate([cell_temps, pcb_temps]))
    avg_soc = np.mean([c.soc for c in cells])

    data_points = (
        *cell_voltages,        # 160 values
        *cell_temps,           # 48 values
        avg_cell_temp,         # 1 value
        *pcb_temps,            # 4 values
        ambient.humidity,
        voltage_low,
        voltage_pack,
        current_low,
        pack_current,
        max_temp,
        min_temp,
        max_bms_temp,
        min_bms_temp,
        avg_total_temp,
        max_cell_v,
        min_cell_v,
        avg_soc
    )

    return ",".join(f"{val:.2f}" for val in data_points) + ","

def simulation(bms, battery, cells, rcd, load, ambient):
    """Generates fake BMS messages based on the [current_mode] and writes to
    the appropriate device"""
    global current_mode, running
    
    print("Simulation started.")
    for _ in np.arange(0, bms.lifetime, bms.period):
        if not running:         # simulation stopped
            break

        for cell in cells:
            if current_mode == "charge":
                cell.charge(battery.cell_current(rcd.current), bms.period, ambient)
            elif current_mode == "discharge":
                V_pack = battery.pack_voltage(cells, load)
                I_pack = V_pack / load
                I_cell = battery.cell_current(I_pack)
                cell.discharge(I_cell, bms.period, ambient)
            else:               # current_mode == "idle"
                cell.idle(bms.period, ambient)

        msg = generate_bms_msg(battery, cells, load, ambient)
        bms.device.write(msg.encode('ascii'))
        sleep(bms.period)
    print("Simulation finished.")

def repl():
    """Run REPL mode for user to input the operation mode (idle, charge,
    discharge, quit, connect). """
    global current_mode, running
    
    print("Starting REPL: 'idle [default]', 'charge', 'discharge', 'quit', 'connect'")
    while True:
        cmd = input("BMS> ").strip().lower()
        if cmd == 'quit':
            running = False
            break
        elif cmd in ['idle', 'charge', 'discharge']:
            current_mode = cmd
            print(f"Mode changed to: {current_mode}")
        else:                   # current_mode == 'connect'
            pass                # TODO: handle this part
    print("Finished REPL.")

if __name__ == "__main__":
    thread = threading.Thread(
        target=simulation,
        args=(bms, battery, cells, rcd, R_load, ambient),
        daemon=True
    )

    # Start the simulation and the REPL
    thread.start()
    repl()

    print("Exiting the simulator...")
