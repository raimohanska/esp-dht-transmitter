int stateStorageOffset = 256; // leave enough room for the wifi settings

void writeToStorage(int index, temp_hum value)
{
   int ee = memoryPos(index);
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       EEPROM.write(ee++, *p++);
   EEPROM.commit();
}

temp_hum readFromStorage(int index)
{
   temp_hum value;
   int ee = memoryPos(index);
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       *p++ = EEPROM.read(ee++);
   return value;
}

void writeState(app_state value)
{
   int ee = stateStorageOffset;
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       EEPROM.write(ee++, *p++);
   EEPROM.commit();
}

app_state readState()
{
   int ee = stateStorageOffset;
   app_state value;
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       *p++ = EEPROM.read(ee++);
   return value;
}

int memoryPos(int index) {
  temp_hum value;
  app_state state;
  return stateStorageOffset + sizeof(state) + index * sizeof(value);
}
