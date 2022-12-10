from lib import Powermeter


PM_PORT = '/dev/ttyACM0'


pm = Powermeter(PM_PORT)

pm.set_averaging(64)
pm.set_frequency(1e9)

print(f'Diag: {pm.diag()}')
print(f'Power: {pm.measure():+.3f} dBm')
