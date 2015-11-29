typedef struct temp_hum temp_hum;
struct temp_hum {
  float temp;
  float hum;
  int age;
};

typedef struct app_state app_state;
struct app_state {
  int version;
  int shouldSend;
};

