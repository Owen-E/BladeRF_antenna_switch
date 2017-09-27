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




//struct blade_metadata rx_meta;
 /*   struct bladerf_metadata {
        uint64_t timestamp;
        uint32_t flags;
        uint32_t status;
        unsigned int actual_count;
        uint8_t reserved [32];
    } ;
    
#endif

*/
    /* Contains
    uint64_t timestamp
    uint32_t flags
    uint32_t status
    unsigned int actual_count
    uint8_t reserved [32]

    I will use reserved to store antenna switch status, as this is currently unused by Nuand
    */




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

static int init_sync(struct bladerf *dev)
{
    int status;
    /* These items configure the underlying asynch stream used by the sync
     * interface. The "buffer" here refers to those used internally by worker
     * threads, not the user's sample buffers.
     *
     * It is important to remember that TX buffers will not be submitted to
     * the hardware until `buffer_size` samples are provided via the
     * bladerf_sync_tx call.  Similarly, samples will not be available to
     * RX via bladerf_sync_rx() until a block of `buffer_size` samples has been
     * received.
     */
    const unsigned int num_buffers   = 16;
    const unsigned int buffer_size   = 8192;  /* Must be a multiple of 1024 */
    const unsigned int num_transfers = 8;
    const unsigned int timeout_ms    = 3500;
    /* Configure both the device's RX and TX modules for use with the synchronous
     * interface. SC16 Q11 samples *without* metadata are used. */
    status = bladerf_sync_config(dev,
                                 BLADERF_MODULE_RX,
                                 BLADERF_FORMAT_SC16_Q11_META, //CURRENT, causes error -3 on receive
                                 num_buffers,
                                 buffer_size,
                                 num_transfers,
                                 timeout_ms);
    if (status != 0) {
        fprintf(stderr, "Failed to configure RX sync interface: %s\n",
                bladerf_strerror(status));
        return status;
    }
    status = bladerf_sync_config(dev,
                                 BLADERF_MODULE_TX,
                                 BLADERF_FORMAT_SC16_Q11_META,
                                 num_buffers,
                                 buffer_size,
                                 num_transfers,
                                 timeout_ms);
    if (status != 0) {
        fprintf(stderr, "Failed to configure TX sync interface: %s\n",
                bladerf_strerror(status));
    }
    return status;
}

int do_work(){
    return -1; //returning 0 will transmit response
}


int close_rx_resources(struct bladerf *dev){

    int status = -1;

    /* Disable RX module, shutting down our underlying RX stream */
    status = bladerf_enable_module(dev, BLADERF_MODULE_RX, false);
    if (status != 0) {
        fprintf(stderr, "Failed to disable RX module: %s\n",
                bladerf_strerror(status));
    }
    /* Disable TX module, shutting down our underlying TX stream */
    status = bladerf_enable_module(dev, BLADERF_MODULE_TX, false);
    if (status != 0) {
        fprintf(stderr, "Failed to disable TX module: %s\n",
                bladerf_strerror(status));
    }

}


int sync_rx_prep(struct bladerf *dev){

    int status, ret;
    
   


    /* Initialize synch interface on RX and TX modules */
    status = init_sync(dev);
    if (status != 0) {
        goto out;
    }
    status = bladerf_enable_module(dev, BLADERF_MODULE_RX, true);
    if (status != 0) {
        fprintf(stderr, "Failed to enable RX module: %s\n",
                bladerf_strerror(status));
        goto out;
    }
    status = bladerf_enable_module(dev, BLADERF_MODULE_TX, true);
    if (status != 0) {
        fprintf(stderr, "Failed to enable RX module: %s\n",
                bladerf_strerror(status));
        goto out;
    }



out:
    ret = status;
    
    /* Free up our resources */
    //free(rx_samples);
    //free(tx_samples);
    return ret;

}



