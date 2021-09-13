using namespace std;

void trackletsToCsv(string outfile = "tracklets.csv")
{
    ofstream trackletsOut(outfile);

    TFile *fTracklets = TFile::Open("trdtracklets.root");
    TTree *trkltTree = (TTree *)fTracklets->Get("o2sim");
    vector<o2::trd::Tracklet64> *tracklets = nullptr;
    trkltTree->SetBranchAddress("Tracklet", &tracklets);

    TFile *fCTracklets = TFile::Open("trdcalibratedtracklets.root");
    TTree *cTrkltTree = (TTree *)fCTracklets->Get("ctracklets");
    vector<o2::trd::CalibratedTracklet> *cTracklets = nullptr;
    cTrkltTree->SetBranchAddress("CTracklets", &cTracklets);

    auto &ccdbmgr = o2::ccdb::BasicCCDBManager::instance();
    // all o2-sims use values from this run
    ccdbmgr.setTimestamp(297595);
    auto calibrations = ccdbmgr.get<o2::trd::ChamberCalibrations>("TRD_test/ChamberCalibrations");

    auto geo = o2::trd::Geometry::instance();
    geo->createPadPlaneArray();

    trackletsOut << "column;detector;hcid;padrow;position;slope;uncal_dy;uncal_y;dy;x;y;z" << endl;

    int nev = trkltTree->GetEntries();
    for (int iev = 0; iev < nev; ++iev)
    {
        trkltTree->GetEvent(iev);
        cTrkltTree->GetEvent(iev);
        for (int iTrklt = 0; iTrklt < tracklets->size(); iTrklt++)
        {
            auto tracklet = (*tracklets)[iTrklt];
            auto cTracklet = (*cTracklets)[iTrklt];

            int padrow = tracklet.getPadRow();
            int hcid = tracklet.getHCID();
            int column = tracklet.getColumn();
            float position = tracklet.getPosition();

            int slope = tracklet.getSlope();
            int slopeSigned = 0;
            int NBITSTRKLSLOPE = 8;
            float PADGRANULARITYTRKLSLOPE = 1000.f;
            float GRANULARITYTRKLSLOPE = 1.f / PADGRANULARITYTRKLSLOPE;
            if (slope & (1 << (NBITSTRKLSLOPE - 1)))
            {
                slopeSigned = -((~(slope - 1)) & ((1 << NBITSTRKLSLOPE) - 1));
            }
            else
            {
                slopeSigned = slope & ((1 << NBITSTRKLSLOPE) - 1);
            }

            float uncal_y = tracklet.getUncalibratedY();
            float uncal_dy = tracklet.getUncalibratedDy();
            int detector = tracklet.getDetector();

            float x = cTracklet.getX();
            float y = cTracklet.getY();
            float z = cTracklet.getZ();
            float dy = cTracklet.getDy();

            float vdrift = calibrations->getVDrift(detector);
            double t0 = calibrations->getT0(detector);
            double exb = calibrations->getExB(detector);

            int stack = geo->getStack(detector);
            int layer = geo->getLayer(detector);
            auto padPlane = geo->getPadPlane(layer, stack);

            double padWidth = padPlane->getWidthIPad();

            // 3 cm
            float xCathode = geo->cdrHght();
            // 3.35
            float xAnode = geo->cdrHght() + geo->camHght() / 2;

            // dy = slope * nTimeBins * padWidth * GRANULARITYTRKLSLOPE;
            // nTimeBins should be number of timebins in drift region. 1 timebin is 100 nanosecond
            double rawDy = slopeSigned * ((xCathode / vdrift) * 10.) * padWidth * GRANULARITYTRKLSLOPE;

            // NOTE: check what drift height is used in calibration code to ensure consistency
            // NOTE: check sign convention of Lorentz angle
            // NOTE: confirm the direction in which vDrift is measured/determined. Is it in x or in direction of drift?
            double lorentzCorrection = TMath::Tan(exb) * xAnode;

            // assuming angle in Bailhache, fig. 4.17 would be positive in our calibration code
            double calibratedDy = rawDy - lorentzCorrection;

            trackletsOut << column << ";" << detector << ";" << hcid << ";" << padrow << ";" << position
                         << ";" << slopeSigned * GRANULARITYTRKLSLOPE << ";" << uncal_dy << ";" << uncal_y << ";" << calibratedDy << ";" << x
                         << ";" << y << ";" << z << endl;
        }
    }
}