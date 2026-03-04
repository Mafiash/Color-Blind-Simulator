// ColorBlindLib.h : Defines the exported functions for the DLL application
#pragma once
#ifdef COLORBLINDLIB_EXPORTS
#define COLORBLINDLIB_API __declspec(dllexport)
#else
#define COLORBLINDLIB_API __declspec(dllimport)
#endif

extern "C" {

    COLORBLINDLIB_API void ProcessImage(
        unsigned char* imgData,
        int width,
        int height,
        int threads,
        int type
    );
}