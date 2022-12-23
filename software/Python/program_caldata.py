from lib import Powermeter
import pandas as pd
import logging
import argparse


if __name__ == '__main__':

    logging.basicConfig(level=logging.INFO)

    parser = argparse.ArgumentParser()
    parser.add_argument('port', type=str)
    parser.add_argument('datafile', type=str)
    parser.add_argument('--verify', action='store_true')
    args = parser.parse_args()
    port = args.port
    datafile = args.datafile
    verify = args.verify

    try:

        error_data = pd.read_csv(datafile)

        # you probably have to adapt these lines
        f_hz = error_data['f/MHz'].to_numpy() * 1e6
        err_db = error_data['PErr/dB'].to_numpy()

        pm = Powermeter(port)
        pm.write_cal_cata(f_hz, err_db, verify_only=verify)
    
    except Exception as ex:
        logging.exception(ex)
