//
// Glue code between libMini and its file format support
//
// Copyright (c) 2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <mini/convbase.h>

void InitMiniConvHook(int iJpegQuality)
{
	// specify conversion parameters
	static convbase::MINI_CONVERSION_PARAMS conversion_params;
	conversion_params.jpeg_quality = (float) iJpegQuality; // jpeg quality in percent
	conversion_params.usegreycstoration = false; // use greycstoration for image denoising
	conversion_params.greyc_p=0.8f; // greycstoration sharpness, useful range=[0.7-0.9]
	conversion_params.greyc_a=0.4f; // greycstoration anisotropy, useful range=[0.1-0.5]

	// register libMini conversion hook (JPEG/PNG)
	databuf::setconversion(convbase::conversionhook,&conversion_params);
}

