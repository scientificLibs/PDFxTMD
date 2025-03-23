#include <vector>
#include <memory>
#include <chrono>
#include <fstream>
#include <PDFxTMDLib/GenericPDF.h>

int main()
{
    {
        PDFxTMD::TMDPDF pbTMD("PB-NLO-HERAI+II-2023-set2-qs=1.04", 0);
        std::vector<double> x_vals = {1e-4, 1e-3, 1e-2, 1e-1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
        std::vector<double> kt_vals = {1.0, sqrt(2.0), sqrt(5.0), sqrt(10.0), sqrt(20.0), sqrt(50.0), sqrt(100.0), sqrt(200.0), sqrt(500.0), sqrt(1000.0), sqrt(2000.0), sqrt(5000.0)};
        std::vector<double> mu_vals = {sqrt(4.0), sqrt(5.0), sqrt(10.0), sqrt(20.0), sqrt(50.0), sqrt(100.0), sqrt(200.0), sqrt(500.0), sqrt(1000.0), sqrt(2000.0), sqrt(5000.0)};
        const int iterations = 1e5;

        // Open files for storing results
        std::ofstream performancePDFxTMD_log("perf-res-PDFxTMD_TMDPDF.csv");
        std::ofstream validationPDFxTMD_log("num-val-PDFxTMD_TMDPDF.csv");
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
                        pbTMD.tmd(x_test, kt_test * kt_test, mu_test * mu_test, tmdLibResult);
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