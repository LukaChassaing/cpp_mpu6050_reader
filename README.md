# Lecteur MPU6050 en C++ pour Raspberry Pi

Ce programme permet de lire les données d'accélération et de vitesse angulaire d'un capteur MPU6050 connecté en I2C à un Raspberry Pi.

## Prérequis

- Un Raspberry Pi (testé avec un modèle 3B+) 
- Un capteur MPU6050
- L'interface I2C activée sur le Raspberry Pi (via `sudo raspi-config`)

## Installation 

1. Clonez ce dépôt sur votre Raspberry Pi
2. Compilez le programme avec la commande `make`

## Utilisation

1. Connectez le capteur MPU6050 au Raspberry Pi en suivant ce schéma de branchement:
   - VCC -> PIN 1 (3.3V)
   - GND -> PIN 6 (GND)
   - SDA -> PIN 3 (SDA)
   - SCL -> PIN 5 (SCL) 
   
2. Lancez la commande `i2cdetect -y 1` pour vérifier que le capteur est bien détecté et récupérer son adresse I2C.

3. Si besoin, modifiez l'adresse I2C dans le fichier `main.cpp` (par défaut `0x68`).

4. Exécutez le programme avec la commande `./mpu6050_reader`.

5. Les données d'accélération sur les 3 axes (ax, ay, az) ainsi que l'écart en % entre 2 mesures successives sur l'axe Z seront affichées en continu dans le terminal. 

6. Les valeurs de l'axe Z sont aussi enregistrées dans le fichier `trace_axe_z.txt`.

7. Appuyez sur `Ctrl+C` pour arrêter le programme.

## Licence

Ce programme est distribué sous licence MIT. Voir le fichier `LICENSE` pour plus d'informations.
