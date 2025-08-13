# Grayscale

This project applies a grayscale effect to a bitmap image.

<p align="center"><img src="/assets/screenshot.jpg" alt="App Screenshot"></p>

## How It Works

The conversion process takes each pixel's color and calculates its grayscale equivalent. One might incorrectly use a simple average, like `(1/3)*R + (1/3)*G + (1/3)*B`, which implies that red, green, and blue appear with the same intensity.

Instead, the program uses a more precise formula based on the **luminance of the YUV color model**.

The following C code snippet illustrates this process:

```c
void DrawGrayScale()
{
 int i;
 BYTE r, g, b, y;
 for(i = 0; i < cxDib * cyDib; i++) {
 b = pbitsbmp[i * 3]; // Blue
 g = pbitsbmp[i * 3 + 1]; // Green
 r = pbitsbmp[i * 3 + 2]; // Red
 y = (306 * r + 601 * g + 117 * b) >> 10; // 0.299 * r + 0.587 * g + 0.114 * b
 pbitsbmp[i * 3] = y;
 pbitsbmp[i * 3 + 1] = y;
 pbitsbmp[i * 3 + 2] = y; 
 }
}
```

This weighted average gives a more realistic grayscale result by prioritizing the green and red components, to which our eyes are more sensitive. This calculation is performed for every pixel in the image to create the final output.

## History

Developed in July 2005 using C/WIN32, compiled with Visual Studio .NET 2003 and tested on Windows XP.
