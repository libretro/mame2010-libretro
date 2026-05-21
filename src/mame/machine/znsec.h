/*  CAT702 ZN security chip */

void znsec_init(int chip, const uint8_t *transform);
void znsec_start(int chip);
uint8_t znsec_step(int chip, uint8_t input);
