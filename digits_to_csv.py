import ROOT
import pandas as pd

F_IN = '../data/trddigits.root'
F_OUT = 'digits_sorted_single_event.csv'

def convert_digits():
	f_in = ROOT.TFile.Open(F_IN)
	tree = f_in.Get('o2sim')

	data = []
	for event in tree:
		digits = event.TRDDigit
		for d in digits:
			row = {
				'hcid': d.getHCId(),
				'padrow': d.getPadRow(),
				'pad': d.getPadCol(),
				'ADC_sum': d.getADCsum(),
			}

			adc_array = d.getADC()
			for i, adc in enumerate(adc_array):
				row['adc_%s' % i] = adc

			data.append(row)

	df = pd.DataFrame(data, columns=row.keys())

	return df

def main():
	digits_df = convert_digits()
	digits_df.to_csv(F_OUT, index=False)

if __name__ == '__main__':
	main()