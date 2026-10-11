#include <Arduino.h>
#include "common.h"
#include "config.h"
#include "i2s_parallel.h"
#include "gfx3d.h"
#include "cct.h"

#include "rectbmp.h"
#include "lenabmp.h"

#define TAG "main"

#ifdef DOUBLE_BUFFERED
   int DrawBufID;
   uint8_t FrameBuffer[2][FRAME_SIZE];
#else
   uint8_t FrameBuffer[FRAME_SIZE];
#endif

void SetupMatrix() {
    tdIdentity(ProjectionMatrix);
    tdIdentity(ModelviewMatrix);
    Perspective(600, 250, 50, 8192, ProjectionMatrix);
    // Shift the center (adjust these values to move X and Y offset)
    tdTranslate(ModelviewMatrix, 0, 0, 0);
}
    
void DrawSphere(int frameIndex) {    
    CNFGPenX = 0;
    CNFGPenY = 0;
    CNFGClearScreen(0);
    CNFGColor(1);
    tdIdentity(ModelviewMatrix);
    tdIdentity(ProjectionMatrix);
    int x = 0;
    int y = 0;
    //CNFGDrawText("Matrix-based 3D engine", 3);
    gfx_printSz(2, 8, "Matrix-based 3D engine");
    SetupMatrix();
    tdRotateEA(ProjectionMatrix, -20, 0, 0);
    tdRotateEA(ModelviewMatrix, frameIndex, 0, 0);
    int sphereset = (frameIndex / 120);
    if(sphereset > 2) sphereset = 2;
    for(y = -sphereset; y <= sphereset; y++){
        for(x = -sphereset; x <= sphereset; x++){
            if(y == 2) continue;
            ModelviewMatrix[11] = 1000 + tdSIN((x + y)*40 + frameIndex*2);
            ModelviewMatrix[3] = 500*x;
            ModelviewMatrix[7] = 500*y+800;
            DrawGeoSphere();
        }
    }
}


void lcd_config() {
#ifdef DOUBLE_BUFFERED

    i2s_parallel_buffer_desc_t bufdesc[2];
    i2s_parallel_config_t cfg = {
        .gpio_bus = { 16, 4, 2, 15, 13, 14, -1, -1 },
        .gpio_clk = 17,
        .clkspeed_hz = 2*1000*1000,
	   
        .bits = I2S_PARALLEL_BITS_8,
        .bufa = &bufdesc[0],
        .bufb = &bufdesc[1]
    };

    bufdesc[0].memory = FrameBuffer[0];
    bufdesc[0].size = FRAME_SIZE;
    bufdesc[1].memory = FrameBuffer[1]; 
    bufdesc[1].size = FRAME_SIZE;
    DrawBufID = 0; 
    CNFGClearScreen(0);
    DrawBufID = 1; 
    CNFGClearScreen(0);
#else
    i2s_parallel_buffer_desc_t bufdesc;
    i2s_parallel_config_t cfg = {
        .gpio_bus = { 16, 4, 2, 15, 13, 14, -1, -1 },
        .gpio_clk = 17,
        .bits = I2S_PARALLEL_BITS_8,
        .clkspeed_hz = 2*1000*1000,
        .buf = &bufdesc
    };

    bufdesc.memory = FrameBuffer;
    bufdesc.size = FRAME_SIZE;
    CNFGClearScreen(0);
#endif    
    delayMs(50);
    i2s_parallel_setup(&I2S1, &cfg);
    delayMs(50);
    digitalWrite(PIN_BIAS_EN, 1);
    delayMs(50); 
    digitalWrite(PIN_DPY_EN, 1);
}

