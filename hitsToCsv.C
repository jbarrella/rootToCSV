using namespace std;

void hitsToCsv(string outfile = "hits.csv")
{
    ofstream hitsOut(outfile);

    TFile *fin = TFile::Open("o2sim_HitsTRD.root");
    TTree *tree = (TTree *)fin->Get("o2sim");
    vector<o2::trd::Hit> *hits = nullptr;
    tree->SetBranchAddress("TRDHit", &hits);

    auto &ccdbmgr = o2::ccdb::BasicCCDBManager::instance();
    // all o2-sims use values from this run
    ccdbmgr.setTimestamp(297595);
    auto calibrations = ccdbmgr.get<o2::trd::ChamberCalibrations>("TRD_test/ChamberCalibrations");

    auto geo = o2::trd::Geometry::instance();
    geo->createPadPlaneArray();

    hitsOut << "charge;localC;localR;localT;x;y;z;detector;pad;padrow;timebin;calPad;vdrift" << endl;

    int nev = tree->GetEntries();
    for (int iev = 0; iev < nev; ++iev)
    {
        tree->GetEvent(iev);
        for (const auto &hit : *hits)
        {
            float localR = hit.getLocalR();
            float localC = hit.getLocalC();
            float localT = hit.getLocalT();
            auto charge = hit.GetCharge();
            float x = hit.GetX();
            float y = hit.GetY();
            float z = hit.GetZ();
            int detector = hit.GetDetectorID();

            auto padPlane = geo->getPadPlane(detector);
            float padrow = padPlane->getPadRow(localR);
            float pad = padPlane->getPad(localC, localR);

            // Calculate time bin
            float vdrift = calibrations->getVDrift(detector);
            double t0 = calibrations->getT0(detector);

            double timebin;
            // x = 0 at anode plane and points toward pad plane.
            if (localT < -geo->camHght() / 2)
            {
                // drift region
                timebin = t0 - (localT + geo->camHght() / 2) / (vdrift * 0.1);
            }
            else
            {
                // anode region: very rough guess
                timebin = t0 - 1.0 + fabs(localT);
            }

            // lorentz correction to pad position
            double exb = calibrations->getExB(detector);
            double shift = geo->cdrHght() * TMath::Tan(exb);
            double correctedY = localC + shift;
            float calPad = padPlane->getPad(correctedY, localR);

            hitsOut << charge << ";" << localC << ";" << localR << ";" << localT << ";" << x
                    << ";" << y << ";" << z << ";" << detector << ";" << pad << ";" << padrow
                    << ";" << timebin << ";" << calPad << ";" << vdrift << endl;
        }
    }
}