int sync_rx(struct bladerf *dev, unsigned int num_samples)//, struct bladerf_metadata *rx_meta)
{




struct bladerf_metadata rx_meta;

    bool have_tx_data = false;
    /* "User" samples buffers and their associated sizes, in units of samples.
     * Recall that one sample = two int16_t values. */
    int16_t *rx_samples = NULL;
    int16_t *tx_samples = NULL;
    const unsigned int samples_len = num_samples;//10000; /* May be any (reasonable) size */
    /* Allocate a buffer to store received samples in */
    rx_samples = malloc(samples_len * 2 * sizeof(int16_t));
    if (rx_samples == NULL) {
        perror("malloc");
        return BLADERF_ERR_MEM;
    }
    /* Allocate a buffer to prepare transmit data in */
    tx_samples = malloc(samples_len * 2 * sizeof(int16_t));
    if (tx_samples == NULL) {
        perror("malloc");
        free(rx_samples);
        return BLADERF_ERR_MEM;
    }

    bool done = false;
    int status = 0;


while (status == 0 && !done) {

       //reset metadata array before usage
        memset(&rx_meta, 0, sizeof(rx_meta));        
        rx_meta.flags = BLADERF_META_FLAG_RX_NOW;

        uint32_t samplestore;

        /* Receive samples */
        status = bladerf_sync_rx(dev, rx_samples, samples_len, &rx_meta, 100);

        if (status == 0) {
            /* Process these samples, and potentially produce a response
             * to transmit */
            done = do_work(rx_samples, samples_len,
                           &have_tx_data, tx_samples, samples_len);
            if (!done && have_tx_data) {
                /* Transmit a response */
                status = bladerf_sync_tx(dev, tx_samples, samples_len,
                                         NULL, 100);
                if (status != 0) {
                    fprintf(stderr, "Failed to TX samples: %s\n",
                            bladerf_strerror(status));
                }
            }
        } else {
            fprintf(stderr, "Failed to RX samples: %s\n",
                    bladerf_strerror(status));
        }
        //receieve timestamp and possibly metadata (takes dev, module, uint64_t buffer)
//        status = bladerf_get_timestamp(dev, BLADERF_MODULE_RX, &meta); //CURRENT


        //TODO: save samples with metadata in csv format
        //printf("rx_samples = %i\n", &rx_samples);

        //status = fwrite(rx_samples, sizeof(int16_t), 2 * num_samples, stdout);

        //stored
        for(int i = 0; i < 2 * num_samples; i = i + 2) {
            printf("%ld,%ld\n", rx_samples[i], rx_samples[i+1]);
        }

        fflush(stdout);
        

       printf("Metadata(antsel) = %d \n", rx_meta.timestamp >> 62);
       printf("Metadata(timestamp) = %lu \n", (rx_meta.timestamp << 2) >>2 );
     //  printf("Metadata(flags) = %x \n", rx_meta.flags);
       printf("Metadata(status) = %u \n", rx_meta.status);
       printf("Metadata(actual_count) = %u \n", rx_meta.actual_count);
  //     printf("Metadata(reserved) = 0x");
    //   for(int a = 0; a < 32; a ++){ printf("%x",rx_meta.reserved[a]);}
       printf("\n\n");

    }

    return status;

}


uint64_t meta = 0;
int GPIOtest(struct bladerf *dev){//, struct bladerf_metadata *rx_meta){

    bool verbose = true;

//number of samples to receive at each antenna configuration
int num_samples = 2;




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
    // TODO: Improve 93ms switching time by removing overhead in sync_rc
    // TODO: Do something with input samples (send to file for now)


//prepare for sync rx:
    sync_rx_prep(dev);

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


            //Receive 10000 samples

            status = sync_rx(dev, num_samples);//, rx_meta); // The overhead in this call moves switching time to 93 ms
           
           if(verbose){ printf("Received %d samples with status (%d)\n\n", num_samples ,status);  }

          // if(verbose){ printf("Metadata :  %lu",meta);}


            //return 0;
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

    
    /* Contains
    uint64_t timestamp
    uint32_t flags
    uint32_t status
    unsigned int actual_count
    uint8_t reserved [32]

    I will use reserved to store antenna switch status, as this is currently unused by Nuand
    */



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
    config.frequency  = 1672500;
    config.bandwidth  = 2000000;
    config.samplerate = 40000000;
    config.rx_lna     = BLADERF_LNA_GAIN_MAX;
    config.vga1       = 20;
    config.vga2       = 6;
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
    GPIOtest(dev);//, rx_meta);


out:
    bladerf_close(dev);
    return status;
}



