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


#define GYRO_RANGE 0 // Mode lecture du gyroscope - Par défaut à 0
//	0	+/- 250 degrés/seconde
//	1	+/- 500 degrés/seconde
//	2	+/- 1000 degrés/seconde
//	3	+/- 2000 degrés/seconde


#define ACCEL_RANGE 0 //Mode de lecture de l'accéléromètre - Par défaut à 0
//	0	+/- 2g
//	1	+/- 4g
//	2	+/- 8g
//	3	+/- 16g

#if ACCEL_RANGE == 1
	#define ACCEL_SENS 8192.0
	#define ACCEL_CONFIG 0b00001000
#elif ACCEL_RANGE == 2
	#define ACCEL_SENS 4096.0
	#define ACCEL_CONFIG 0b00010000
#elif ACCEL_RANGE == 3
	#define ACCEL_SENS 2048.0
	#define ACCEL_CONFIG 0b00011000
#else // Par défaut à zéro
	#define ACCEL_SENS 16384.0
	#define ACCEL_CONFIG 0b00000000
#endif
#undef ACCEL_RANGE

#if GYRO_RANGE == 1
	#define GYRO_SENS 65.5
	#define GYRO_CONFIG 0b00001000
#elif GYRO_RANGE == 2
	#define GYRO_SENS 32.8
	#define GYRO_CONFIG 0b00010000
#elif GYRO_RANGE == 3
	#define GYRO_SENS 16.4
	#define GYRO_CONFIG 0b00011000
#else // Par défaut à zéro
	#define GYRO_SENS 131.0
	#define GYRO_CONFIG 0b00000000
#endif
#undef GYRO_RANGE


//Offsets - En fonction de la fonction getOffsets
//     Accéléromètre
#define A_OFF_X -2951.47
#define A_OFF_Y -382.905
#define A_OFF_Z -15987.4
//    Gyroscope
#define G_OFF_X -733
#define G_OFF_Y 433
#define G_OFF_Z -75


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
    // std::cout << "RAW x => " << x << " | " << "y => " << y << " | " << "z => " << z << "\n";
	*x = round((*x - A_OFF_X) * 1000.0 / ACCEL_SENS) / 1000.0; // On enlève l'offset et on divise par la sensibilité de l'accéléromètre (round et 1000 pour avoir 3 décimales...)
	*y = round((*y - A_OFF_Y) * 1000.0 / ACCEL_SENS) / 1000.0;
	*z = round((*z - A_OFF_Z) * 1000.0 / ACCEL_SENS) / 1000.0;
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
	*roll = round((*roll - G_OFF_X) * 1000.0 / GYRO_SENS) / 1000.0; // On enlève l'offset et on divise par la sensibilité du gyroscope (round et 1000 pour avoir 3 décimales...)
	*pitch = round((*pitch - G_OFF_Y) * 1000.0 / GYRO_SENS) / 1000.0;
	*yaw = round((*yaw - G_OFF_Z) * 1000.0 / GYRO_SENS) / 1000.0;
}

float ax_off, ay_off, az_off, gr_off, gp_off, gy_off;

float ax, ay, az, gr, gp, gy;
float pre_ax = 0;
float pre_ay = 0;
float pre_az = 0;
float pre_gr = 0;
float pre_gp = 0;
float pre_gy = 0;

void getOffsets(float *ax_off, float *ay_off, float *az_off, float *gr_off, float *gp_off, float *gy_off) {
	float gyro_off[3]; // Valeurs temporaires
	float accel_off[3];

	*gr_off = 0, *gp_off = 0, *gy_off = 0; // Initialisation des offsets à 0
	*ax_off = 0, *ay_off = 0, *az_off = 0; // Initialisation des offsets à 0

	for (int i = 0; i < 1000; i++) { // Boucle pour lisser les offsets
        if(i%10 == 0){
            std::cout << i << "/1000\n";
        }
		getGyroRaw(&gyro_off[0], &gyro_off[1], &gyro_off[2]); // Valeurs brutes du gyroscope
		*gr_off = *gr_off + gyro_off[0], *gp_off = *gp_off + gyro_off[1], *gy_off = *gy_off + gyro_off[2]; // On ajoute à la somme

		getAccelRaw(&accel_off[0], &accel_off[1], &accel_off[2]); // Valeurs brutes de l'accéléromètre
		*ax_off = *ax_off + accel_off[0], *ay_off = *ay_off + accel_off[1], *az_off = *az_off + accel_off[2]; // On ajoute à la somme
	}

	*gr_off = *gr_off / 10000, *gp_off = *gp_off / 10000, *gy_off = *gy_off / 10000; //On divise par le nombre de tour de boucle pour avoir une moyenne
	*ax_off = *ax_off / 10000, *ay_off = *ay_off / 10000, *az_off = *az_off / 10000;

	*az_off = *az_off - ACCEL_SENS; // On enlève 1g de la valeur pour compenser la gravité
}

