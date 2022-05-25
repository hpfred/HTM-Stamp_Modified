##Para liberar a execução de shell script:
##  chmod 777 [nome_arquivo.sh]

##Define caracteristicas da execução
listaBenchs=(
            "bayes"
            "genome"
            "intruder"
            "kmeans"
            "labyrinth"
            "ssca2"
            "vacation"
            "yada"
            )
numExecs=30; #30;
##----------------------------------

# cd htm-stamp/;

if [ ! -d $PWD/Results ]
then
    mkdir Results
fi;

SECONDS=0

for benchs in ${listaBenchs[*]}
do
    cd $benchs;

    make -f Makefile.hle_intel clean;
    make -f Makefile.hle_intel default;

    echo >  ../Results/"$benchs"_1.txt;

    if [[ "$benchs" = "kmeans" || "$benchs" = "vacation" ]]
    then
        echo >  ../Results/"$benchs"_2.txt;
    fi;

    ##Se arquivos de input estiverem como gunzip ele irá descompactar
    if [ -d $PWD/inputs ]
    then
        ##if [ -f *.gz ] tem problema pq se houver mais de um arquivo ele retorna falso
        cd inputs
        if ls *.gz >/dev/null 2>&1
        then
            yes n | gzip -dk *.gz >/dev/null 2>&1
        fi;
        cd ../
    fi;

    cd ../
done;

for i in $(seq 1 $numExecs)
do
    
    for benchs in ${listaBenchs[*]}
    do
        cd $benchs;

        if [ "$benchs" = "bayes" ]
        then
            params="-v32 -r4096 -n10 -p14 -i2 -e8 -s1";
            params2="";
        elif [ "$benchs" = "genome" ]
        then
            params="-g16384 -s64 -n8486974";
            params2="";
        elif [ "$benchs" = "intruder" ]
        then
            params="-a10 -l128 -n142144 -s1";
            params2="";
        elif [ "$benchs" = "kmeans" ]
        then
            params="-m40 -n40 -t0.00001 -i inputs/random-n65536-d32-c16.txt";
            params2="-m15 -n15 -t0.00001 -i inputs/random-n65536-d32-c16.txt";
        elif [ "$benchs" = "labyrinth" ]
        then
            params="-i inputs/random-x512-y512-z7-n512.txt";
            params2="";
        elif [ "$benchs" = "ssca2" ]
        then
            params="-s18 -i1.0 -u1.0 -l3 -p3";
            params2="";
        elif [ "$benchs" = "vacation" ]
        then
            params="-n2 -q90 -u98 -r1048576 -t2194304";
            params2="-n4 -q60 -u90 -r1048576 -t2194304";
        elif [ "$benchs" = "yada" ]
        then
            params="-a15 -i inputs/ttimeu10000.2";
            params2="";
        fi;

        #./"$benchs".hle_intel $params >> ../Results/"$benchs"_1.txt #"$benchs"_1.txt
        eval "./"$benchs".hle_intel $params" >> ../Results/"$benchs"_1.txt #"$benchs"_1.txt
        echo "end_of_execution" >> ../Results/"$benchs"_1.txt
        echo "$i - $benchs" >> ../Results/"$benchs"_1.txt
        times >> ../Results/"$benchs"_1.txt
        echo -e "\n\n" >> ../Results/"$benchs"_1.txt
        echo "$i - $benchs"
        times

        if [[ -n $params2 ]]
        then
            eval "./"$benchs".hle_intel $params2" >> ../Results/"$benchs"_2.txt
            echo "end_of_execution" >> ../Results/"$benchs"_2.txt
            echo "$i - $benchs" >> ../Results/"$benchs"_2.txt
            times >> ../Results/"$benchs"_2.txt
            echo -e "\n\n" >> ../Results/"$benchs"_2.txt
            echo "$i - $benchs"
            times
        fi;
        
        cd ../;
    done;

done;

echo -e "\n\n"
echo "Tempo Final: $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"