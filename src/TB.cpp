#include "TB.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <iomanip>
#include <utility>

#include <chrono> // for time-related functions
using namespace std::chrono;

double get_wall_time_TB(){
    struct timeval time;
    if(gettimeofday(&time,nullptr)){
        // HANDLE ERROR
        return 0;
    }else{
        return static_cast<double>(time.tv_sec) + static_cast<double>(time.tv_usec*0.000001); //microsegundos
    }
}



TB::TB(shared_ptr<Instance> instance, uint_t seed):instance(std::move(instance)) {
    engine.seed(seed);
//    cout << "TB heuristic initialized\n";
//    instance->print();

    // type_eval_solution = "GAPrelax";
    // this->type_eval_solution = "Heuristic";


}


Solution_std TB::initRandomSolution() {
    // Sample p distinct locations
    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();

    uniform_int_distribution<uint_t> distribution (0, locations.size() - 1);
    while (p_locations.size() < p) {
        auto loc_id = distribution(engine);
        auto loc = locations[loc_id];
        p_locations.insert(loc);
    }
    Solution_std sol(instance, p_locations);
    return sol;
}

Solution_cap TB::initRandomCapSolution() {
    // Sample p distinct locations
    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();

    uniform_int_distribution<uint_t> distribution (0, locations.size() - 1);
    while (p_locations.size() < p) {
        auto loc_id = distribution(engine);
        auto loc = locations[loc_id];
        p_locations.insert(loc);
    }
    Solution_cap sol(instance, p_locations);
    return sol;
}

Solution_cap TB::initHighestCapSolution() {

    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();

    vector<pair<uint_t, uint_t>> sorted_locations;
    for (auto loc:locations) {
        auto cap = instance->getLocCapacity(loc);
        sorted_locations.emplace_back(cap, loc);
    }
    sort(sorted_locations.begin(), sorted_locations.end());
    reverse(sorted_locations.begin(), sorted_locations.end());

    for (uint_t i = 0; i < p; i++) {
        p_locations.insert(sorted_locations[i].second);
    }

    Solution_cap solut(instance, p_locations);

    return solut;
}

Solution_cap TB::initSmartRandomCapSolution(){

    cout << "Initial Solution Smart Random \n";

    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();
    auto filter_locations = instance->getLocations();  

    vector<pair<uint_t, uint_t>> sorted_locations;
    for (auto loc:locations) {
        auto cap = instance->getLocCapacity(loc);
        sorted_locations.emplace_back(cap, loc);
        if (cap <= 1){
            filter_locations.erase(std::remove(filter_locations.begin(), filter_locations.end(), loc), filter_locations.end());
        }
    }
    sort(sorted_locations.begin(), sorted_locations.end());
    reverse(sorted_locations.begin(), sorted_locations.end());

    // p_locations = {1,2,3,4};
    // Solution_cap solut(instance, p_locations, "GAPrelax");

    // uniform_int_distribution<uint_t> distribution (0, filter_locations.size() - 1);
    uniform_int_distribution<uint_t> distribution (0, min(90, (int)filter_locations.size()) - 1);
    while (p_locations.size() < p) {
        auto loc_id = distribution(engine);
        auto loc = filter_locations[loc_id];
        p_locations.insert(loc);
    }

    Solution_cap solut(instance, p_locations, "GAPrelax");
    bool feasible = solut.getFeasibility();
    while(!feasible){
        
            vector<pair<uint_t, uint_t>> sorted_p_locations;
            for (auto p_loc:solut.get_pLocations()) {
                auto cap = instance->getLocCapacity(p_loc);
                sorted_p_locations.emplace_back(cap, p_loc);
            }
            sort(sorted_p_locations.begin(), sorted_p_locations.end());
            if (sorted_p_locations[0].second != sorted_locations[0].second){
                solut.replaceLocation(sorted_p_locations[0].second, sorted_locations[0].second, "GAPrelax");
                if (!sorted_locations.empty()) {sorted_locations.erase(sorted_locations.begin());}
                feasible = solut.getFeasibility();
            }

            if (solut.getFeasibility() == true){
                cout << "feasible solution found\n";
            }else{
                cout << "infeasible solution found\n";
            }
    }


    return solut;


}

