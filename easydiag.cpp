#include <iostream>
#include <stdexcept>
#include <vector>
#include <limits>
#include <algorithm>
#include <gnuplot-iostream.h>

class Beam {
    public:
        double length;
        
    Beam(double l=0) {
        length = l;
    }
};
class PointLoad {
    public:
        double value;
        double distance;
    // Vertical and horizontal loads are appended to their respective context vectors by the frontend, or by the user.

    PointLoad(double v, double d) {
        value = v;
        distance = d;
    }
};
class PointMoment {
    public:
        double value;
        double distance;
        double moment() {
            return value*distance;
        }
    PointMoment(double v, double d) {
        value = v;
        distance = d;
    }
};
class DistributedLoad {
    public:
        double q1;
        double q2;
        double distance;
        double length;
    DistributedLoad(double val1, double val2, double d, double l) {
        q1 = val1;
        q2 = val2;
        distance = d;
        length = l;
    }
};

class Context {
    public:
        Beam beam;
        std::vector<PointLoad> verticalLoads;
        std::vector<PointLoad> horizontalLoads;
        std::vector<DistributedLoad> distributedLoads;
        std::vector<PointMoment> pointMoments;
        double dx = 0.0005;

    Context(Beam b,
            std::vector<PointLoad> v, 
            std::vector<PointLoad> h,
            std::vector<DistributedLoad> d, 
            std::vector<PointMoment> m) {
        beam = b;
        verticalLoads = v;
        horizontalLoads = h;
        distributedLoads = d;
        pointMoments = m;
    }

    // parts of the beam divided by the horizontal loads
    std::vector<std::pair<double, double>> hPoints() {
        std::vector<double> hp;
        std::vector<std::pair<double,double>> pairs;

        for (PointLoad &h : horizontalLoads) {
            hp.push_back(h.distance);
        };
        
        if (!std::count(hp.begin(),hp.end(),0)) {
            hp.push_back(0);
        }

        std::sort(hp.begin(), hp.end());
        for (int i = 0; i < hp.size()-1; i++) {
            std::pair<double, double> pair;
            if (i+1<=hp.size() && hp[i] != hp[i+1]) {
                pair.first = hp[i];
                pair.second = hp[i+1];
            }
            pairs.push_back(pair);
        }
        return pairs;
    }
    // parts of the beam divided by the vertical loads
    std::vector<std::pair<double, double>> vPoints() {
        std::vector<double> vp;
        std::vector<std::pair<double,double>> pairs;

        for (PointLoad &v : verticalLoads) {
            vp.push_back(v.distance);
        };
        for (DistributedLoad &q : distributedLoads) {
            vp.push_back(q.distance);
            vp.push_back(q.distance+q.length);
        }

        if (!std::count(vp.begin(),vp.end(),0)) {
            vp.push_back(0);
        }

        std::sort(vp.begin(), vp.end());
        for (int i = 0; i < vp.size()-1; i++) {
            std::pair<double, double> pair;
            if (i+1<=vp.size() && vp[i] != vp[i+1]) {
                pair.first = vp[i];
                pair.second = vp[i+1];
            }
            pairs.push_back(pair);
            // std::cout << "COPPIA: " << pair[0] << ", " << pair[1] << "\n";
        }
        return pairs;
    }
    // parts of the beam divided by the vertical loads and the point moments
    std::vector<std::pair<double,double>> mPoints() {
        std::vector<double> mp = {0};
        std::vector<std::pair<double,double>> pairs;

        for (PointLoad &v : verticalLoads) {
            mp.push_back(v.distance);
        };
        for (DistributedLoad &q : distributedLoads) {
            mp.push_back(q.distance);
            mp.push_back(q.distance+q.length);
        };
        for (PointMoment &m : pointMoments) {
            mp.push_back(m.distance);
        };
        
        if (!std::count(mp.begin(),mp.end(),0)) {
            mp.push_back(0);
        }        
        std::sort(mp.begin(), mp.end());
        for (int i = 0; i < mp.size()-1; i++) {
            std::pair<double, double> pair;
            if (i+1<=mp.size() && mp[i] != mp[i+1]) {
                pair.first = mp[i];
                pair.second = mp[i+1];
            }
            pairs.push_back(pair);
        }
        return pairs;
    }

