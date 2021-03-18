#include <assert.h>
#include <glib.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include "gattlib.h"
#include "./lib/thingy52.h"
#include "./lib/sensorcalib.h"

//function pointer ble_discovered_device
typedef void (*ble_discovered_device_t)(const char* addr, const char* name);

//Mutex to make BLE connections synchronous
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

/*LIST
 * LIST_HEAD(HEADNAME, TYPE);
	HEADNAME is the name of the structure to be defined, and TYPE is the type of the elements to be linked to the list
 * LIST_ENTRY declares a structure that connects the elements in the list
 * LIST_INIT initializes the list referenced by head 
 * LIST_INSERT_HEAD inserts the new element elm at the head of the list
 * LIST_REMOVE removes the element elm from the list 
*/
LIST_HEAD(listhead, connection_t) g_ble_connections;
struct connection_t {
	pthread_t thread;
	char* addr;
	char* name;
	LIST_ENTRY(connection_t) entries;
};

//static uuid_t g_uuid;
static uuid_t temp_uuid;
static uuid_t hum_uuid;
static uuid_t gas_uuid;
static uuid_t color_uuid;

static GMainLoop *m_main_loop;

//BLE packet
struct Packet {
	char* name;
	
	uint8_t humidity;

	//Temperature Moving Average Filter Variables
	int arrNumbers[10];
	int pos;
	int newAvg;
	int sum;
	int len;
	
	//Level
	int level;
};

//File
FILE *fp;


//Notification Handler
void notification_handler(const uuid_t* uuid, const void* data, size_t data_length, struct Packet* p) {
	
	//Local Variables
	char uuid_str[MAX_LEN_UUID_STR + 1];
	
	//Timestamp
	time_t t;
	t = time(NULL);
	struct tm tm = *localtime(&t);

	fp = fopen(p->name, "a");

	gattlib_uuid_to_string(uuid, uuid_str, sizeof(uuid_str));
	
	if(!strcmp(uuid_str, UUID_ENV_HUMD)) {
		p->humidity = *(uint8_t*)data;
	}
	else {
		printf("Notification Handler Error\n");
	}
	
	p->newAvg = movingAvg(p->arrNumbers, &(p->sum), p->pos, p->len, p->humidity);
	(p->pos)++;
	if((p->pos) >= (p->len)){
		p->pos = 0;
	}
	
	p->level = get_level(p->newAvg, MAX_HUMD_LEVEL, humd_value);
	
	fprintf(fp, "%d-%d-%d %d:%d:%d,%s,%u,%d,%d\n", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, 
		p->name, p->humidity, p->newAvg, p->level);
	
	fflush(fp);
	fclose(fp);
}

static void on_user_abort(int arg) {
	g_main_loop_quit(m_main_loop);
}

//Thingy thread connection 
static void *ble_connect_device(void *arg) {
	struct connection_t *connection = arg;
	char* addr = connection->addr;
	char* name = connection->name;
	gatt_connection_t* gatt_connection;
	gattlib_primary_service_t* services;
	gattlib_characteristic_t* characteristics;
	int services_count, characteristics_count, characteristics_num;
	int ret, i;
	
	struct Packet *p;
	
	pthread_mutex_lock(&g_mutex);
	printf("--------------- START %s ---------------\n", name);
	
	//Connect to assigned device address
	gatt_connection = gattlib_connect(NULL, addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
	if (gatt_connection == NULL) {
		fprintf(stderr, "Fail to connect to the bluetooth device.\n");
		printf("--------------- DONE %s ---------------\n", name);
	} else {
		puts("Succeeded to connect to the bluetooth device.\n");
	}

	printf("--------------- DONE %s ---------------\n", name);
			
	//Create CSV File for each connected Thingy
	fp = fopen(name, "w");
	fputs("DateTime,Name,Humidity, Filtered Humd, Level\n", fp);
	fclose(fp);
	
	//Create and Initialize Packet for each connected Thingy 
	p = malloc(sizeof(struct Packet));
	p->name = name;
	p->pos = 0;
	p->newAvg = 0;
	p->sum = 0;
	p->len = sizeof(p->arrNumbers)/sizeof(int);
	for(int i=0; i < p->len; i++) {
		p->arrNumbers[i] = 0;
	}
	p->level = 0;
	pthread_mutex_unlock(&g_mutex);
	
	/*HUMIDITY DATA*/
	//Change UUID string into UUID
	gattlib_string_to_uuid(UUID_ENV_HUMD, strlen(UUID_ENV_HUMD), &hum_uuid);
	//Register and start notification (pass Packet to the handler)
	gattlib_register_notification(gatt_connection, notification_handler, ((struct Packet *)p));
	ret = gattlib_notification_start(gatt_connection, &hum_uuid);
	if (ret) {
		fprintf(stderr, "Fail to start notification.\n");
		printf("Error code: %d\n", ret);
		free(p);
		gattlib_disconnect(gatt_connection);
	}
}


//Fuction to discover devices
//If the name starts with Thingy, it creates connection thread
static void ble_discovered_device(void *adapter, const char* addr, const char* name, void *user_data) {
	struct connection_t *connection;
	int ret;
	
	if (name) {
		printf("Discovered %s - '%s'\n", addr, name);
	} else {
		printf("Discovered %s\n", addr);
	}
	
	if(name && strncmp(name, "Thingy", 6) == 0) {
		connection = malloc(sizeof(struct connection_t));
		if (connection == NULL) {
			fprintf(stderr, "Fail to allocate connection.\n");
			return;
		}
		connection->addr = strdup(addr);
		connection->name = strdup(name);
		
		ret = pthread_create(&connection->thread, NULL, ble_connect_device, connection);
		if (ret != 0) {
			fprintf(stderr, "Fail to create BLE conneciton thread.\n");
			free(connection);
			return;
		}
		LIST_INSERT_HEAD(&g_ble_connections, connection, entries);
	}
}


int main(int argc, char *argv[]) {
	const char* adapter_name;
	void* adapter;
	int ret;

	//Terminal input
	if (argc == 1) {
		adapter_name = NULL;
	} else if (argc == 2) {
		adapter_name = argv[1];
	} else {
		fprintf(stderr, "%s [<bluetooth-adapter>]\n", argv[0]);
		return 1;
	}
	
	//Scan
	ret = gattlib_adapter_open(adapter_name, &adapter);
	if (ret) {
		fprintf(stderr, "ERROR: Failed to open adapter,\n");
		return 1;
	}
	
	pthread_mutex_lock(&g_mutex);
	ret = gattlib_adapter_scan_enable(adapter, ble_discovered_device, BLE_SCAN_TIMEOUT, NULL);
	if (ret) {
		fprintf(stderr, "ERROR: Failed to scan.\n");
		goto EXIT;
	}
	
	gattlib_adapter_scan_disable(adapter);
	
	puts("Scan completed");
	pthread_mutex_unlock(&g_mutex);
	
	//Wait for connection thread to complete and remove when thread is over
	while (g_ble_connections.lh_first != NULL) {
		struct connection_t* connection = g_ble_connections.lh_first;
		pthread_join(connection->thread, NULL);
		LIST_REMOVE(g_ble_connections.lh_first, entries);
		free(connection->addr);
		free(connection);
	}


	// Catch CTRL-C
	signal(SIGINT, on_user_abort);

	m_main_loop = g_main_loop_new(NULL, 0);
	g_main_loop_run(m_main_loop);

	// In case we quit the main loop, clean the connection
	//gattlib_notification_stop(connection, &g_uuid);
	g_main_loop_unref(m_main_loop);

EXIT:
	gattlib_adapter_close(adapter);
	return ret;
}