Solution_cap TB::initCPLEXCapSolution(double time_limit) {
    
    PMP pmp(instance, "CPMP");
    CLOCK_LIMIT_CPLEX = time_limit;
    pmp.run();
    auto sol = pmp.getSolution_cap();
    return sol;
}


Solution_std TB::run(bool verbose, int MAX_ITE) {

    auto sol_best = initRandomSolution();
    sol_best = localSearch_std(sol_best, verbose, MAX_ITE);

    return sol_best;
}

Solution_cap TB::run_cap(bool verbose, int MAX_ITE) {

    auto sol_best = initHighestCapSolution();
    // auto sol_best = initRandomCapSolution();
    sol_best = localSearch_cap(sol_best, verbose, MAX_ITE);
    return sol_best;
}


void writeReport_TB(const string& filename, dist_t objective, int num_ite, int num_solutions, double time) {
    // Open the file for writing in append mode
    ofstream outputFile(filename, ios::app);
    if (!outputFile.is_open()) {
        cerr << "Error opening the output file." << endl;
        return;
    }

    outputFile << fixed << setprecision(15) << objective << ";"; // obj value
    outputFile << num_ite << ";";
    outputFile << num_solutions << ";";
    outputFile << fixed << setprecision(15) << time << "\n";


    // Close the file
    outputFile.close();
}

Solution_std TB::localSearch_std(Solution_std sol_best, bool verbose, int MAX_ITE) {

    cout << "uncapacitated TB local search started\n";

    //// time limit ////
    auto time_limit_seconds = CLOCK_LIMIT;
    ///

    auto locations = instance->getLocations();
    bool improved = true;
    Solution_std sol_tmp;
    Solution_std sol_cand;
    // int objectiveCpt = 0;

    int ite=1;
    auto start_time_total = high_resolution_clock::now();
    while (improved && ite < MAX_ITE) {

        improved = false;
        sol_cand = sol_best;
        // auto start = tick();
        auto p_locations = sol_best.get_pLocations();

        auto start_time = high_resolution_clock::now();
        for (auto loc:locations) { // First improvement over locations
            // if (!p_locations.contains(loc)) {
            if (std::find(p_locations.begin(), p_locations.end(), loc) == p_locations.end()){
                for (auto p_loc:p_locations) { // Best improvement over p_locations
                    sol_tmp = sol_best;
                    sol_tmp.replaceLocation(p_loc, loc);
                    
                    if (sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE_OBJ) { 
                        sol_cand = sol_tmp;
                        improved = true;
                        // objectiveCpt = 0;
                    }
                    // else{
                    //     objectiveCpt++;

                    //     if(objectiveCpt == TOLERANCE_CPT){
                    //         break;
                    //     }
                    // }
                }
            }
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
            if (improved) {

                sol_best = sol_cand;

                if (elapsed_time >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the algorithm.\n";
                    cout << "Num ite uncapacited TB: " << ite << "\n";
                    cout << "uncapacitated TB loop FINAL elapsed time: " << elapsed_time << " seconds\n";
                    return sol_best;
                }

                break;
            }else{

                if (elapsed_time >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the uncapacitated TB  algorithm.\n";
                    cout << "Num ite total uncapacited TB: " << ite << "\n";
                    cout << "uncapacitated TB loop FINAL elapsed time: " << elapsed_time << " seconds\n";
                    return sol_best;
                }


            };
        }
        if (verbose) {
            sol_best.print();
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
            cout << "Num ite uncapacited TB: " << ite << "\n";
            cout << "uncapacitated TB loop elapsed time: " << elapsed_time << " seconds\n";
            cout << "total time: " << duration_cast<seconds>(current_time - start_time_total).count() << " seconds\n";
            // tock(start);
            cout << endl;
        }
        ite++;
    }

    cout << "Num ite total uncapacited TB: " << ite << "\n";
    return sol_best;
}



