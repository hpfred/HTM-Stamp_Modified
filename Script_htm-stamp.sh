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
            #"stm"      #stm tá quebrando pq ele tenta usar uma biblioteca <stm.h> que não sei daonde ele tirou que existe
            )
stmTypes=(
          #"tinystm"
          #"swisstm"
          "tl2"
          #"norec"
          )
numExecs=30; #30;
##----------------------------------

if [ ! -d $PWD/Results ]
then
    mkdir Results
fi;

SECONDS=0
start=`date +%s`
data=$(date +%d%m%g_%s)

for execs in ${listaExecs[*]}
do
    execType=$execs
    #Somente fazer uma STM por vez, precisa modificar mais coisas pra fazer mais de uma STM
    if [[ "$execs" = "stm" ]]
    then
        for stms in ${stmTypes[*]}
        do
            #limpa a compilação de todas stms
            #compila a stm em questão
            cd stms/$stms
            make clean
            make
            cd ../../

            #o /commom/Makefile.stm tem "STM := " que precisa ser modificado com a STM escolhida
            execType="$execs+$stms"
        done;
    fi;

    for benchs in ${listaBenchs[*]}
    do
        cd $benchs;
        
        eval make -f Makefile.$execs clean;
        eval make -f Makefile.$execs default;
        
        echo >  ../Results/"$benchs"_1-"$execType"-"$data".txt;

        if [[ "$benchs" = "kmeans" || "$benchs" = "vacation" ]]
        then
            echo >  ../Results/"$benchs"_2-"$execType"-"$data".txt;
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

#Para aqui pra testar
#exit

for execs in ${listaExecs[*]}
do
    for i in $(seq 1 $numExecs)
    do
        
        for benchs in ${listaBenchs[*]}
        do
            cd $benchs;

            if [ "$benchs" = "bayes" ]
            then
                #sim runs params
                #params="-v32 -r1024 -n2 -p20 -s0 -i2 -e2";
                #params2="";
                #non sim runs params
                ##params="-v32 -r4096 -n10 -p14 -i2 -e8 -s1";
                ##params2="";
                #custom params
                #params=''; #-t <number_of_threads>
                params="-v32 -r4096 -n10 -p14 -i2 -e8 -s1 -t24";
                params2="";

            elif [ "$benchs" = "genome" ]
            then
                #sim runs params
                #params="-g256 -s16 -n16384";
                #non sim runs params
                #params="-g16384 -s64 -n16777216";
                #custom params
                ##params="-g16384 -s64 -n8486974";
                ##params2="";
                #params=''; #-t <number_of_threads>
                params="-g16384 -s64 -n16777216 -t24";
                params2="";

            elif [ "$benchs" = "intruder" ]
            then
                #sim runs params
                #params="-a10 -l4 -n2038 -s1";
                #non sim runs params
                #params="-a10 -l128 -n262144 -s1";
                #custom params
                ##params="-a10 -l128 -n142144 -s1";
                ##params2="";
                #params=''; #-t <number_of_threads>
                params="-a10 -l128 -n262144 -s1 -t24";
                params2="";

            elif [ "$benchs" = "kmeans" ]
            then
                #sim runs params
                #params="-m40 -n40 -t0.05 -i inputs/random2048-d16-c16.txt";
                #params2="-m15 -n15 -t0.05 -i inputs/random2048-d16-c16.txt";
                #non sim runs params
                ##params="-m40 -n40 -t0.00001 -i inputs/random-n65536-d32-c16.txt";
                ##params2="-m15 -n15 -t0.00001 -i inputs/random-n65536-d32-c16.txt";
                #custom params
                #params=''; #-p <number_of_threads>
                params="-m40 -n40 -t0.00001 -i inputs/random-n65536-d32-c16.txt -p 24";
                params2="-m15 -n15 -t0.00001 -i inputs/random-n65536-d32-c16.txt -p 24";
            
            elif [ "$benchs" = "labyrinth" ]
            then
                #sim runs params
                #params="-i inputs/random-x32-y32-z3-n96.txt";
                #non sim runs params
                ##params="-i inputs/random-x512-y512-z7-n512.txt";
                ##params2="";
                #custom params
                #params='';
                params="-i inputs/random-x512-y512-z7-n512.txt -t24";
                params2="";

            elif [ "$benchs" = "ssca2" ]
            then
                #sim runs params
                #params="-s13 -i1.0 -u1.0 -l3 -p3";
                #non sim runs params
                #params="-s20 -i1.0 -u1.0 -l3 -p3";
                #custom params
                ##params="-s18 -i1.0 -u1.0 -l3 -p3";
                ##params2="";
                #params=''; #-t <number_of_threads>
                params="-s20 -i1.0 -u1.0 -l3 -p3 -t24";
                params2="";

            elif [ "$benchs" = "vacation" ]
            then
                #sim runs params
                #params="-n2 -q90 -u98 -r16384 -t4096";
                #params2="-n4 -q60 -u90 -r16384 -t4096";
                #non sim runs params
                #params="-n2 -q90 -u98 -r1048576 -t4194304";
                #params2="-n4 -q60 -u90 -r1048576 -t4194304";
                #custom params
                ##params="-n2 -q90 -u98 -r1048576 -t2194304";
                ##params2="-n4 -q60 -u90 -r1048576 -t2194304";
                #params='';
                params="-n2 -q90 -u98 -r1048576 -t4194304 -c24";
                params2="-n4 -q60 -u90 -r1048576 -t4194304 -c24";

            elif [ "$benchs" = "yada" ]
            then
                #sim runs params
                #params="-a20 -i inputs/633.2";
                #non sim runs params
                ##params="-a15 -i inputs/ttimeu10000.2";
                ##params2="";
                #custom params
                #params=''; #-t <number_of_threads>
                params="-a15 -i inputs/ttimeu10000.2 -t24";
                params2="";
            fi;

            #./"$benchs".rtm_intel $params >> ../Results/"$benchs"_1.txt #"$benchs"_1.txt
            eval "./"$benchs"."$execs" $params" >> ../Results/"$benchs"_1-"$execType"-"$data".txt;
            echo "end_of_execution" >> ../Results/"$benchs"_1-"$execType"-"$data".txt;
            echo "$i - $benchs - $execType" >> ../Results/"$benchs"_1-"$execType"-"$data".txt;
            times >> ../Results/"$benchs"_1-"$execType"-"$data".txt;
            echo -e "\n\n" >> ../Results/"$benchs"_1-"$execType"-"$data".txt;
            echo "$i - $benchs - $execType"
            times

            #Acho que o erro com o Seq foi acabar entrando aqui pra todos? N sei
            if [[ -n $params2 ]]
            then
                eval "./"$benchs"."$execs" $params2" >> ../Results/"$benchs"_2-"$execType"-"$data".txt
                echo "end_of_execution" >> ../Results/"$benchs"_2-"$execType"-"$data".txt
                echo "$i - $benchs - $execType" >> ../Results/"$benchs"_2-"$execType"-"$data".txt
                times >> ../Results/"$benchs"_2-"$execType"-"$data".txt
                echo -e "\n\n" >> ../Results/"$benchs"_2-"$execType"-"$data".txt
                echo "$i - $benchs - $execType"
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