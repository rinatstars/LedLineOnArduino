uint16_t analogReadSens(uint8_t sens)
{
  uint16_t sum = 0;
  for (uint8_t i = 0; i < 64; i++) {
    sum += analogRead(sens);
  }
  return sum;
}

//int16_t analogReadTemp()
//{
//  temperature_table_entry_type summ = 0;
//  for (uint8_t i = 0; i < 64; i++) {
//    summ += analogRead(sensTempGreenhouse);
//  }
//  return calc_temperature(summ);
//}
