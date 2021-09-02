import ROOT
import pandas as pd

F_RAW_IN = 'trdtracklets.root'
F_CALIBRATED_IN = 'trdcalibratedtracklets.root'
F_OUT = 'tracklets_single_event.csv'

def convert_raw_tracklets():
	f_in = ROOT.TFile.Open(F_RAW_IN)
	tree = f_in.Get('o2sim')

	data = []
	for event in tree:
		tracklets = event.Tracklet
		for t in tracklets:
			row = {
				'padrow': t.getPadRow(),
				'hcid': t.getHCID(),
				'column': t.getColumn(),
				'position': t.getPosition(),
				'slope': t.getSlope(),
				'uncal_y': t.getUncalibratedY(),
				'uncal_dy': t.getUncalibratedDy(),
				'detector': t.getDetector()
			}

			data.append(row)

	df = pd.DataFrame(data)

	return df

def convert_c_tracklets():
	f_in = ROOT.TFile.Open(F_CALIBRATED_IN)
	tree = f_in.Get('ctracklets')

	data = []
	for event in tree:
		tracklets = event.CTracklets
		for t in tracklets:
			row = {
				'x': t.getX(),
				'y': t.getY(),
				'z': t.getZ(),
				'dy': t.getDy(),
			}

			data.append(row)

	df = pd.DataFrame(data)

	return df

def main():
	raw_df = convert_raw_tracklets()
	c_df = convert_c_tracklets()

	merged = pd.concat([raw_df, c_df], axis=1)
	merged.to_csv(F_OUT, index=False)

if __name__ == '__main__':
	main()
	