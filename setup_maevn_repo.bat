@echo off
REM MAEVN Repository Setup Script
REM Prepares directory structure and initial configuration

echo ========================================
echo MAEVN Repository Setup
echo ========================================
echo.

REM Create directory structure
echo Creating directory structure...
if not exist "Models\drums" mkdir Models\drums
if not exist "Models\instruments" mkdir Models\instruments
if not exist "Models\vocals" mkdir Models\vocals
if not exist "Presets" mkdir Presets
if not exist "Resources" mkdir Resources
if not exist "Build" mkdir Build
echo ✓ Directories created

REM Check if config.json exists
if exist "Models\config.json" (
    echo ✓ config.json already exists
) else (
    echo Creating Models\config.json...
    (
        echo {
        echo   "808": "drums/808_ddsp.onnx",
        echo   "hihat": "drums/hihat_ddsp.onnx",
        echo   "snare": "drums/snare_ddsp.onnx",
        echo   "piano": "instruments/piano_ddsp.onnx",
        echo   "synth": "instruments/synth_fm.onnx",
        echo   "vocal_tts": "vocals/vocals_tts.onnx",
        echo   "vocal_hifigan": "vocals/vocals_hifigan.onnx"
        echo }
    ) > Models\config.json
    echo ✓ config.json created
)

REM Create README for vocal models
echo Creating vocal models README...
(
    echo MAEVN Vocal Models Directory
    echo =============================
    echo.
    echo This directory should contain your custom vocal synthesis models:
    echo.
    echo 1. vocals_tts.onnx - Text-to-Speech ONNX model
    echo 2. vocals_hifigan.onnx - HiFiGAN vocoder ONNX model
    echo.
    echo These models are not included in the repository.
    echo Please add your own ONNX models or use compatible models from:
    echo - Coqui TTS: https://github.com/coqui-ai/TTS
    echo - ONNX Model Zoo: https://github.com/onnx/models
    echo.
    echo Model requirements:
    echo - Input: Text or phoneme embeddings
    echo - Output: Mel-spectrogram or waveform
    echo - Format: ONNX ^(opset 11+^)
) > Models\vocals\README.txt
echo ✓ Vocal models README created

echo.
echo ========================================
echo Setup Complete!
echo ========================================
echo.
echo Next steps:
echo 1. Run build_maevn_onnx.bat to generate instrument models
echo 2. Add your vocal ONNX models to Models\vocals\
echo 3. Build with CMake:
echo    cmake -B Build -S . -DJUCE_PATH="C:/JUCE" -DONNXRUNTIME_PATH="C:/onnxruntime"
echo    cmake --build Build --config Release
echo.
pause
