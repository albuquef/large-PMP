using Dates, Plots, JuMP
using Cbc, CPLEX
using DataFrames, CSV
# using ArgParse

include("instance.jl")
include("pmp.jl")
include("run.jl")


if isempty(ARGS) 
    println("No args")
    exit()
end


println(ARGS)
# exit()

#solver 
global solver = "CPLEX"

# args
global problem = ARGS[1]
global num_customers = parse(Int,ARGS[2])
global num_locations = parse(Int,ARGS[3])
global p = parse(Int,ARGS[4])
global typeSub = ARGS[5]
global typeCap = ARGS[6]

# utils vars
# data path
# global PATH_TO_DATA="./data/Random_area_784_ub_"*string(num_urban_centers)*"/random_"*string(num_customers)*"_"*string(max_locations)*"/"
global PATH_TO_DATA = "./data/toulon/"

# load customer weights
# global cust_weights = readCustomers(PATH_TO_DATA,num_customers,"cust_weights.txt")
global cust_weights = readCustomers(PATH_TO_DATA,num_customers,"cust_weights.txt")


# typeServ = "poste" # typeServ = "mat" # typeServ = "lycee" # typeServ = "urgenc"
# global loc_capacities = readLocations(PATH_TO_DATA,num_locations,"loc_capacities_cap_"*typeServ*".txt")
global loc_capacities = readLocations(PATH_TO_DATA,num_locations,"loc_capacities.txt")


# typeDist = metres or minutes
# global dist_matrix = readDistances(PATH_TO_DATA,"dist_matrix_"*typeDist*".txt")
global dist_matrix = readDistances(PATH_TO_DATA,"dist_matrix.txt")


# problem with and without coverages
if problem == "CPMP"
    global vet_subareas = []
    global typeDiv = "null"
else
    global vet_subareas = readSubareas(PATH_TO_DATA,"loc_coverages_"*typeDiv*".txt")
end


solve()




