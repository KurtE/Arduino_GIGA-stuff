#include <RPC.h>
void setup() {
  // put your setup code here, to run once:
  // If CM4 is already booted, disable auto-boot and reset.
  Serial.begin(115200);
  while (!Serial && millis() < 10000) {}
  FLASH_OBProgramInitTypeDef OBInit;

  OBInit.Banks = FLASH_BANK_1;
  HAL_FLASHEx_OBGetConfig(&OBInit);
  if (OBInit.USERConfig & FLASH_OPTSR_BCM4) {
    OBInit.OptionType = OPTIONBYTE_USER;
    OBInit.USERType = OB_USER_BCM4;
    OBInit.USERConfig = 0;
    check_return_status("HAL_FLASH_OB_Unlock ", HAL_FLASH_OB_Unlock());
    check_return_status("HAL_FLASH_Unlock ", HAL_FLASH_Unlock());
    check_return_status("HAL_FLASHEx_OBProgram ", HAL_FLASHEx_OBProgram(&OBInit));
    check_return_status("HAL_FLASH_OB_Launch ", HAL_FLASH_OB_Launch());
    check_return_status("HAL_FLASH_OB_Lock ", HAL_FLASH_OB_Lock());
    check_return_status("HAL_FLASH_Lock ", HAL_FLASH_Lock());
    Serial.println("CM4 autoboot disabled\n");
    NVIC_SystemReset();
    return;
  } else {
    Serial.println("FLASH_OPTSR_BCM4 not set");
  }
}

void check_return_status(const char *sz, HAL_StatusTypeDef ret) {
  if (ret == HAL_OK) return; 
  Serial.print(sz);
  Serial.print("failed: ");
  Serial.println(ret);
}



void loop() {
  // put your main code here, to run repeatedly:
}