    // where x is the distance from left
    double pointH(double x) {
        double hValue = 0;
        for (PointLoad &hl : horizontalLoads) {
            if (x == beam.length) {
                hValue -= hl.value;
            }
            else if (hl.distance<x) {
                hValue -= hl.value;
            }
        }
        return hValue;
    }
    double pointV(double x) {
        double vValue = 0;
        for (PointLoad &vl : verticalLoads) {
            if (x>=vl.distance) {
                vValue += vl.value;
            }
        }
        for (DistributedLoad &dl : distributedLoads) {
            // loads with w(x) = k
            double p1 = x-dl.distance;
            double p2;
            if (x<dl.distance+dl.length) {
                if (dl.q1 == dl.q2) {
                    vValue += dl.q1*p1;
                } else
                if (dl.q1 < dl.q2) {
                    p2 = p1*((dl.q2-dl.q1)/dl.length);
                    if (dl.q1==0) {
                        vValue -= p1*dl.q1;
                    }
                    vValue += p1*(dl.q1+(p2/2));
                } else
                if (dl.q1 > dl.q2) {
                    p2 = p1*((dl.q1-dl.q2)/dl.length);
                    vValue += p1*(dl.q1-(p2/2));
                }
            }
            if (x>=dl.distance+dl.length) {
                if (dl.q1 == dl.q2) {
                    vValue += dl.q1*dl.length;
                } else
                if (dl.q1 < dl.q2) {
                    p2 = p1*((dl.q2-dl.q1)/dl.length);
                    vValue += ((dl.q1+dl.q2)*dl.length)/2;
                } else
                if (dl.q1 > dl.q2) {
                    p2 = p1*((dl.q1-dl.q2)/dl.length);
                    if (dl.q1==0) {
                        vValue -= dl.q2*dl.length;
                    }
                    vValue += dl.q2*dl.length+(dl.length*(dl.q1-dl.q2))/2;
                }
            }
        }
        return vValue;
    }
    double pointM(double x) {
        double mValue = 0;
        for (PointLoad &vl : verticalLoads) {
            if (x>=vl.distance) {
                mValue += vl.value*(x-vl.distance);
            }
        }
        for (DistributedLoad &dl : distributedLoads) {
            double p1 = x-dl.distance;
            double p2;
            if (x<dl.distance+dl.length) {
                if (dl.q1 == dl.q2) {
                    mValue += (dl.q1*pow(p1,2))/2;
                } else
                if (dl.q1 < dl.q2) {
                    p2 = p1*((dl.q2-dl.q1)/dl.length);
                    if (dl.q1==0) {
                        mValue -= (dl.q1*pow(p1,2))/2;
                    }
                    mValue += (pow(p1,2)*(3*dl.q1+p2))/6;
                } else
                if (dl.q1 > dl.q2) {
                    p2 = p1*((dl.q1-dl.q2)/dl.length);
                    mValue += (pow(p1,2)*(3*dl.q1-p2))/6;
                }
            }
            if (x>=dl.distance+dl.length) {
                if (dl.q1 == dl.q2) {
                    mValue += dl.q1*dl.length*(p1-dl.length/2);
                } else
                if (dl.q1 < dl.q2) {
                    p2 = p1*((dl.q2-dl.q1)/dl.length);
                    mValue += dl.q1*dl.length*(p1-dl.length/2)+((dl.length*(dl.q2-dl.q1))/2)*(p1-(2/3)*dl.length);
                } else
                if (dl.q1 > dl.q2) {
                    p2 = p1*((dl.q1-dl.q2)/dl.length);
                    if (dl.q1==0) {
                        mValue -= dl.length*dl.q2*(3*p1-2*dl.length)/6;
                    }
                    mValue += dl.q2*dl.length*(p1-dl.length/2)+((dl.length*(dl.q1-dl.q2))/2)*(p1-dl.length/3);
                }
            }
        }
        for (PointMoment &m : pointMoments) {
            if (x>= m.distance) {
                mValue += m.value;
            }
        }
        return -mValue;
    }

