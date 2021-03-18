#ifndef SENSORCALIB_H_
#define SENSORCALIB_H_

//Sensor Level State
#define MAX_TEMP_LEVEL	5 //Temperature sensor: Very Low, Low, Normal, High, Very High
#define MAX_HUMD_LEVEL	4 //Humidity sensor: Dry, Normal, Humid, Very Humid
#define MAX_CO2_LEVEL	4 //CO2 sensor: Bad, Normal, Good, Very Good
#define MAX_VOC_LEVEL	4 //VOC sensor: Bad, Normal, Good, Very Good
#define MAX_COL_LEVEL	2 //Color sensor: Off, On

//Sensor Level Value
#define TEMP_COLD	0
#define TEMP_NORMAL	1
#define TEMP_WARM	2
#define TEMP_HOT	3
#define TEMP_VHOT	4

#define HUMD_DRY	0
#define HUMD_NORMAL	1
#define HUMD_HUMID	2
#define HUMD_VHUMID	3

#define CO2_BAD		0
#define CO2_NORMAL	1
#define CO2_GOOD	2
#define CO2_VGOOD	3

#define VOC_BAD		0
#define VOC_NORMAL	1
#define VOC_GOOD	2
#define VOC_VGOOD	3

#define COL_OFF		0
#define COL_ON		1

//Sensor Value
static int temp_value[MAX_TEMP_LEVEL] = {18, 21, 24, 27, 30};
static int humd_value[MAX_HUMD_LEVEL] = {20, 30, 40, 50};
static int co2_value[MAX_CO2_LEVEL] = {700, 1000, 2000, 5000};
static int voc_value[MAX_VOC_LEVEL] = {220, 660, 2200, 10000};
static int col_value[MAX_COL_LEVEL] = {1600, 1800};

void calibsensor(char port, char states, int * value_p);

int get_level(int value, int state, int * value_p)
{
	int i, min, tmp, level;
	
	min = value;
	for (i=0; i<state; i++)
	{
		tmp = abs(value_p[i] - value);
		if (tmp < min) {
			min = tmp;
			level = i;
		}
	}
	return level;
}

int movingAvg(int *ptrArrNumbers, int *ptrSum, int pos, int len, int nextNum) 
{
	//Subtract the oldest number from the prev sum, add the new number
	*ptrSum = *ptrSum - ptrArrNumbers[pos] + nextNum;
	//Assign the nextNum to the position in the array
	ptrArrNumbers[pos] = nextNum;
	//return the average
	return *ptrSum / len;
}

#endif
