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
    // Terminal application
/*
    std::cout << "----- BASIC CONSOLE PROJECT -----" << std::endl << std::endl;
    std::cout << "Creating environment...  \n";
 
    double latitude_degree;
    std::cout << "Please, enter LATITUDE (DEGREE NORTH): ";
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
    std::cout << "Please, enter EFFICIENCY TYPE [1: Cosine only, 2: Cosine + Attenuation, 3: All factors]: ";
    std::cin >> int_efficiency_type;
*/
    // Loop




    for (int lat=10; lat < 61; lat=lat+10)
    {
       for (int th=20; th < 1001; th=th+10)
       {
           for (int rec_rad=1; rec_rad < 13.1; rec_rad=rec_rad+1)
           {
                double receiver_height=th;
                double latitude_degree=lat;
                double receiver_radius=rec_rad;

                int nrows=600;
                int ncolumns=250;
                double ymax=2000.;
                double ymin=-1000.;
                double delta_t=225.;
                int int_efficiency_type=3;
                std::cout << "Resolution: " << (ymax-ymin)/nrows << std::endl;
                std::string filename="Efficiency-AllFactors_";
                filename.append("Lat-").append(std::to_string((int) lat)).append("_");
                filename.append("TH-").append(std::to_string((int) th)).append("_");
                filename.append("RecRadius-").append(std::to_string((int) rec_rad)).append("_");
                filename.append("Resolution-5x5").append(".dat");

                hypl::Location location(latitude_degree * hypl::mathconstants::degree);
                hypl::Atmosphere atmosphere;
                hypl::Environment environment(location, atmosphere);

                hypl::Boundaries boundaries(-1250.,0.,ymin,ymax);

                std::vector<hypl::Receiver> receivers;
                receivers.push_back(hypl::Receiver(hypl::vec3d(0.0, 0.0, receiver_height), receiver_radius));

                std::ofstream outputFile;
                outputFile.open (filename, std::ios::out | std::ios::app | std::ios::binary);   

                // Efficiency Matrix

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
                std::cout << duration.count()/60000000. << " minutes" << std::endl;

                //std::cout << "Writing binary output file... \n";
                
                start = std::chrono::high_resolution_clock::now();

                outputFile.write( (char *) &ideal_efficiency_map.environment().location().latitude(), sizeof(double));
                outputFile.write( (char *) &ideal_efficiency_map.environment().atmosphere().beta(), sizeof(double));
                outputFile.write( (char *) &ideal_efficiency_map.environment().atmosphere().io(), sizeof(double));
                outputFile.write( (char *) ideal_efficiency_map.environment().atmosphere().TransmittanceModelName(), 4);

                int n_receivers = ideal_efficiency_map.receivers().size();
                outputFile.write( (char *)  &n_receivers,sizeof(int));
                for (int j=0; j<n_receivers; j++)
                {
                    outputFile.write( (char *) &ideal_efficiency_map.receivers().at(j).aiming_point().x, sizeof(double));
                    outputFile.write( (char *) &ideal_efficiency_map.receivers().at(j).aiming_point().y, sizeof(double));
                    outputFile.write( (char *) &ideal_efficiency_map.receivers().at(j).aiming_point().z, sizeof(double));
                    outputFile.write( (char *) &ideal_efficiency_map.receivers().at(j).radius(), sizeof(double));
                }

                outputFile.write( (char *) &ideal_efficiency_map.nrows(), sizeof(int));
                outputFile.write( (char *) &ideal_efficiency_map.ncolumns(), sizeof(int));
                outputFile.write( (char *) &ideal_efficiency_map.boundaries().xmin(), sizeof(double));
                outputFile.write( (char *) &ideal_efficiency_map.boundaries().xmax(), sizeof(double));
                outputFile.write( (char *) &ideal_efficiency_map.boundaries().ymin(), sizeof(double));
                outputFile.write( (char *) &ideal_efficiency_map.boundaries().ymax(), sizeof(double));

                char * ideal_efficiency_name;
                switch ( ideal_efficiency_type )
                {
                    case hypl::Heliostat::IdealEfficiencyType::CosineOnly:
                        ideal_efficiency_name = "Cosine Only\0";
                        break;
                    case hypl::Heliostat::IdealEfficiencyType::CosineAndTransmittance:
                        ideal_efficiency_name = "Cosine and Attenuation\0";
                        break;
                    case hypl::Heliostat::IdealEfficiencyType::AllFactors:
                        ideal_efficiency_name = "All Factors\0";
                        break;
                    default:
                        ideal_efficiency_name = "Not Defined\0";
                        break;
                }
                outputFile.write( (char *) ideal_efficiency_name, strlen(ideal_efficiency_name)+1);

                std::vector<hypl::Heliostat> const& heliostats = ideal_efficiency_map.heliostats();

                for (auto& element : heliostats)
                {
                    double annual_ideal_efficiency = element.m_annual_ideal_efficiency;
                    outputFile.write((char *) &annual_ideal_efficiency, sizeof(double));
                }
                outputFile.close();

                stop = std::chrono::high_resolution_clock::now();

                duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                //std::cout << duration.count() << std::endl;

                std::cout << "DONE" <<std::endl;
            }// end loop on receiver radius
        }// end loop on Tower height
    }// end loop on Latitude
    return 1;
}