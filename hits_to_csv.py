import ROOT
import pandas as pd

F_IN = 'o2sim_HitsTRD.root'
F_OUT = 'hits_single_event2.csv'

def convert_hits():
	f_in = ROOT.TFile.Open(F_IN)
	tree = f_in.Get('o2sim')

	data = []
	for event in tree:
		hits = event.TRDHit
		for h in hits:
			row = {
				'localC': h.getLocalC(),
				'localR': h.getLocalR(),
				'localT': h.getLocalT(),
				'charge': h.GetCharge(),
				'x': h.GetX(),
				'y': h.GetY(),
				'z': h.GetZ(),
                'detector': h.GetDetectorID()
			}

			data.append(row)

	df = pd.DataFrame(data)

	return df

def main():
	hits_df = convert_hits()
	hits_df.to_csv(F_OUT, index=False)

if __name__ == '__main__':
	main()
