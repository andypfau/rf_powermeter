import serial
import logging
from dataclasses import dataclass
import time
import json
from types import SimpleNamespace


MEM_MAX_ENTRIES = 1638

MEM_ADDR_CAL_DATA_COUNT        = 0x0000
MEM_ADDR_CAL_VAR_SHIFT         = 0x0002
MEM_ADDR_CAL_SLOPE_SHIFT       = 0x0004
MEM_ADDR_CAL_FREQ_DATA_START   = 0x4000
MEM_ADDR_CAL_SLOPE_DATA_START  = 0x8000
MEM_ADDR_CAL_OFFSET_DATA_START = 0xC000

CAL_VARIABLE_SHIFT = 1
CAL_SLOPE_SHIFT    = 8

ERROR_CODES = {
    11: 'Invalid input',
    21: 'Temperature sensor read failed',
    22: 'I2C busy',
    23: 'Mem write rejected',
    24: 'Mem write failed',
    25: 'Mem read rejected',
    26: 'Mem read failed',
}


EEPROM_TWR_S = 5e-3


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


    def __init__(self, port: str, offline: bool = False, timeout_s: float = 3.0):
        self.offline = offline
        self._connect(port, timeout_s)
    

    def _connect(self, port: str, timeout_s: float):
        if self.offline:
            return
        self.ser = serial.Serial(port=port, timeout=timeout_s)
        self.ser.write([0]) # start communication


    def _send(self, buf: str):
        self.ser.write(buf.encode('ASCII'))

    
    def _receive(self) -> str:
        if self.offline:
            return ''
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
                if error in ERROR_CODES:
                    message = ERROR_CODES[error]
                    raise RuntimeError(f'Device reported error "{message}" (code {error})')
                else:
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
        self._command(f'a{n}\n')

    
    def set_frequency(self, hz: float):
        mhz = int(round(hz/1e6))
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

    
    def write_to_eeprom(self, address: int, data_16b: int):
        if self.read_from_eeprom(address) == data_16b:
            logging.debug(f'Skipping EEPROM write [0x{address:04X}] <- 0x{data_16b:04X} (data already matches)')
            return
        assert (address & 0xFFFF) == address, '<address> must be a 16 bit value'
        assert (data_16b & 0xFFFF) == data_16b, '<data_16b> must be a 16 bit value'
        logging.debug(f'EEPROM write [0x{address:04X}] <- 0x{data_16b:04X}')
        self._command(f'mw{address:04X}{data_16b:04X}\n')
        time.sleep(EEPROM_TWR_S * 2.5) # ensure sufficient write cycle time

    
    def read_from_eeprom(self, address: int) -> int:
        assert (address & 0xFFFF) == address, '<address> must be a 16 bit value'
        response = self._query(f'mr{address:04X}\n')
        data_16b = int(response, 16)
        logging.debug(f'EEPROM read [0x{address:04X}] -> 0x{data_16b:04X}')
        return data_16b
    

    def write_and_verify_eeprom(self, buffer: "dict[int,int]"):
        
        buffer_size = len(buffer)
        
        for buffer_index, (address, data) in enumerate(buffer.items()):
            self.write_to_eeprom(address, data)
            data_read = self.read_from_eeprom(address)
            if data_read != data:
                raise RuntimeError(f'EEPROM verify [0x{address:04X}] -> 0x{data_read:04X} -> ERROR, should be 0x{data:04X}')
            if buffer_index % 1000 == 0:
                logging.info(f'Writing to EEPROM, {buffer_index+1:,.0f}/{buffer_size:,.0f}')
        
        logging.debug(f'Writing to EEPROM done, verifying...')
        
        for buffer_index, (address, data_written) in enumerate(buffer.items()):
            data_read = self.read_from_eeprom(address)
            if data_read != data_written:
                raise RuntimeError(f'EEPROM verify [0x{address:04X}] -> 0x{data_read:04X} -> ERROR, should be 0x{data_written:04X}')
            if buffer_index % 1000 == 0:
                logging.info(f'Verifying EEPROM, {buffer_index+1:,.0f}/{buffer_size:,.0f}')

        logging.info(f'EEPROM verification done.')
    
    
    def _prepare_cal_data(self, frequencies_hz: "list[float]", errors_db: "list[float]") -> "tuple[int,int,list[int],list[int],list[int]]":
        
        if len(frequencies_hz) != len(errors_db):
            raise ValueError(f'Number of entries must be the same for both lists')
        
        data_points = []
        previous_freq_hz = None
        previous_freq_mhz = None
        for f,e in zip(frequencies_hz, errors_db):
            
            frequency_mhz = int(round(f / 1e6))
            error_mdb = int(round(e * 1e3))
            
            if previous_freq_hz is not None:
                if f <= previous_freq_hz:
                    raise ValueError(f'List of frequencies must be monotonic')
            
            if frequency_mhz == previous_freq_mhz:
                # avoid duplicate entries
                continue
            
            previous_freq_hz = f
            previous_freq_mhz = frequency_mhz

            data_points.append(SimpleNamespace(f_hz = f, f_mhz = frequency_mhz, e_db = e, e_mdb = error_mdb))
        
        if not (2 <= len(frequencies_hz) <= MEM_MAX_ENTRIES-1):
            raise ValueError(f'Number of entries must be 2..{MEM_MAX_ENTRIES} (note that due to roudning of the MHz, some data points may be removed from the dataset)')

        freqs, slopes, offsets = [], [], []
        for i in range(len(data_points)-1):

            f_mhz = data_points[i].f_mhz
            err_mdb = data_points[i].e_mdb
            d_err_mdb = 1e3 * (data_points[i+1].e_db - data_points[i].e_db)
            d_f_mhz = (data_points[i+1].f_hz - data_points[i].f_hz) / 1e6

            exp_var = 2**CAL_VARIABLE_SHIFT
            exp_slope = 2**CAL_SLOPE_SHIFT
            
            slope = d_err_mdb / d_f_mhz * exp_slope
            offset = (exp_slope * err_mdb - round(slope) * round(f_mhz)) * exp_var

            freq_int = int(round(f_mhz))
            slope_int = int(round(slope))
            offset_int = int(round(offset))

            if not (0 <= freq_int <= 0xFFFF):
                raise ValueError(f'Frequency {frequencies_hz[i]} does not fit into unsigned 16 bit')
            if not (-0x7FFFFFFF <= slope_int <= +0x7FFFFFFE):
                raise ValueError(f'Slope for error-value {errors_db[i]} does not fit into signed 32 bit')
            if not (-0x7FFFFFFF <= offset_int <= +0x7FFFFFFE):
                raise ValueError(f'Offset for error-value {errors_db[i]} does not fit into signed 32 bit')

            freqs.append(freq_int)
            slopes.append(slope_int)
            offsets.append(offset_int)
        
        return CAL_VARIABLE_SHIFT, CAL_SLOPE_SHIFT, freqs, slopes, offsets


    def write_cal_cata(self, frequencies_hz: "list[float]", errors_db: "list[float]", dump_to_file: str = None):

        var_shift, slope_shift, freqs, slopes, offsets = self._prepare_cal_data(frequencies_hz, errors_db)
        
        buffer_size = len(freqs)
        assert 0 < buffer_size <= MEM_MAX_ENTRIES
        buffer = {}
        buffer[MEM_ADDR_CAL_DATA_COUNT] = buffer_size
        buffer[MEM_ADDR_CAL_VAR_SHIFT] = var_shift
        buffer[MEM_ADDR_CAL_SLOPE_SHIFT] = slope_shift
        for buffer_index,freq in enumerate(freqs):
            buffer[MEM_ADDR_CAL_FREQ_DATA_START + buffer_index*2] = freq
        for buffer_index,slope in enumerate(slopes):
            buffer[MEM_ADDR_CAL_SLOPE_DATA_START + buffer_index*4 + 0] = (slope>>16)&0xFFFF
            buffer[MEM_ADDR_CAL_SLOPE_DATA_START + buffer_index*4 + 2] = (slope>> 0)&0xFFFF
        for buffer_index,offset in enumerate(offsets):
            buffer[MEM_ADDR_CAL_OFFSET_DATA_START + buffer_index*4 + 0] = (offset>>16)&0xFFFF
            buffer[MEM_ADDR_CAL_OFFSET_DATA_START + buffer_index*4 + 2] = (offset>> 0)&0xFFFF
        

        if dump_to_file is not None:
            with open(dump_to_file, 'w') as fp:
                obj = dict(var_shift=var_shift, slope_shift=slope_shift, freqs=freqs, slopes=slopes, offsets=offsets, memory=buffer)
                obj_json = json.dumps(obj, indent='\t')
                fp.write(obj_json)
            return

        self.write_and_verify_eeprom(buffer)
