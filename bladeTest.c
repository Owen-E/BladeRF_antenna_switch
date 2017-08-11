#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <libbladeRF.h>


/* Define XB200 GPIO pins, J16 */
const uint32_t pins_to_config =
    BLADERF_XB200_PIN_J16_1 |
    BLADERF_XB200_PIN_J16_2 |
    BLADERF_XB200_PIN_J16_3 |
    BLADERF_XB200_PIN_J16_4;
const uint32_t output_pins =
    BLADERF_XB200_PIN_J16_1 |
    BLADERF_XB200_PIN_J16_2;




/* The RX and TX modules are configured independently for these parameters */
struct module_config {
    bladerf_module module;
    unsigned int frequency;
    unsigned int bandwidth;
    unsigned int samplerate;
    /* Gains */
    bladerf_lna_gain rx_lna;
    int vga1;
    int vga2;
};
int configure_module(struct bladerf *dev, struct module_config *c)
{
    int status;
    status = bladerf_set_frequency(dev, c->module, c->frequency);
    if (status != 0) {
        fprintf(stderr, "Failed to set frequency = %u: %s\n",
                c->frequency, bladerf_strerror(status));
        return status;
    }
    status = bladerf_set_sample_rate(dev, c->module, c->samplerate, NULL);
    if (status != 0) {
        fprintf(stderr, "Failed to set samplerate = %u: %s\n",
                c->samplerate, bladerf_strerror(status));
        return status;
    }
    status = bladerf_set_bandwidth(dev, c->module, c->bandwidth, NULL);
    if (status != 0) {
        fprintf(stderr, "Failed to set bandwidth = %u: %s\n",
                c->bandwidth, bladerf_strerror(status));
        return status;
    }
    switch (c->module) {
        case BLADERF_MODULE_RX:
            /* Configure the gains of the RX LNA, RX VGA1, and RX VGA2  */
            status = bladerf_set_lna_gain(dev, c->rx_lna);
            if (status != 0) {
                fprintf(stderr, "Failed to set RX LNA gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }
            status = bladerf_set_rxvga1(dev, c->vga1);
            if (status != 0) {
                fprintf(stderr, "Failed to set RX VGA1 gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }
            status = bladerf_set_rxvga2(dev, c->vga2);
            if (status != 0) {
                fprintf(stderr, "Failed to set RX VGA2 gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }
            break;
        case BLADERF_MODULE_TX:
            /* Configure the TX VGA1 and TX VGA2 gains */
            status = bladerf_set_txvga1(dev, c->vga1);
            if (status != 0) {
                fprintf(stderr, "Failed to set TX VGA1 gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }
            status = bladerf_set_txvga2(dev, c->vga2);
            if (status != 0) {
                fprintf(stderr, "Failed to set TX VGA2 gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }
            break;
        default:
            status = BLADERF_ERR_INVAL;
            fprintf(stderr, "%s: Invalid module specified (%d)\n",
                    __FUNCTION__, c->module);
    }
    return status;
}




int initXB200(struct bladerf *dev){

   //Attach XB200 Expansion board
    int status = bladerf_expansion_attach(dev, BLADERF_XB_200);
    printf("Attached XB-200 with status %d\n", status);

    //Select filter 149-159MHz filter bank (change later)
    status = bladerf_xb200_set_filterbank(dev, BLADERF_MODULE_RX, BLADERF_XB200_144M);
    printf("Selected filter bank with status %d\n", status);

    //Bypass XB-200 mixer path
    status = bladerf_xb200_set_path(dev, BLADERF_MODULE_RX, BLADERF_XB200_BYPASS);
    printf("Selected bypass mixer with status %d\n", status);

}


int GPIOtest(struct bladerf *dev){

const uint32_t pins_to_write =
    BLADERF_XB200_PIN_J16_1 |
    BLADERF_XB200_PIN_J16_2 |
    BLADERF_XB200_PIN_J16_3 |
    BLADERF_XB200_PIN_J16_4 ;

// Truth table for antenna selection
const uint32_t antennas[4] = {0, 
    BLADERF_XB200_PIN_J16_1, 
    BLADERF_XB200_PIN_J16_2, 
    BLADERF_XB200_PIN_J16_1 | BLADERF_XB200_PIN_J16_2};

    //Configure J16 pins 1 and 2 as outputs
    int status = bladerf_expansion_gpio_dir_masked_write(dev, pins_to_config, output_pins);
    printf("Configured direction register with status %d\n", status);

    

    /* Loop endlessly, cycling through antennas */
    
    // Max switching speed using following code measured at 101.5uS using logic analyser
    // GPIO voltage 1.8v
    // TODO: Test ability to stop/start sample acquisition between switches
    while(1){ 
        for (int i = 0; i < 4; i ++){
            status =  bladerf_expansion_gpio_masked_write(dev, pins_to_write, antennas[i]);
            //printf("wrote antenna config %d with status %d. ",i, status);
            if(status != 0){
              printf("GPIO write error (%d)\n", status);  
            }
            uint32_t val = 0;
            //status = bladerf_expansion_gpio_read(dev, &val);
            //printf("Pin values are %d\n", val);
            //fflush(stdout);
    
        }
    }

    return 0;
}






/* Usage:
 *   libbladeRF_example_boilerplate [serial #]
 *
 * If a serial number is supplied, the program will attempt to open the
 * device with the provided serial number.
 *
 * Otherwise, the first available device will be used.
 */
int main(int argc, char *argv[])
{
    int status;
    struct module_config config;
    struct bladerf *dev = NULL;
    struct bladerf_devinfo dev_info;
    /* Initialize the information used to identify the desired device
     * to all wildcard (i.e., "any device") values */
    bladerf_init_devinfo(&dev_info);
    /* Request a device with the provided serial number.
     * Invalid strings should simply fail to match a device. */
    if (argc >= 2) {
        strncpy(dev_info.serial, argv[1], sizeof(dev_info.serial) - 1);
    }
    status = bladerf_open_with_devinfo(&dev, &dev_info);
    if (status != 0) {
        fprintf(stderr, "Unable to open device: %s\n",
                bladerf_strerror(status));
        return 1;
    }
    /* Set up RX module parameters */
    config.module     = BLADERF_MODULE_RX;
    config.frequency  = 910000000;
    config.bandwidth  = 2000000;
    config.samplerate = 300000;
    config.rx_lna     = BLADERF_LNA_GAIN_MAX;
    config.vga1       = 30;
    config.vga2       = 3;
    status = configure_module(dev, &config);
    if (status != 0) {
        fprintf(stderr, "Failed to configure RX module. Exiting.\n");
        goto out;
    }
    /* Set up TX module parameters */
    config.module     = BLADERF_MODULE_TX;
    config.frequency  = 918000000;
    config.bandwidth  = 1500000;
    config.samplerate = 250000;
    config.vga1       = -14;
    config.vga2       = 0;
    status = configure_module(dev, &config);
    if (status != 0) {
        fprintf(stderr, "Failed to configure TX module. Exiting.\n");
        goto out;
    }
    /* Application code goes here.
     *
     * Don't forget to call bladerf_enable_module() before attempting to
     * transmit or receive samples!
     */

    initXB200(dev);
    GPIOtest(dev);


out:
    bladerf_close(dev);
    return status;
}



