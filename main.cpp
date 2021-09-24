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
    std::cout << "Please enter the latitude of the location in degree North: ";
    std::cin >> latitude_degree;
    hypl::Location location(latitude_degree * hypl::mathconstants::degree);

    hypl::Atmosphere atmosphere;

    hypl::Environment environment(location, atmosphere);

    std::cout << "\nDefining the boundaries...  \n";
    hypl::Boundaries boundaries;

    std::cout << "\nDefining the receiver...  \n";

    double receiver_height;
    std::cout << "Please, enter the height of the center of the receiver:";
    std::cin >> receiver_height;
    std::vector<hypl::Receiver> receivers;
    double receiver_radius = 5.0;
    receivers.push_back(hypl::Receiver(hypl::vec3d(0.0, 0.0, receiver_height), receiver_radius));

    std::cout << "Heliostat field environment created \n\n";
    std::cout << "-- Location parameters:" <<std::endl;
    std::cout << "---- Latitude (degree North): " << environment.location().LatitudeDegree() <<std::endl <<std::endl;

    std::cout << "-- Atmosphere parameters:" << std::endl;
    std::cout << "---- Io: " << environment.atmosphere().io() << std::endl;
    std::cout << "---- beta: " << environment.atmosphere().beta() << std::endl << std::endl;

    std::cout << "-- Receivers:" <<std::endl;
    std::cout << "---- Number of receivers: " << receivers.size() << std::endl;

    int i = 0;
    for (auto& element : receivers)
    {
        std::cout << "------ Aiming point of receiver (" << i << "): ";
        std::cout << element.aiming_point() << std::endl;
        i++;
    }
    std::cout << std::endl;

    std::cout << std::endl;
    double delta_t;
    std::cout << "Please, enter delta_t in seconds:";
    std::cin >> delta_t;

// Generating the output
    std::string filename;
    std::cout << "Please, input the name of the output file: ";
    std::cin >> filename;

    std::ofstream outputFile;
    outputFile.open (filename, std::ios::out | std::ios::app | std::ios::binary);


    int nrows, ncolumns;

    std::cout << "Please, input the number of rows: ";
    std::cin >> nrows;

    std::cout << "Please, input the number of columns: ";
    std::cin >> ncolumns;

    std::cout << "Dimensions: " << nrows << ", " << ncolumns << "\n";


    std::cout << "Computing annual heliostat efficiencies... \n";
    auto start = std::chrono::high_resolution_clock::now();

    hypl::IdealEfficiencyMap ideal_efficiency_map(environment, boundaries, receivers, nrows, ncolumns);    
    ideal_efficiency_map.EvaluateAnnualEfficiencies(hypl::Heliostat::IdealEfficiencyType::CosineAndTransmittance, delta_t);

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

    std::cout << "Entering writing loop... \n";   

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