int main(int argc, const char** argv) {
    std::cout << "Lancement du programme de lecture de l'accéléromètre\n";
    i2cFileDescriptor = open("/dev/i2c-1", O_RDWR);
    if (i2cFileDescriptor < 0) {
		std::cout << "Erreur (MPU6050.cpp:MPU6050()): Impossible d'ouvrir /dev/i2c-1. Vérifiez que l'interface I2C est activée dans raspi-config\n";
        return 1;
	}
    int status = ioctl(i2cFileDescriptor, I2C_SLAVE, 0x68);
	if (status < 0) {
		std::cout << "Erreur (MPU6050.cpp:MPU6050()): Impossible de communiquer en I2C à l'adresse " << 0x68 << ". Vérifiez que l'adresse est correcte\n";
        return 1;
	}

    // i2c_smbus_write_byte_data(i2cFileDescriptor, 0x6b, 0b00000000); // On sort l'accéléromètre du mode veille
    // i2c_smbus_write_byte_data(i2cFileDescriptor, 0x1a, 0b00000011); // Filtre passe bas à 44Hz, donc aucun bruit au-dessus de 44Hz ne passera
    // i2c_smbus_write_byte_data(i2cFileDescriptor, 0x19, 0b00000100); // Diviseur de taux d'échantillonage à 200Hz
    i2c_smbus_write_byte_data(i2cFileDescriptor, 0x1b, GYRO_CONFIG);
    i2c_smbus_write_byte_data(i2cFileDescriptor, 0x1c, ACCEL_CONFIG);

    // Remise à zéro des offsets
    i2c_smbus_write_byte_data(i2cFileDescriptor, 0x06, 0b00000000);
    i2c_smbus_write_byte_data(i2cFileDescriptor, 0x07, 0b00000000);
    i2c_smbus_write_byte_data(i2cFileDescriptor, 0x08, 0b00000000);
    i2c_smbus_write_byte_data(i2cFileDescriptor, 0x09, 0b00000000);
    i2c_smbus_write_byte_data(i2cFileDescriptor, 0x0A, 0b00000000);
    i2c_smbus_write_byte_data(i2cFileDescriptor, 0x0B, 0b00000000);
    i2c_smbus_write_byte_data(i2cFileDescriptor, 0x00, 0b10000001);
    i2c_smbus_write_byte_data(i2cFileDescriptor, 0x01, 0b00000001);
    i2c_smbus_write_byte_data(i2cFileDescriptor, 0x02, 0b10000001);

    
    // std::cout << "Récupération des offsets...\n";
    // getOffsets(&ax_off, &ay_off, &az_off, &gr_off, &gp_off, &gy_off);
    // std::cout << "ax_off => " << ax_off << " | " << "ay_off => " << ay_off << " | " << "az_off => " << az_off << "\n";

    std::cout << "Lancement de la boucle de lecture...\n";
    while(1){
        getAccel(&ax, &ay, &az);
        // std::cout << "ax => " << ax << " | " << "ay => " << ay << " | " << "az => " << az << "\n";
        // std::cout << "ax - pre => " << ax-pre_ax << " | " << "ay - pre => " << ay-pre_ay << " | " << "az - pre => " << az-pre_az << "\n";
        auto ecart = 100 - (az/pre_az)*100;
        if(ecart == -0){
            ecart = 0;
        }
        printf("az => %.3f", az);
        if(ecart < 0){
            printf(" | Ecart avec valeur précédente => %.2f %\n", ecart);
        }
        else if (ecart == 0)
        {
            printf(" | Ecart avec valeur précédente =>  0.00 %\n", ecart);
        }
        else{
            printf(" | Ecart avec valeur précédente => +%.2f %\n", ecart);
        }

        std::ofstream outfile;
        outfile.open("trace_axe_z.txt", std::ios_base::app); // append instead of overwrite
        outfile << az << "\n"; 
        
        pre_ax = ax;
        pre_ay = ay;
        pre_az = az;
        usleep(100000);

    }

    
    return 0;
}