Solution_cap TB::localSearch_cap(Solution_cap sol_best, bool verbose, int MAX_ITE) {

    // cout << "TB local search capacitated started\n";
    
    string report_filename = "./reports/report_"+ this->typeMethod + "_" + instance->getTypeService() + "_p_" + to_string(sol_best.get_pLocations().size()) + ".csv";

    //// time limit ////
    auto time_limit_seconds = CLOCK_LIMIT;
    ///
    auto locations = instance->getLocations();
    bool improved = true;
    Solution_cap sol_cand;

    if (generate_reports)
        writeReport_TB(report_filename, sol_best.get_objective(), 0, solutions_map.getNumSolutions(), external_time);


    int ite = 1;
    // auto start_time_total = high_resolution_clock::now();
    auto start_time_total = get_wall_time_TB();
    while (improved && ite < MAX_ITE) {
        
        improved = false;
        sol_cand = sol_best;
        auto p_locations = sol_best.get_pLocations();

        vector<uint_t> p_locations_vec;
        p_locations_vec.reserve(p_locations.size());
        for (auto p_loc:p_locations) p_locations_vec.push_back(p_loc);

        // auto start_time = high_resolution_clock::now();
        auto start_time = get_wall_time_TB();
        for (auto loc:locations) { // First improvement over locations
            // if (!p_locations.contains(loc)) {
            if (std::find(p_locations.begin(), p_locations.end(), loc) == p_locations.end()){    // if loc is not in p_locations
                // #pragma omp parallel for
                for (auto p_loc:p_locations_vec) { // Best improvement over p_locations
                    Solution_cap sol_tmp = sol_best;

                    if (sol_tmp.getTotalCapacity() - instance->getLocCapacity(p_loc) + instance->getLocCapacity(loc) >= instance->getTotalDemand()) {
                        // #pragma omp critical
                        // cout << "capacity constraint violated: no\n";

                        unordered_set<uint_t> p_locations_test = sol_best.get_pLocations();
                        p_locations_test.erase(p_loc);
                        p_locations_test.insert(loc);
                        
                        // auto time_find_sol_start = get_wall_time_TB();
                        int index = solutions_map.pSetExists_index(p_locations_test);
                        // auto time_find_sol_end = high_resolution_clock::now();
                        // cout << "Try to find solution in map elapsed time: " << duration_cast<seconds>(time_find_sol_end - time_find_sol_start).count() << " seconds\n";
                        if (index != -1){
                            // cout << "solution already exists in map\n";
                            if(solutions_map.getObjectiveByIndex(index)  < sol_cand.get_objective()){
                                sol_cand = solutions_map.getSolution(index);
                                improved = true;
                            }

                        }else{
                            // cout << "solution does not exist in map\n";

                            // auto time_solve_pmp_start =  get_wall_time_TB();
                            sol_tmp.replaceLocation(p_loc, loc, "PMP");
                            // auto time_solve_pmp_end =  get_wall_time_TB();
                            // cout << "Capacity relax elapsed time: " << duration_cast<seconds>(time_solve_pmp_end - time_solve_pmp_start).count() << " seconds\n";
                            if (sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE_OBJ) { // LB1
                                
                                Solution_cap sol_tmp2 = sol_best;
                                // evaluate solution with GAP assignment
                                // auto time_solve_gap_start =  get_wall_time_TB();
                                // sol_tmp2.replaceLocation(p_loc, loc, "GAPrelax");
                                sol_tmp2.replaceLocation(p_loc, loc, "heuristic");
                                // auto time_solve_gap_end =  get_wall_time_TB();
                                // cout << "replace location elapsed time: " << duration_cast<seconds>(time_solve_gap_end - time_solve_gap_start).count() << " seconds\n";
                                
                                // solutions_map.addUniqueSolution(sol_tmp2);

                                auto elapsed_time = (get_wall_time_TB() - start_time) + external_time;
                                if (verbose) {
                                    cout << "solution candidate: \n";
                                    sol_tmp2.print();
                                    cout << "Num ite capacited TB: " << ite << "\n";
                                    cout << "capacitated TB loop elapsed time: " << elapsed_time << " seconds\n";
                                    cout << endl;
                                }


                                // sol_tmp.GAP_eval();
                                if (sol_cand.get_objective() - sol_tmp2.get_objective() > TOLERANCE_OBJ) { // LB2
                        
                                    if (verbose) {
                                        cout << "improved solution found\n";
                                        cout << "Solution Improved: \n";
                                        sol_tmp2.print();
                                        cout << "Num ite capacited TB: " << ite << "\n";
                                        cout << "capacitated TB loop elapsed time: " << elapsed_time << " seconds\n";
                                        cout << endl;
                                    }
                                    sol_cand = sol_tmp2;
                                    improved = true;

                                    auto elapsed_time_total = (get_wall_time_TB() - start_time_total) + external_time;
                                    if (generate_reports)
                                        writeReport_TB(report_filename, sol_cand.get_objective(), ite, solutions_map.getNumSolutions(),elapsed_time_total);
                                    


                                }
                            }
                        }

                    }else{
                        cout << "capacity constraint violated: yes\n";
                    }
                }
            }

            auto elapsed_time_total = (get_wall_time_TB() - start_time_total) + external_time;
                
            if (improved) {


                // sol_best = sol_cand;

                auto p_loc = sol_cand.get_pLocations();
                sol_best = Solution_cap(instance, p_loc);
                solutions_map.addUniqueSolution(sol_best);

                if (generate_reports)
                    writeReport_TB(report_filename, sol_cand.get_objective(), ite, solutions_map.getNumSolutions(),elapsed_time_total);
                
                if (elapsed_time_total >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the TB algorithm.\n";
                    cout << "Num ite capacited TB: " << ite << "\n";
                    cout << "capacitated TB loop FINAL elapsed time: " << elapsed_time_total << " seconds\n";
                    return sol_best;
                }

                cout << "Best Solution inside tb: \n";
                sol_best.print();
                cout << "Num ite capacited TB: " << ite << "\n";
                cout << "capacitated TB loop elapsed time: " <<  get_wall_time_TB() - start_time << " seconds\n";
                cout << "total time: " << elapsed_time_total << " seconds\n";
                // tock(start);
                cout << endl;


                ite++;
                break;
            }else{
                if (elapsed_time_total >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the TB algorithm.\n";
                    cout << "Num ite capacited TB: " << ite << "\n";
                    cout << "capacitated TB loop FINAL elapsed time: " << elapsed_time_total << " seconds\n";
                    cout << "Num ite TB: " << ite << "\n";
                    return sol_best;
                }
            };
        }

        auto elapsed_time_total = (get_wall_time_TB() - start_time_total) + external_time;

        if (verbose) {
            cout << "Solution Best: \n";
            sol_best.print();
            cout << "Num ite capacited TB: " << ite << "\n";
            cout << "total time: " << elapsed_time_total << " seconds\n";
            // tock(start);
            cout << endl;
        }

        
        ite++;
    }

    cout << "Num total ite capacited TB: " << ite << "\n\n";
    return sol_best;
}

void TB::setSolutionMap(Solution_MAP sol_map) {
    this->solutions_map = sol_map;
}

void TB::setGenerateReports(bool generate_reports) {
    this->generate_reports = generate_reports;
}

void TB::setMethod(string Method) {
    this->typeMethod = Method;
}

void TB::setExternalTime(double time) {
    this->external_time = time;
}