#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
extern "C" {
    #include <linux/i2c-dev.h>
    #include <i2c/smbus.h>
}
#include <math.h>
#include <fstream>

using namespace std;

// Configuration du gyroscope
// Sensibilité du gyroscope : 131 LSB/(degrés/s) pour une plage de ±250 degrés/s
// Voir le tableau de sensibilités dans la datasheet du MPU6050 pour les autres plages
#define GYRO_SENS 131.0
// GYRO_CONFIG définit la plage du gyroscope :
// 0 = ±250 degrés/s
// 1 = ±500 degrés/s
// 2 = ±1000 degrés/s
// 3 = ±2000 degrés/s  
#define GYRO_CONFIG 0b00000000

// Configuration de l'accéléromètre
// Sensibilité de l'accéléromètre : 16384 LSB/g pour une plage de ±2g
// Voir le tableau de sensibilités dans la datasheet du MPU6050 pour les autres plages  
#define ACCEL_SENS 16384.0
// ACCEL_CONFIG définit la plage de l'accéléromètre :   
// 0 = ±2g
// 1 = ±4g
// 2 = ±8g
// 3 = ±16g
#define ACCEL_CONFIG 0b00000000

int i2cFileDescriptor;

void getAccelRaw(float *x, float *y, float *z) {
    int16_t X = i2c_smbus_read_byte_data(i2cFileDescriptor, 0x3b) << 8 | i2c_smbus_read_byte_data(i2cFileDescriptor, 0x3c); // Lecture des registres x
    int16_t Y = i2c_smbus_read_byte_data(i2cFileDescriptor, 0x3d) << 8 | i2c_smbus_read_byte_data(i2cFileDescriptor, 0x3e); // Lecture des registres y
    int16_t Z = i2c_smbus_read_byte_data(i2cFileDescriptor, 0x3f) << 8 | i2c_smbus_read_byte_data(i2cFileDescriptor, 0x40); // Lecture des registres z
    *x = (float)X;
    *y = (float)Y;
    *z = (float)Z;
}

void getAccel(float *x, float *y, float *z) {
    getAccelRaw(x, y, z); //Initialisation des variables avec les valeurs brutes
    *x = round(*x * 1000.0 / ACCEL_SENS) / 1000.0; // On divise par la sensibilité de l'accéléromètre (round et 1000 pour avoir 3 décimales...)
    *y = round(*y * 1000.0 / ACCEL_SENS) / 1000.0;
    *z = round(*z * 1000.0 / ACCEL_SENS) / 1000.0;
}

void getGyroRaw(float *roll, float *pitch, float *yaw) {
    int16_t X = i2c_smbus_read_byte_data(i2cFileDescriptor, 0x43) << 8 | i2c_smbus_read_byte_data(i2cFileDescriptor, 0x44); // Lecture des registres x
    int16_t Y = i2c_smbus_read_byte_data(i2cFileDescriptor, 0x45) << 8 | i2c_smbus_read_byte_data(i2cFileDescriptor, 0x46); // Lecture des registres y
    int16_t Z = i2c_smbus_read_byte_data(i2cFileDescriptor, 0x47) << 8 | i2c_smbus_read_byte_data(i2cFileDescriptor, 0x48); // Lecture des registres z
    *roll = (float)X; // Roll sur l'axe X   
    *pitch = (float)Y; // Pitch sur l'axe Y
    *yaw = (float)Z; // Yaw sur l'axe Z
}

void getGyro(float *roll, float *pitch, float *yaw) {
    getGyroRaw(roll, pitch, yaw); //Initialisation des variables avec les valeurs brutes    
    *roll = round(*roll * 1000.0 / GYRO_SENS) / 1000.0; // On divise par la sensibilité du gyroscope (round et 1000 pour avoir 3 décimales...) 
    *pitch = round(*pitch * 1000.0 / GYRO_SENS) / 1000.0;
    *yaw = round(*yaw * 1000.0 / GYRO_SENS) / 1000.0; 
}

float ax, ay, az; 
float pre_ax = 0;
float pre_ay = 0;  
float pre_az = 0;

int main(int argc, const char** argv) {    
    cout << "Lancement du programme de lecture du MPU6050" << endl;
    
    // Ouverture du fichier i2c
    i2cFileDescriptor = open("/dev/i2c-1", O_RDWR);
    if (i2cFileDescriptor < 0) {
        cout << "Erreur: Impossible d'ouvrir /dev/i2c-1. Vérifiez que l'interface I2C est activée." << endl;
        return 1;
    }   

    // Configuration de l'adresse du MPU6050  
    int status = ioctl(i2cFileDescriptor, I2C_SLAVE, 0x68);
    if (status < 0) {
        cout << "Erreur: Impossible de communiquer en I2C à l'adresse 0x68. Vérifiez que l'adresse est correcte." << endl;
        return 1;
    }

    // Configuration des registres du MPU6050
    i2c_smbus_write_byte_data(i2cFileDescriptor, 0x1b, GYRO_CONFIG);
    i2c_smbus_write_byte_data(i2cFileDescriptor, 0x1c, ACCEL_CONFIG);
    
    cout << "Début de la lecture des données du MPU6050..." << endl;
    
    ofstream outfile;
    outfile.open("trace_axe_z.txt"); // Ouverture du fichier en mode écriture
    
    while(1) {
        // Lecture des valeurs d'accélération
        getAccel(&ax, &ay, &az);
        
        // Calcul de l'écart en % avec la valeur précédente sur l'axe Z  
        float ecart = 100 - (az/pre_az)*100;
        if(ecart == -0) ecart = 0;
        
        // Affichage des valeurs 
        printf("az => %.3f", az);
        
        if(ecart < 0)
            printf(" | Ecart avec valeur précédente => %.2f %%\n", ecart);
        else if (ecart == 0) 
            printf(" | Ecart avec valeur précédente =>  0.00 %%\n");
        else
            printf(" | Ecart avec valeur précédente => +%.2f %%\n", ecart);
        
        // Sauvegarde de la valeur de l'axe Z dans un fichier
        outfile << az << "\n"; 
        
        // Mise à jour des valeurs précédentes
        pre_ax = ax;
        pre_ay = ay;
        pre_az = az;
        
        // Attente de 100ms avant la prochaine lecture
        usleep(100000);
    }
    
    outfile.close();
    
    return 0;
}
