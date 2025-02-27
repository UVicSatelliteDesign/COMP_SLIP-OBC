/*
 * camera_snapshot.c
 *
 *  Created on: Feb 26, 2025
 *      Author: golde
 */


#include "ov5640.h"
#include "ov5640_reg.h"
#include "camera.h"

static OV5640_Object_t cameraHandle;

void camera_init() {
	// set 640x480 resolution and JPEG format
	// OV5640_Init should write to necessary registers
	OV5640_Init(&cameraHandle, OV5640_R640x480, OV5640_JPEG);

	// look into brightness, contrast, hue, polarity, etc.
	// might not need to worry about sync and timing
}

int camera_snapshot(uint8_t *imageBuffer, uint32_t image_size) {

}
