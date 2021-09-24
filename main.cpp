#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <execution>

#include "location.h"
#include "atmosphere.h"
#include "receiver.h"
#include "vec3d.h"
#include "boundaries.h"
#include "heliostat.h"
#include "idealefficiencymap.h"
#include "auxfunction.h"

int main(int argc, char *argv[])
{
    // application

    std::cout << "----- BASIC CONSOLE PROJECT -----" << std::endl << std::endl;
    std::cout << "Creating environment...  \n";
 
    double latitude_degree;
    std::cout << "Please enter LATITUDE (DEGREE NORTH): ";
    std::cin >> latitude_degree;

    hypl::Location location(latitude_degree * hypl::mathconstants::degree);
    hypl::Atmosphere atmosphere;
    hypl::Environment environment(location, atmosphere);

    hypl::Boundaries boundaries;

    double receiver_height;
    std::cout << "Please, enter AIMING POINT HEIGHT (METERS):";
    std::cin >> receiver_height;

    double receiver_radius;
    std::cout << "Please, enter RECEIVER RADIUS (METERS):";
    std::cin >> receiver_radius;

    std::vector<hypl::Receiver> receivers;
    receivers.push_back(hypl::Receiver(hypl::vec3d(0.0, 0.0, receiver_height), receiver_radius));

    double delta_t;
    std::cout << "Please, enter DELTA_T (SECONDS):";
    std::cin >> delta_t;

    int nrows, ncolumns;

    std::cout << "Please, enter NUMBER OF ROWS: ";
    std::cin >> nrows;

    std::cout << "Please, enter NUMBER OF COLUMNS: ";
    std::cin >> ncolumns;

    std::string filename;
    std::cout << "Please, enter OUTPUT FILE NAME: ";
    std::cin >> filename;

    std::ofstream outputFile;
    outputFile.open (filename, std::ios::out | std::ios::app | std::ios::binary);

    int int_efficiency_type;
    std::cout << "Please, enter EFFICIENCY TYPE [1, 2, 3]: ";
    std::cin >> int_efficiency_type;

    hypl::Heliostat::IdealEfficiencyType ideal_efficiency_type;
    if( int_efficiency_type == 1 ) ideal_efficiency_type = hypl::Heliostat::IdealEfficiencyType::CosineOnly;
    else if( int_efficiency_type == 2 ) ideal_efficiency_type = hypl::Heliostat::IdealEfficiencyType::CosineAndTransmittance;
    else ideal_efficiency_type = hypl::Heliostat::IdealEfficiencyType::AllFactors;

    std::cout << "Computing annual heliostat efficiencies... \n";
    auto start = std::chrono::high_resolution_clock::now();

    hypl::IdealEfficiencyMap ideal_efficiency_map(environment, boundaries, receivers, nrows, ncolumns);    
    ideal_efficiency_map.EvaluateAnnualEfficiencies(ideal_efficiency_type, delta_t);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() << std::endl;

    std::cout << "Writing binary output file... \n";   
    start = std::chrono::high_resolution_clock::now();

    outputFile.write( (char *) &ideal_efficiency_map.nrows(), sizeof(int));
    outputFile.write( (char *) &ideal_efficiency_map.ncolumns(), sizeof(int));
    outputFile.write( (char *) &ideal_efficiency_map.boundaries().xmin(), sizeof(double));
    outputFile.write( (char *) &ideal_efficiency_map.boundaries().xmax(), sizeof(double));
    outputFile.write( (char *) &ideal_efficiency_map.boundaries().ymin(), sizeof(double));
    outputFile.write( (char *) &ideal_efficiency_map.boundaries().ymax(), sizeof(double));

    std::vector<hypl::Heliostat> const& heliostats = ideal_efficiency_map.heliostats();

    for (auto& element : heliostats)
    {
        double annual_ideal_efficiency = element.m_annual_ideal_efficiency;
        outputFile.write((char *) &annual_ideal_efficiency, sizeof(double));
    }
    outputFile.close();

    stop = std::chrono::high_resolution_clock::now();

    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() << std::endl;

    std::cout << "DONE" <<std::endl;

    return 1;
}