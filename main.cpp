#include "easydiag.h"
#include <vector>
#include <gnuplot-iostream.h>

void Context::Hdiag() {
    Gnuplot hdiag;
    hdiag << "set key noautotitle;\n set offsets 1, 1, 1, 1" << std::endl;
    hdiag << "plot" << hdiag.file1d(std::make_pair(std::vector<double>{0,beam.length},std::vector<double>{0,0}))  << "with lines notitle lw 1.2 lc rgb 'red'," << hdiag.file1d(std::make_pair(getHpair().first,getHpair().second)) << "with lines title 'Normal force' lw 3 lc rgb 'black'," << std::endl;
    hdiag << "pause mouse close;" << std::endl;

    #ifdef _WIN32
    // For Windows, prompt for a keystroke before the Gnuplot object goes out of scope so that
    // the gnuplot window doesn't get closed.
    std::cout << "Press enter to exit." << std::endl;
    std::cin.get();
    #endif
}

void Context::Vdiag() {
    Gnuplot vdiag;
    vdiag << "set key noautotitle;\n set offsets 1, 1, 1, 1" << std::endl;
    vdiag << "plot" << vdiag.file1d(std::make_pair(std::vector<double>{0,beam.length},std::vector<double>{0,0}))  << "with lines notitle lw 1.2 lc rgb 'red'," << vdiag.file1d(std::make_pair(getTpair().first,getTpair().second)) << "with lines title 'Shear force' lw 3 lc rgb 'black'," << std::endl;
    vdiag << "pause mouse close;" << std::endl;

    #ifdef _WIN32
    // For Windows, prompt for a keystroke before the Gnuplot object goes out of scope so that
    // the gnuplot window doesn't get closed.
    std::cout << "Press enter to exit." << std::endl;
    std::cin.get();
    #endif
}

void Context::Mdiag() {
    Gnuplot mdiag;
    mdiag << "set key noautotitle; \nset offsets 1, 1, 1, 1" << std::endl;
    mdiag << "plot" << mdiag.file1d(std::make_pair(std::vector<double>{0,beam.length},std::vector<double>{0,0}))  << "with lines notitle lw 1.2 lc rgb 'red'," << mdiag.file1d(std::make_pair(getMpair().first,getMpair().second)) << "with lines title 'Bending moment' lw 3 lc rgb 'black'," << std::endl;
    mdiag << "pause mouse close;" << std::endl;

    #ifdef _WIN32
    // For Windows, prompt for a keystroke before the Gnuplot object goes out of scope so that
    // the gnuplot window doesn't get closed.
    std::cout << "Press enter to exit." << std::endl;
    std::cin.get();
    #endif
}

int main() {
    Beam b(5);
    std::vector<PointLoad> v {PointLoad(1290,0),PointLoad(1290,5)};
    std::vector<PointLoad> h {};
    std::vector<DistributedLoad> q {DistributedLoad(-516,-516,0,5)};
    std::vector<PointMoment> m {};
    Context ctx(b,v,h,q,m);
    ctx.Hdiag();
    ctx.Vdiag();
    ctx.Mdiag();
}