/*
void lcdTask(void *pvParameters) {
    lcd_config();
    int counter = 0;
    int drawState = 1;
    
    while(1) {
        // Always clear and prepare the current draw buffer
        CNFGClearScreen(0);
        CNFGColor(1);

        switch (drawState) {
            case 0 :
            default : 
                {
                    CNFGPenX = rand() % 12;
                    CNFGPenY = rand() % 60;
                    char sztext[10];
                    sprintf(sztext, "%02d:%02d%s", CNFGPenX, CNFGPenY, rand() & 1 ? "pm" : "am");
                    CNFGDrawText(sztext, 3 + (rand() % 8));
                }
                if (counter > 100) {
                    drawState = 1;
                    counter = 0;
                }
                break;

            case 1 :
                gfx_printSz(0, 8, "System active and running smoothly.");
                gfx_printSz(8, 8, "Configuring display parameters.");
                gfx_printSz(16, 8, "Loading fonts and graphics assets.");
                gfx_printSz(24, 8, "Initializing hardware interfaces.");
                gfx_printSz(32, 8, "Checking frame buffer memory.");
                gfx_printSz(40, 8, "Synchronizing pixel clock signals.");
                gfx_printSz(48, 8, "Ready for real-time rendering.");
                gfx_printSz(56, 8, "Display test sequence starting.");
                gfx_printSz(64, 8, "All modules operational.");
                gfx_printSz(72, 8, "Complete.");
                
                if (counter > 100) {
                    drawState = 2;
                    counter = 0;
                }
                break;

            case 2 :
                {
                    uint8_t* pImg = (counter / 50) & 1 ? (uint8_t*)lenaBitmap : (uint8_t*)rectBitmap;
                    CNFGLoadBitmap(pImg);
                }
                if (counter > 100) {
                    drawState = 3;
                    counter = 0;
                }
                break;

            case 3 :
                DrawSphere(counter % 240);
                if (counter > 100) {
                    drawState = 0;
                    counter = 0;
                }
                break;
        }

#ifdef DOUBLE_BUFFERED
        i2s_parallel_flip_to_buffer(&I2S1, DrawBufID);
        DrawBufID ^= 1;
#endif

        delayMs(20); 
        counter++;         
    }
}

*/
void lcdTask(void *pvParameters) {
    lcd_config();
    int counter = 0;
    int drawState = 1;
    
    while(1) {
        switch (drawState) {
            case 0 :
            default : 
                if ((counter % 100) == 0) {
                    cct_SetMarker();
                    CNFGClearScreen(0);
                    CNFGColor(1);
                    CNFGPenX = rand() % 12;
                    CNFGPenY = rand() % 60;
                    char sztext[10];
                    sprintf(sztext, "%02d:%02d%s", CNFGPenX, CNFGPenY, rand() & 1 ? "pm" : "am");
                    CNFGDrawText(sztext, 3 + (rand() % 8));
                    uint32_t elapsedUs = cct_ElapsedTimeUs();
                    ESP_LOGI(TAG, "vector txt : %dus\r\n", elapsedUs);
#ifdef DOUBLE_BUFFERED
                  //  i2s_parallel_flip_to_buffer(&I2S1, DrawBufID);

                    // Trigger one-shot transmission of the completed draw buffer
        i2s_parallel_send_one_shot(&I2S1, DrawBufID);
                    DrawBufID ^= 1;
#endif
                }
                if (counter > 800) {
                    drawState = 1;
                    counter = 0;
                }
                break;

            case 1 :
                if (counter == 0) {
                    cct_SetMarker();
                    CNFGClearScreen(0);
                    CNFGColor(1);
                    gfx_printSz(2, 8, "System active and running smoothly.");
                    gfx_printSz(10, 8, "Configuring display parameters.");
                    gfx_printSz(20, 8, "Loading fonts and graphics assets.");
                    gfx_printSz(30, 8, "Initializing hardware interfaces.");
                    gfx_printSz(40, 8, "Checking frame buffer memory.");
                    gfx_printSz(50, 8, "Synchronizing pixel clock signals.");
                    gfx_printSz(60, 8, "Ready for real-time rendering.");
                    gfx_printSz(70, 8, "Display test sequence starting.");
                    gfx_printSz(80, 8, "All modules operational.");
                    gfx_printSz(90, 8, "Complete.");
                    uint32_t elapsedUs = cct_ElapsedTimeUs();
                    ESP_LOGI(TAG, "bitmap txt : %dus\r\n", elapsedUs);
#ifdef DOUBLE_BUFFERED
                   // i2s_parallel_flip_to_buffer(&I2S1, DrawBufID);
                   // Trigger one-shot transmission of the completed draw buffer
        i2s_parallel_send_one_shot(&I2S1, DrawBufID);
                    DrawBufID ^= 1;
#endif
                }
                if (counter > 500) { //250
                    drawState = 2;
                    counter = 0;
                }
                break;

            case 2 :
                if ((counter % 100) == 0) {
                    cct_SetMarker();
                    CNFGClearScreen(0);
                    uint8_t* pImg = (counter / 100) & 1 ? (uint8_t*)lenaBitmap : (uint8_t*)rectBitmap;
                    CNFGLoadBitmap(pImg);
                    uint32_t elapsedUs = cct_ElapsedTimeUs();
                    ESP_LOGI(TAG, "Load bmp : %dus\r\n", elapsedUs);
#ifdef DOUBLE_BUFFERED
                   // i2s_parallel_flip_to_buffer(&I2S1, DrawBufID);
                   // Trigger one-shot transmission of the completed draw buffer
        i2s_parallel_send_one_shot(&I2S1, DrawBufID);
                    DrawBufID ^= 1;
#endif
                }
                if (counter == 500) {
                    drawState = 3;
                    counter = 0;
                    CNFGClearScreen(0);
                }
                break;

            case 3 :
                cct_SetMarker();
                DrawSphere(counter % 240);
                uint32_t elapsedUs = cct_ElapsedTimeUs();
#ifdef DOUBLE_BUFFERED
               // i2s_parallel_flip_to_buffer(&I2S1, DrawBufID);
                // Trigger one-shot transmission of the completed draw buffer
        i2s_parallel_send_one_shot(&I2S1, DrawBufID);
                DrawBufID ^= 1;
#endif
                if (counter > 400) {
                    drawState = 0;
                    counter = 0;
                }
                break;
        }
        delayMs(20); 
        counter++;         
    }
}
    
