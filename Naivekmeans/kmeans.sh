#!bin/bash

make clean kmeans

echo $HOSTNAME

if [ $HOSTNAME == "aware.aladdin.cs.cmu.edu" ] 
then
    echo "Running on aware"
    P=/ssd1/data
    G=/home/landrum/outputs
    O=/ssd1/results

    BP=$P/bigann
    # ./kmeans -k 1000 -i $BP/base.1B.u8bin.crop_nb_1000000 -f bin -t uint8 -D Euclidian -m 10

    TP=$P/text2image1B
    ./kmeans -k 10 -i $TP/base.1B.fbin.crop_nb_1000000 -f bin -t float -D Euclidian -m 10
else
    echo "Running on local"
    ./kmeans -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 
fi
    # ./kmeans -k 10 -i ../Data/base.1B.u8bin.crop_nb_1000000 -f bin -t uint8 -m 10

