RF Design
=========

## PCB Stackup

The PCB was produced with the 4-layer stackup from [Aisler](https://aisler.net/) (as of 2022):

| Layer | Base Th. | Final Th. | Material                  | εr   | Df    |
|-------|----------|-----------|---------------------------|------|-------|
| Mask  |          |           | Elppemer AS 2467 SM-DG    | 3.70 |       |
| Top   | 18 µm    | 40 µm     | Copper                    |      |       |
|       | 69 µm    | 68.13 µm  | Prepreg 1080 / R-1551 (W) | 4.30 | 0.013 |
|       | 69 µm    | 68.13 µm  | Prepreg 1080 / R-1551 (W) | 4.30 | 0.013 |
| 2     | 35 µm    | 35 µm     | Copper                    |      |       |
|       | 1130 µm  | 1130 µm   | FR4 Core / R-1566 (W)     | 4.60 | 0.010 |
| 3     | 35 µm    | 35 µm     | Copper                    |      |       |
|       | 69 µm    | 68.13 µm  | Prepreg 1080 / R-1551 (W) | 4.30 | 0.013 |
|       | 69 µm    | 68.13 µm  | Prepreg 1080 / R-1551 (W) | 4.30 | 0.013 |
| Bot.  | 18 µm    | 40 µm     | Copper                    |      |       |
| Mask  |          |           | Elppemer AS 2467 SM-DG    | 3.70 |       |


The dielectric parameters are in the range from 2..6 GHz; for more details, [see here](https://industrial.panasonic.com/ww/products/pt/halogen-free/models/R-1566%25252FR-1551).


## RF Structures

The RF structures were simulated and optimized in [CST](https://www.3ds.com/products-services/simulia/products/cst-studio-suite/).

### Coplanar Waveguide

S-Parameters of a 20 mm trace:

<img src="./img/sim_cpwg_spar.png" style="margin-left: 2em" />

<img src="./img/sim_cpwg_spar_il.png" style="margin-left: 2em" />

| Frequency | Loss        |
|-----------|-------------|
| 1 GHz     | 0.049 dB/cm |
| 6 GHz     | 0.16 dB/cm  |
| 8 GHz     | 0.20 dB/cm  |

Geometry:

<img src="./img/sim_cpwg_3d.png" style="margin-left: 2em" />

<img src="./img/sim_cpwg_dim.png" style="margin-left: 2em" />

#### Sensitivity

Swept Parameters:

- Tline width: ±0.3 mm
- Prepreg thickness: ±20 µm
- Prepreg Dk: ±0.1 → almost no impact

<img src="./img/sim_cpwg_spar_sens1.png" style="margin-left: 2em" />

Swept Parameters:

- Via diameter: 0.25 mm, 0.50 mm
- Via annular ring: 0.25 mm, 0.50mm
- Gap between annular rings is fixed at 0.025 mm.

<img src="./img/sim_cpwg_spar_sens2.png" style="margin-left: 2em" />


### SMA Transition

S-Parameters:

<img src="./img/sim_sma_spar.png" style="margin-left: 2em" />

Geometry:

<img src="./img/sim_sma_3d.png" style="margin-left: 2em" />

<img src="./img/sim_sma_dim1.png" style="margin-left: 2em" />

<img src="./img/sim_sma_dim2.png" style="margin-left: 2em" />


### 0402 Resistors

S-Parameters:

<img src="./img/sim_res_spar.png" style="margin-left: 2em" />

<img src="./img/sim_res_spar_il.png" style="margin-left: 2em" />

Impact of GND-Cutout:

<img src="./img/sim_res_spar_gndcut.png" style="margin-left: 2em" />

Geometry:

<img src="./img/sim_res_3d1.png" style="margin-left: 2em" />

<img src="./img/sim_res_3d2.png" style="margin-left: 2em" />

Top Cu:

<img src="./img/sim_res_dim1.png" style="margin-left: 2em" />

Inner (GND) Cu:

<img src="./img/sim_res_dim2.png" style="margin-left: 2em" />


## QFN Transition


S-Parameters:

<img src="./img/sim_qfn_spar.png" style="margin-left: 2em" />

Geometry:

<img src="./img/sim_qfn_3d.png" style="margin-left: 2em" />

<img src="./img/sim_qfn_dim1.png" style="margin-left: 2em" />

<img src="./img/sim_qfn_dim2.png" style="margin-left: 2em" />
