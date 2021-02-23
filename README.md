# mpu6050-cpp-reader

Prévu pour interfacer un raspberry 3b+ avec un mpu6050 en i2c.

1. Activer l'interface I2C avec ```sudo raspi-config```
2. Lancer la commande ```i2cdetect -y 1``` et récupérer l'adresse i2c du mpu
3. Modifier l'adresse en conséquence dans main.cpp
4. Rendre le script compil.sh executable
5. Executer le script compil.sh
6. Executer le programme ! ```./main```
