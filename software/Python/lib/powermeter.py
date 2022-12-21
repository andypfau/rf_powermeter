import serial
import logging
from dataclasses import dataclass


MEM_MAX_ENTRIES = 1638

MEM_ADDR_CAL_DATA_COUNT        = 0x0000
MEM_ADDR_CAL_VAR_SHIFT         = 0x0002
MEM_ADDR_CAL_SLOPE_SHIFT       = 0x0004
MEM_ADDR_CAL_FREQ_DATA_START   = 0x4000
MEM_ADDR_CAL_SLOPE_DATA_START  = 0x8000
MEM_ADDR_CAL_OFFSET_DATA_START = 0xC000

CAL_VARIABLE_SHIFT = 1
CAL_SLOPE_SHIFT    = 8


def is_power_of_2(i: int) -> bool:
    return (i & (i-1)) == 0


class Powermeter:

    @dataclass
    class Diag:
        UsbV: float
        Analog5V: float
        TempC: float

        def __str__(self) -> str:
            return f'5V0USB = {self.UsbV:+.3f} V, 5V0A: {self.Analog5V:+.3f} V, Temperature: {self.TempC:+.2f} °C'


    def __init__(self, port: str, offline: bool = False):
        self.offline = offline
        self._connect(port)
    

    def _connect(self, port: str):
        if self.offline:
            return
        self.ser = serial.Serial(port=port, timeout=3.0)
        self.ser.write([0]) # start communication


    def _send(self, buf: str):
        self.ser.write(buf.encode('ASCII'))

    
    def _receive(self) -> str:
        buf = self.ser.read_until(b'\n')
        if len(buf) == 0:
            raise RuntimeError('No response after timeout')
        return buf.decode('ASCII').strip('\n')

    
    def _check_for_errors(self):
        
        if self.offline:
            return
        
        try:
            self._send('e')
            buf = self._receive()
            error = int(buf)
            if error != 0:
                raise RuntimeError(f'Device reported error code {error}')

        except Exception as ex:
            raise RuntimeError(f'Unable to query error code ({ex})')


    def _command(self, cmd: str):
        self._send(cmd)
        self._check_for_errors()


    def _query(self, qry: str) -> str:
        self._send(qry)
        result = self._receive()
        self._check_for_errors()
        return result
    

    def reset(self):
        self.set_averaging(16)
        self.set_frequency(10e6)
        self.enable_cal(True)

    
    def set_averaging(self, n: int):
        if n<1 or n>512 or not is_power_of_2(n):
            raise ValueError(f'Averaging must be a power of 2 in the range 1..512')
        self._command(f'a{n}\n')

    
    def set_frequency(self, hz: float):
        mhz = int(round(hz/1e6))
        if mhz<10 or mhz>6000:
            raise ValueError(f'Frequency must be 10..6000 MHz')
        self._command(f'f{mhz}\n')

    
    def enable_cal(self, enable: bool = True):
        self._command(f'l{"1" if enable else "0"}\n')

    
    def measure(self) -> float:
        
        if self.offline:
            return -99
        
        try:
            buf = self._query('t')
            db = float(buf)

        except Exception as ex:
            raise RuntimeError(f'Unable to query reading ({ex})')
        
        return db

    
    def diag(self) -> "Powermeter.Diag":
        
        if self.offline:
            v_usb = 5
            v_a = 5
            temp = 20
        
        else:
            try:
                buf = self._query('d')
                values = [float(part) for part in buf.split(';')]
                [v_usb, v_a, temp] = values

            except Exception as ex:
                raise RuntimeError(f'Unable to query diag ({ex})')
            
        return Powermeter.Diag(v_usb, v_a, temp)

    
    def _write_to_eeprom(self, address: int, data_16b: int):
        assert (address & 0xFFFF) == address, '<address> must be a 16 bit value'
        assert address % 2 == 0, '<address> not aligned to 16 bit word address'
        assert (data_16b & 0xFFFF) == data_16b, '<data_16b> must be a 16 bit value'
        logging.debug(f'EEPROM write [0x{address:04X}] <- 0x{data_16b:04X}')
        self._command(f'mw{address:04X}{data_16b:04X}\n')

    
    def read_from_eeprom(self, address: int) -> int:
        assert (address & 0xFFFF) == address, '<address> must be a 16 bit value'
        response = self._query(f'mr{address:04X}\n')
        data_16b = int(response, 16)
        logging.debug(f'EEPROM read [0x{address:04X}] -> 0x{data_16b:04X}')
        return data_16b
    

    def _write_and_verify_eeprom(self, buffer: "dict[int,int]"):
        
        buffer_size = len(buffer)
        
        for buffer_index,(address,data_written) in enumerate(buffer.items()):
            self._write_to_eeprom(address,data_written)
            if buffer_index % 1000 == 0:
                logging.debug(f'Writing to EEPROM, {buffer_index+1:,.0f}/{buffer_size:,.0f}')
        
        logging.debug(f'Writing to EEPROM done, verifying...')
        
        for buffer_index,(address,data_written) in enumerate(buffer.items()):
            data_read = self.read_from_eeprom(address)
            if data_read!=data_written:
                raise RuntimeError(f'EEPROM verification failed: data at address 0x{address:04X} is 0x{data_read:04X}, but should be 0x{data_written:04X}')
            if buffer_index % 1000 == 0:
                logging.debug(f'Verifying EEPROM, {buffer_index+1:,.0f}/{buffer_size:,.0f}')

        logging.debug(f'EEPROM verification done.')
    
    
    def _prepare_cal_data(self, frequencies_hz: "list[float]", errors_db: "list[float]") -> "tuple[int,int,list[int],list[int],list[int]]":
        
        if len(frequencies_hz) != len(errors_db):
            raise ValueError(f'Number of entries must be the same for both lists')
        if not (2 <= len(frequencies_hz) <= MEM_MAX_ENTRIES-1):
            raise ValueError(f'Number of entries must be 2..{MEM_MAX_ENTRIES}')

        freqs, slopes, offsets = [], [], []
        for i in range(1, len(frequencies_hz)):

            if frequencies_hz[i-1] >= frequencies_hz[i]:
                raise ValueError(f'Frequency list must be monotonic (check index {i})')

            f_mhz = frequencies_hz[i]/1e6
            err_mdb = 1e3 * errors_db[i]
            d_err_mdb = 1e3 * (errors_db[i] - errors_db[i-1])
            d_f_mhz = (frequencies_hz[i] - frequencies_hz[i-1]) / 1e6

            exp_var = 2**CAL_VARIABLE_SHIFT
            exp_slope = 2**CAL_SLOPE_SHIFT
            
            slope = d_err_mdb / d_f_mhz * exp_slope
            offset = (exp_slope * err_mdb - round(slope) * f_mhz) * exp_var

            freqs.append(int(round(f_mhz)))
            slopes.append(int(round(slope)))
            offsets.append(int(round(offset)))
        
        return CAL_VARIABLE_SHIFT, CAL_SLOPE_SHIFT, freqs, slopes, offsets


    def write_cal_cata(self, frequencies_hz: "list[float]", errors_db: "list[float]"):

        var_shift, slope_shit, freqs, slopes, offsets = self._prepare_cal_data(frequencies_hz, errors_db)
        buffer_size = len(freqs)

        assert 0 < buffer_size <= MEM_MAX_ENTRIES

        buffer = {}
        buffer[MEM_ADDR_CAL_DATA_COUNT] = buffer_size
        buffer[MEM_ADDR_CAL_VAR_SHIFT] = var_shift
        buffer[MEM_ADDR_CAL_SLOPE_SHIFT] = slope_shit
        for buffer_index,freq in enumerate(freqs):
            buffer[MEM_ADDR_CAL_FREQ_DATA_START + buffer_index*2] = freq
        for buffer_index,slope in enumerate(slopes):
            buffer[MEM_ADDR_CAL_SLOPE_DATA_START + buffer_index*4 + 0] = (slope>>16)&0xFFFF
            buffer[MEM_ADDR_CAL_SLOPE_DATA_START + buffer_index*4 + 2] = (slope>> 0)&0xFFFF
        for buffer_index,offset in enumerate(offsets):
            buffer[MEM_ADDR_CAL_OFFSET_DATA_START + buffer_index*4 + 0] = (offset>>16)&0xFFFF
            buffer[MEM_ADDR_CAL_OFFSET_DATA_START + buffer_index*4 + 2] = (offset>> 0)&0xFFFF

        self._write_and_verify_eeprom(buffer)
