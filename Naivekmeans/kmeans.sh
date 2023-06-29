#!bin/zsh

#run all of the kmeans functions just to show that they are running

MYHOST=$(hostname -s) #no spaces!
echo $MYHOST

if [[ $MYHOST == "wireless-10-104-78-145" ]] #yes spaces! spaces matter in this language 
then
echo "A kmeans"
make clean
make a_kmeans_integrated
./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c original
./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c euc_dist
./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c doubled_centers
./kmeans_integrated -k 50 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c vd

else

echo "Other kmeans"
make clean
make kmeans_integrated
./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c original
./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c euc_dist
./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c doubled_centers
./kmeans_integrated -k 50 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c vd

fi