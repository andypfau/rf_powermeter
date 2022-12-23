from lib import Powermeter
import argparse


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('port', type=str)
    args = parser.parse_args()
    port = args.port

    pm = Powermeter(port)
    pm.reset()
    print(pm.diag())
    print(f'Power: {pm.measure():+.3f} dBm')