/*
    void lcdTask(void *pvParameters) {
    lcd_config();
    int counter = 0;

    const char* messages[] = {
        "System active and running smoothly.",
        "Configuring display parameters.",
        "Loading fonts and graphics assets.",
        "Initializing hardware interfaces.",
        "Checking frame buffer memory.",
        "Synchronizing pixel clock signals.",
        "Ready for real-time rendering.",
        "Display test sequence starting.",
        "All modules operational.",
        "Complete."
    };
    const int total_lines = 10;

    while(1) {
        // Clear screen at the start of a new loop cycle
        if (counter == 0) {
            cct_SetMarker();
            CNFGClearScreen(0);
            CNFGColor(1);
        }

        // Reveal lines progressively every 15 counts (~750ms per line)
        int lines_to_show = counter / 15;
        if (lines_to_show > total_lines) {
            lines_to_show = total_lines;
        }

        // Print visible lines up to the current progress point
        for (int i = 1; i < lines_to_show; i++) {
            gfx_printSz(i * 8, 8, (char*)messages[i]);
        }

        if (lines_to_show == total_lines && (counter % 100 == 0)) {
            uint32_t elapsedUs = cct_ElapsedTimeUs();
            ESP_LOGI(TAG, "bitmap txt : %dus\r\n", elapsedUs);
        }

#ifdef DOUBLE_BUFFERED
        i2s_parallel_flip_to_buffer(&I2S1, DrawBufID);
        DrawBufID ^= 1;
#endif

        delayMs(50);
        counter++;

        // Hold the final complete screen for a few seconds before restarting
        if (counter > (total_lines * 15 + 60)) {
            counter = 0;
        }
    }
}
    */


void setup() {
    pinMode(PIN_DPY_EN, OUTPUT);
    digitalWrite(PIN_DPY_EN, 0);
    pinMode(PIN_BIAS_EN, OUTPUT);
    digitalWrite(PIN_BIAS_EN, 0);   
    pinMode(PIN_BTN, INPUT_PULLUP);
    /*
lcd_config();
    cct_SetMarker();
                    CNFGClearScreen(0);
                    CNFGColor(1);
                    gfx_printSz(1, 8, "System active and running smoothly.");
                    gfx_printSz(10, 8, "Configuring display parameters.");
                    gfx_printSz(20, 8, "Loading fonts and graphics assets.");
                    gfx_printSz(30, 8, "Initializing hardware interfaces.");
                    gfx_printSz(40, 8, "Checking frame buffer memory.");
                    gfx_printSz(50, 8, "Synchronizing pixel clock signals.");
                    gfx_printSz(60, 8, "Ready for real-time rendering.");
                    gfx_printSz(70, 8, "Display test sequence starting.");
                    gfx_printSz(80, 8, "All modules operational.");
                    gfx_printSz(90, 8, "Complete.");
                    uint32_t elapsedUs = cct_ElapsedTimeUs();
                    ESP_LOGI(TAG, "bitmap txt : %dus\r\n", elapsedUs);
                    #ifdef DOUBLE_BUFFERED
                    i2s_parallel_flip_to_buffer(&I2S1, DrawBufID);
                    DrawBufID ^= 1;
#endif
*/
}

void loop() {
    // Increased stack size from 2048 to 4096 to prevent overflow
    xTaskCreatePinnedToCore(&lcdTask, "lcdTask", 4096, NULL, 20, NULL, 1);
    vTaskDelete(NULL); // Delete setup loop task to prevent duplication
}

