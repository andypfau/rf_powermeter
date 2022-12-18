from lib import Powermeter


PM_PORT = '/dev/ttyACM0'


pm = Powermeter(PM_PORT)
pm.reset()

MEM_ADDR_CAL_DATA_COUNT = 6
MEM_ADDR_CAL_FREQ_DATA_START = 8
MEM_ADDR_CAL_SLOPE_DATA_START = 184
MEM_ADDR_CAL_OFFSET_DATA_START = 532
#pm.write_to_eeprom(MEM_ADDR_CAL_DATA_COUNT, 42)

#pm.set_averaging(64)
#pm.set_frequency(1e9)
#
#print(f'Diag: {pm.diag()}')
#print(f'Power: {pm.measure():+.3f} dBm')

pm.enable_cal(False)
print(f'Power: {pm.measure():+.3f} dBm')
pm.enable_cal(True)
print(f'Power: {pm.measure():+.3f} dBm')
