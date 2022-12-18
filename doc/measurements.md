Measurements
============

## Summary

- Return Loss: -12 dB up to 6 GHz, which is good enough for a hobby project.
- Measurement error: no dips/resonances/etc, can easily be compensated with LUT.
- Linearity: around 1 dB error over a 20 dB range, which is more than expected, but acceptable for a hobby project.
- Generated noise: no significant noise detected.

The DUT reported a temperature of 38 °C during the measurement.


## RF Measurements

### Return Loss

<img src="./img/meas_rl.png" style="margin-left: 2em" />

Setup: powermeter → network analyzer.


### Measurement Error

<img src="./img/meas_perr.png" style="margin-left: 2em" />

Setup: signal source → power divider → powermeter (bench instrument) | powermeter (DUT).

There is a small systematic error, because there were different adapters on each leg of the divider.


#### Linearity

<img src="./img/meas_perr-vs-p.png" style="margin-left: 2em" />

<img src="./img/meas_perr-vs-p_norm.png" style="margin-left: 2em" />


### Generated Noise

<img src="./img/meas_noise_lf.png" style="margin-left: 2em" />

<img src="./img/meas_noise_hf.png" style="margin-left: 2em" />

Setup: powermeter → spectrum analyzer.

For the different frequency ranges, the measured noisefloor levels differ; this is because the RBW was changed, in order to speed up the measurement time. However, the measurements show that there is no change in noisefloor when turning the DUT on or off.


## DC Rail measurements

### Levels

- 5V0USB: 4.79 V
- 5V5: 5.64 V
- 5V0A: 4.99 V
- 3V3A: 3.29 V
- 3V3D: 3.29 V
- Ref: 2.998 V
- RF: 2.05 V (with RF input disconnected/open)

Measured with DMM.


### Switching Noise

- DC-DC output: around 30 mVpp, 2 MHz and harmonics
- LDOs output: around 25 mVpp, 2 MHz, harmonics below noise

Measured with Oscilloscope and 1 MΩ probe.


## Bench Instruments

- Network analyzer: Rohde & Schwarz ZVA40, 4 ports (calibrated with e-cal)
- Signal source: Keysight E8257D
- Power meter: Keysight N1914A (calibrated with internal zero+cal)
- Spectrum analyzer: Keysight N9041B
- Power divider: Keysight 11636C
- DMM: Fluke 175 (*out of cal*)
- Oscilloscope: Siglent SDS1204X-E (*out of cal*)
