#include <tmdlib/TMDlib.h>
#include <vector>
#include <memory>
#include <chrono>
#include <fstream>
#include <PDFxTMDLib/Factory.h>
#include <PDFxTMDLib/ITMD.h>

int main()
{
    {
        using namespace TMDlib;
        std::shared_ptr<TMD> test = std::make_shared<TMD>();
        int irep = 0;
        test->TMDinit("PB-NLO-HERAI+II-2023-set2-qs=1.04", irep);
        test->TMDpdf(0.001, 0, 2, 100); //ignore readtime
        const double tmdlibCoeff = 13.0/14.0;
        PDFxTMD::GenericTMDFactory tmdFactory;
        auto pbTMDPDFxTMD = tmdFactory.mkTMD("PB-NLO-HERAI+II-2023-set2-qs=1.04", irep);
        std::vector<double> x_vals = {1e-4, 1e-3, 1e-2, 1e-1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
        std::vector<double> kt_vals = {1.0, sqrt(2.0), sqrt(5.0), sqrt(10.0), sqrt(20.0), sqrt(50.0), sqrt(100.0), sqrt(200.0), sqrt(500.0), sqrt(1000.0), sqrt(2000.0), sqrt(5000.0)};
        std::vector<double> mu_vals = {sqrt(4.0), sqrt(5.0), sqrt(10.0), sqrt(20.0), sqrt(50.0), sqrt(100.0), sqrt(200.0), sqrt(500.0), sqrt(1000.0), sqrt(2000.0), sqrt(5000.0)};
        const int iterations = 1e5;

        // Open files for storing results
        std::ofstream performance_log("perf-res-TMDLIB_KTAPI2.csv");
        std::ofstream validation_log("num-val-TMDLIB_KTAPI2.csv");
        performance_log << "x,kt2,mu2,time_per_call_ns" << std::endl;
        validation_log << "x,kt2,mu2,bbar,cbar,sbar,ubar,dbar,g, d, u, s, c, b" << std::endl;

        int t1 = time(0);
        int total_points = x_vals.size() * kt_vals.size() * mu_vals.size();
        int current_point = 0;

        for (const auto &x_test : x_vals)
        {
            for (const auto &kt_test : kt_vals)
            {
                for (const auto &mu_test : mu_vals)
                {
                    double up, down, strange, charm, bottom, ubar, dbar, sbar, cbar, bbar, top, tbar, gluon, photon;
                    current_point++;
                    // Store results for validation
                    auto start_time = std::chrono::high_resolution_clock::now();
                    for (int i = 0; i < iterations; i++)
                    {
                        // Performance test with all flavors
                        test->TMDpdf(x_test, 0, kt_test, mu_test, up, ubar, down, dbar, strange, sbar, charm, cbar, bottom, bbar, gluon, photon);
                    }

                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
                    double time_per_call = duration.count() / (iterations);

                    // Log performance results
                    performance_log << x_test << "," << kt_test * kt_test << "," << mu_test * mu_test << ","
                                    << time_per_call << std::endl;

                    // Log numerical validation results
                    validation_log << x_test << "," << kt_test * kt_test << "," << mu_test * mu_test << ","
                                   << bbar << "," << cbar << "," << sbar << ","
                                   << ubar << "," << dbar << "," << down << ","
                                   << up << "," << strange << "," << charm << ","
                                   << bottom << std::endl;

                    // Progress update
                    std::cout << "Completed point " << current_point << "/" << total_points
                                << " (" << (current_point * 100.0 / total_points) * tmdlibCoeff << "%)"
                                << " - Time per call: " << time_per_call * tmdlibCoeff << " ns" << std::endl;
                }
            }
        }
        int t2 = time(0);

        // Close log files
        performance_log.close();
        validation_log.close();

        std::cout << "Total execution time: " << t2 - t1 << " seconds" << std::endl;
        std::cout << "Results written to performance_results.txt and numerical_validation.txt" << std::endl;
        ///////////////////////////////////////////////////////////////////

        // Open files for storing results
        std::ofstream performancePDFxTMD_log("perf-res-PDFxTMD_mkTMD.csv");
        std::ofstream validationPDFxTMD_log("num-val-PDFxTMD_mkTMD.csv");
        performancePDFxTMD_log << "x,kt2,mu2,time_per_call_ns" << std::endl;
        validationPDFxTMD_log << "x,kt2,mu2,bbar,cbar,sbar,ubar,dbar,g, d, u, s, c, b" << std::endl;

        int t1PDFxTMD = time(0);
        int total_pointsPDFxTMD = x_vals.size() * kt_vals.size() * mu_vals.size();
        int current_pointPDFxTMD = 0;

        for (const auto &x_test : x_vals)
        {
            for (const auto &kt_test : kt_vals)
            {
                for (const auto &mu_test : mu_vals)
                {
                    current_pointPDFxTMD++;
                    // Store results for validation
                    auto start_time = std::chrono::high_resolution_clock::now();
                    std::array<double, 13> tmdLibResult;
                    for (int i = 0; i < iterations; i++)
                    {
                        // Performance test with all flavors
                        pbTMDPDFxTMD.tmd(x_test, kt_test * kt_test, mu_test * mu_test, tmdLibResult);
                    }

                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
                    double time_per_call = duration.count() / (iterations); 

                    // Log performance results
                    performancePDFxTMD_log << x_test << "," << kt_test * kt_test << "," << mu_test * mu_test << ","
                                           << time_per_call << std::endl;

                    // Log numerical validation results
                    validationPDFxTMD_log << x_test << "," << kt_test * kt_test << "," << mu_test * mu_test << ","
                                          <<  tmdLibResult[1] << "," << tmdLibResult[2] << ","
                                          << tmdLibResult[3] << "," << tmdLibResult[4] << "," << tmdLibResult[5] << ","
                                          << tmdLibResult[6] << "," << tmdLibResult[7] << "," << tmdLibResult[8] << ","
                                          << tmdLibResult[9] << "," << tmdLibResult[10] << "," << tmdLibResult[11] << std::endl;

                    // Progress update
                    std::cout << "Completed point " << current_pointPDFxTMD << "/" << total_pointsPDFxTMD
                              << " (" << (current_pointPDFxTMD * 100.0 / total_pointsPDFxTMD) << "%)"
                              << " - Time per call: " << time_per_call << " ns" << std::endl;
                }
            }
        }
        int t2PDFxTMD = time(0);

        // Close log files
        performancePDFxTMD_log.close();
        validationPDFxTMD_log.close();

        std::cout << "Total execution time: " << t2PDFxTMD - t1PDFxTMD << " seconds" << std::endl;
        std::cout << "Results written to performance_results.txt and numerical_validation.txt" << std::endl;
    }

    return 0;
}