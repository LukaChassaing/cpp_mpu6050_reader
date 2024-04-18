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

## Configuration du gyroscope et de l'accéléromètre

Le programme utilise les constantes suivantes pour configurer le gyroscope et l'accéléromètre du MPU6050 :

- `GYRO_SENS` : sensibilité du gyroscope en LSB/(degrés/s). Dépend de la plage sélectionnée via `GYRO_CONFIG`.
- `GYRO_CONFIG` : plage du gyroscope (0 = ±250 degrés/s, 1 = ±500 degrés/s, 2 = ±1000 degrés/s, 3 = ±2000 degrés/s).
- `ACCEL_SENS` : sensibilité de l'accéléromètre en LSB/g. Dépend de la plage sélectionnée via `ACCEL_CONFIG`.
- `ACCEL_CONFIG` : plage de l'accéléromètre (0 = ±2g, 1 = ±4g, 2 = ±8g, 3 = ±16g).

Pour changer la plage du gyroscope ou de l'accéléromètre :

1. Modifiez les constantes `GYRO_CONFIG` et `ACCEL_CONFIG` dans `main.cpp` selon les valeurs ci-dessus.
2. Ajustez les valeurs de `GYRO_SENS` et `ACCEL_SENS` selon les tableaux de sensibilités de la datasheet du MPU6050.
3. Recompilez le programme avec `make`.

Ces constantes sont utilisées lors de la configuration des registres du MPU6050 et pour convertir les valeurs brutes des capteurs en unités physiques (degrés/s pour le gyroscope, g pour l'accéléromètre).

## Licence

Ce programme est distribué sous licence MIT. Voir le fichier `LICENSE` pour plus d'informations.
