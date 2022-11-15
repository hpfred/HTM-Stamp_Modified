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
listaExecs=(
            "rtm_intel"
            #"seq"
            "stm"      #stm tá quebrando pq ele tenta usar uma biblioteca <stm.h> que não sei daonde ele tirou que existe
            )
stmTypes=(
          "tinystm"
          #"swisstm"
          #"tl2"
          #"norec"
          )
numExecs=30; #30;
##----------------------------------

#Somente fazer uma STM por vez, precisa modificar mais coisas pra fazer mais de uma STM
for stms in ${stmTypes[*]}
do
    #limpa a compilação de todas stms
    #compila a stm em questão

    #o /commom/Makefile.stm tem "STM := " que precisa ser modificado com a STM escolhida
done

if [ ! -d $PWD/Results ]
then
    mkdir Results
fi;

SECONDS=0
start=`date +%s`
data=$(date +%d%m%g_%s)

for execs in ${listaExecs[*]}
do
    for benchs in ${listaBenchs[*]}
    do
        cd $benchs;
        
        eval make -f Makefile.$execs clean;
        eval make -f Makefile.$execs default;

        echo >  ../Results/"$benchs"_1-"$execs"-"$data".txt;

        if [[ "$benchs" = "kmeans" || "$benchs" = "vacation" ]]
        then
            echo >  ../Results/"$benchs"_2-"$execs"-"$data".txt;
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
done;

for execs in ${listaExecs[*]}
do
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

            #./"$benchs".rtm_intel $params >> ../Results/"$benchs"_1.txt #"$benchs"_1.txt
            eval "./"$benchs"."$execs" $params" >> ../Results/"$benchs"_1-"$execs"-"$data".txt;
            echo "end_of_execution" >> ../Results/"$benchs"_1-"$execs"-"$data".txt;
            echo "$i - $benchs - $execs" >> ../Results/"$benchs"_1-"$execs"-"$data".txt;
            times >> ../Results/"$benchs"_1-"$execs"-"$data".txt;
            echo -e "\n\n" >> ../Results/"$benchs"_1-"$execs"-"$data".txt;
            echo "$i - $benchs - $execs"
            times

            if [[ -n $params2 ]]
            then
                eval "./"$benchs"."$execs" $params2" >> ../Results/"$benchs"_2-"$execs"-"$data".txt
                echo "end_of_execution" >> ../Results/"$benchs"_2-"$execs"-"$data".txt
                echo "$i - $benchs - $execs" >> ../Results/"$benchs"_2-"$execs"-"$data".txt
                times >> ../Results/"$benchs"_2-"$execs"-"$data".txt
                echo -e "\n\n" >> ../Results/"$benchs"_2-"$execs"-"$data".txt
                echo "$i - $benchs - $execs"
                times
            fi;
            
            cd ../;
        done;

    done;
done;
end=`date +%s`
runtime=$((end-start))

echo -e "\n\n"
echo "Tempo Final: $((($SECONDS / 60 /60) % 60))h $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
echo -e "\n"
echo "Tempo Final: $((($runtime / 60 /60) % 60))h $((($runtime / 60) % 60))min $(($runtime % 60))sec"