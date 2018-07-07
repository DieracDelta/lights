# note please run as root
rm /etc/init.d/lightdaemon
rm /usr/sbin/alienfxd
cp lightdaemon /etc/init.d/lightdaemon
chmod +755 /etc/init.d/lightdaemon
gcc main.c -o alienfxd
cp alienfxd /usr/sbin/alienfxd
