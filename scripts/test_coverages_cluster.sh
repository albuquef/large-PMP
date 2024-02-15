#!/bin/bash
#SBATCH --job-name=pmpCover
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --partition=cpuonly
#SBATCH --mem=64G
#SBATCH --time=100:00:00 
#SBATCH --array=0-30%5

# Activate the conda env if needed
#source /etc/profile.d/conda.sh # Required before using conda
#conda activate myenv

CMD=./build/large_PMP
DIR_DATA=./data/filterData_PACA_may23/
DIST_TYPE=minutes

D_MATRIX=${DIR_DATA}dist_matrix_${DIST_TYPE}.txt
WEIGHTS=${DIR_DATA}cust_weights.txt
TIME_CPLEX=3600
TIME_CLOCK=3600

###### mat
SERVICE=mat # lycee, mat, poste, urgenc
SUBAREA=arrond # arrond  canton epci commune epci
p_values=(26 30 34 38 42 46 50 54)

CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
COVERAGES=${DIR_DATA}loc_coverages_${SUBAREA}.txt
OUTPUT=./solutions/test_paca_${SERVICE}_${SUBAREA}


METHOD="EXACT_CPMP"
for p in "${p_values[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
        -cover $COVERAGES -subarea $SUBAREA\
        -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
        -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
        -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_p_${p}.txt")
done

###### mat
SERVICE=mat # lycee, mat, poste, urgenc
SUBAREA=epci # arrond  canton epci commune epci
p_values=(51 54 58 62)

CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
COVERAGES=${DIR_DATA}loc_coverages_${SUBAREA}.txt
OUTPUT=./solutions/test_paca_${SERVICE}_${SUBAREA}


METHOD="EXACT_CPMP"
for p in "${p_values[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
        -cover $COVERAGES -subarea $SUBAREA\
        -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
        -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
        -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_p_${p}.txt")
done


###### urgenc
SERVICE=urgenc # lycee, mat, poste, urgenc
SUBAREA=arrond # arrond  canton epci commune epci
p_values=(42 48 54 60 66 72 78)

CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
COVERAGES=${DIR_DATA}loc_coverages_${SUBAREA}.txt
OUTPUT=./solutions/test_paca_${SERVICE}_${SUBAREA}


METHOD="EXACT_CPMP"
for p in "${p_values[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
        -cover $COVERAGES -subarea $SUBAREA\
        -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
        -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
        -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_p_${p}.txt")
done

###### urgenc
SERVICE=urgenc # lycee, mat, poste, urgenc
SUBAREA=arrond # arrond  canton epci commune epci
p_values=(42 48 54 60 66 72 78)

CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
COVERAGES=${DIR_DATA}loc_coverages_${SUBAREA}.txt
OUTPUT=./solutions/test_paca_${SERVICE}_${SUBAREA}


METHOD="EXACT_CPMP"
for p in "${p_values[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
        -cover $COVERAGES -subarea $SUBAREA\
        -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
        -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
        -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_p_${p}.txt")
done


###### urgenc
SERVICE=urgenc # lycee, mat, poste, urgenc
SUBAREA=epci # arrond  canton epci commune epci
p_values=(54 60 66 72 78)

CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
COVERAGES=${DIR_DATA}loc_coverages_${SUBAREA}.txt
OUTPUT=./solutions/test_paca_${SERVICE}_${SUBAREA}


METHOD="EXACT_CPMP"
for p in "${p_values[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
        -cover $COVERAGES -subarea $SUBAREA\
        -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
        -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
        -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_p_${p}.txt")
done


# METHOD="EXACT_CPMP"
# METHOD="RSSV"
# METHOD_RSSV_FINAL="VNS_CPMP"
# metsp="TB_PMP"
# for p in "${p_values[@]}"
# do
#   arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
#         -cover $COVERAGES -subarea $SUBAREA\
#         -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
#         -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
#         -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt")
# done


if [ -z "$arr" ]; then
    echo "No instances"
fi

# for element in "${arr[@]}"; do
#     echo "$element && wait"
# done

echo "Number of instances: ${#arr[@]}"
#chmod +x ${arr[$SLURM_ARRAY_TASK_ID]}
# srun ${arr[$SLURM_ARRAY_TASK_ID]}