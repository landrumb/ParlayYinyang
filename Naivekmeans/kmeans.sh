#!bin/bash

#run all of the kmeans functions just to show that they are running

# else
#     echo "Running on local"
#     ./kmeans -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 


echo $HOSTNAME

if [ $HOSTNAME == "wireless-10-104-78-145.umd.edu" ] || [ $HOSTNAME == "MacBook-Pro-4.local" ] #yes spaces! spaces matter in this language 
then
echo "A kmeans"
make clean
make a_kmeans_integrated
./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c original
./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c euc_dist
./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c doubled_centers
./kmeans_integrated -k 50 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c vd

elif [ $HOSTNAME == "aware.aladdin.cs.cmu.edu" ] 
then

    echo "Running on aware"
    P=/ssd1/data
    G=/home/landrum/outputs
    O=/ssd1/results

    BP=$P/bigann
    # ./kmeans -k 1000 -i $BP/base.1B.u8bin.crop_nb_1000000 -f bin -t uint8 -D Euclidian -m 10

    TP=$P/text2image1B

    make clean kmeans

    ./kmeans -k 10 -i $TP/base.1B.fbin.crop_nb_1000000 -f bin -t float -D Euclidian -m 10
else
echo "Other kmeans"
make clean
make kmeans_integrated
./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c original
./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c euc_dist
./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c doubled_centers
./kmeans_integrated -k 50 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c vd
fi