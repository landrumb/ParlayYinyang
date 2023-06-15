#!bin/bash
cd ~/ParlayYinyang
make clean all

P=/ssd1/data
G=/home/landrum/outputs
O=/ssd1/results

BP=$P/bigann
./kmeans -k 1000 -i $BP/base.1B.u8bin.crop_nb_1000000 -f bin -t uint8 -D Euclidian -m 10