    std::pair<std::vector<double>,std::vector<double>> getHpair() {
        std::vector<std::pair<double,double>> hPair;

        for (std::pair<double, double> &segment : hPoints()) {
            double dist = 0;
            hPair.push_back(std::pair<double,double>(segment.first, pointH(segment.first)));
            hPair.push_back(std::pair<double,double>(segment.second, pointH(segment.second)));
            while (dist < segment.second) {
                hPair.push_back(std::pair<double,double>(dist, pointH(dist)));
                dist += dx;
            }
        }
        hPair.insert(hPair.begin(), std::pair<double,double>(0,0));

        std::sort(hPair.begin(),hPair.end());
        
        std::vector<double> xH, yH;
        for (std::pair<double,double> &point : hPair) {
            xH.push_back(point.first);
            yH.push_back(point.second);
        }
        return std::make_pair(xH,yH);
    }
    std::pair<std::vector<double>,std::vector<double>> getTpair() {
        std::vector<std::pair<double,double>> tPair;

        for (std::pair<double, double> &segment : vPoints()) {
            double dist = 0;
            tPair.push_back(std::pair<double,double>(segment.first, pointV(segment.first)));
            tPair.push_back(std::pair<double,double>(segment.second, pointV(segment.second)));
            while (dist < segment.second) {
                tPair.push_back(std::pair<double,double>(dist, pointV(dist)));
                dist += dx;
            }
        }
        tPair.insert(tPair.begin(), std::pair<double,double>(0,0));
        
        std::sort(tPair.begin(),tPair.end());
        
        std::vector<double> xT, yT;
        for (std::pair<double,double> &point : tPair) {
            xT.push_back(point.first);
            yT.push_back(point.second);
        }
        return std::make_pair(xT,yT);
    }
    std::pair<std::vector<double>,std::vector<double>> getMpair() {
        std::vector<std::pair<double,double>> mPair;

        for (std::pair<double, double> &segment : mPoints()) {
            double dist = 0;
            mPair.push_back(std::pair<double,double>(segment.first, pointM(segment.first)));
            mPair.push_back(std::pair<double,double>(segment.second, pointM(segment.second)));
            while (dist < segment.second) {
                mPair.push_back(std::pair<double,double>(dist, pointM(dist)));
                dist += dx;
            }
        }
        mPair.insert(mPair.begin(), std::pair<double,double>(0,0));
        
        std::sort(mPair.begin(),mPair.end());
        
        std::vector<double> xM, yM;
        for (std::pair<double,double> &point : mPair) {
            xM.push_back(point.first);
            yM.push_back(point.second);
        }

        return std::make_pair(xM,yM);
    }

    void Hdiag() {
        Gnuplot hdiag;
        hdiag << "set key noautotitle;\n set offsets 1, 1, 1, 1" << std::endl;
        hdiag << "plot" << hdiag.file1d(std::make_pair(std::vector<double>{0,beam.length},std::vector<double>{0,0}))  << "with lines notitle lw 1.2 lc rgb 'red'," << hdiag.file1d(std::make_pair(getHpair().first,getHpair().second)) << "with lines title 'Sforzo normale' lw 3 lc rgb 'black'," << std::endl;
        hdiag << "pause mouse close;" << std::endl;

        #ifdef _WIN32
        // For Windows, prompt for a keystroke before the Gnuplot object goes out of scope so that
        // the gnuplot window doesn't get closed.
        std::cout << "Press enter to exit." << std::endl;
        std::cin.get();
        #endif
    }
    void Vdiag() {
        Gnuplot vdiag;
        vdiag << "set key noautotitle;\n set offsets 1, 1, 1, 1" << std::endl;
        vdiag << "plot" << vdiag.file1d(std::make_pair(std::vector<double>{0,beam.length},std::vector<double>{0,0}))  << "with lines notitle lw 1.2 lc rgb 'red'," << vdiag.file1d(std::make_pair(getTpair().first,getTpair().second)) << "with lines title 'Taglio' lw 3 lc rgb 'black'," << std::endl;
        vdiag << "pause mouse close;" << std::endl;

        #ifdef _WIN32
        // For Windows, prompt for a keystroke before the Gnuplot object goes out of scope so that
        // the gnuplot window doesn't get closed.
        std::cout << "Press enter to exit." << std::endl;
        std::cin.get();
        #endif
    }
    void Mdiag() {
            Gnuplot mdiag;
        mdiag << "set key noautotitle; \nset offsets 1, 1, 1, 1" << std::endl;
        mdiag << "plot" << mdiag.file1d(std::make_pair(std::vector<double>{0,beam.length},std::vector<double>{0,0}))  << "with lines notitle lw 1.2 lc rgb 'red'," << mdiag.file1d(std::make_pair(getMpair().first,getMpair().second)) << "with lines title 'Momento flettente' lw 3 lc rgb 'black'," << std::endl;
        mdiag << "pause mouse close;" << std::endl;
        
        #ifdef _WIN32
        // For Windows, prompt for a keystroke before the Gnuplot object goes out of scope so that
        // the gnuplot window doesn't get closed.
        std::cout << "Press enter to exit." << std::endl;
        std::cin.get();
        #endif
    }
};

int main() {    
    Beam b;
    std::vector<PointLoad> v{};
    std::vector<PointLoad> h{};
    std::vector<DistributedLoad> d{};
    std::vector<PointMoment> m{};
    Context ctx(b,v,h,d,m);

    return 0;
}